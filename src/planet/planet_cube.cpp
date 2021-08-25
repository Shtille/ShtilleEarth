#include "planet_cube.h"
#include "planet_tree.h"
#include "planet_tile_mesh.h"
#include "planet_renderable.h"
#include "planet_albedo_task.h"

#include "math/matrix3.h"
#include "math/matrix4.h"
#include "math/frustum.h"
#include "graphics/renderer.h"

#include <cmath>

namespace {
	// The more detail coefficient is, the less detalization is required
	const float kGeoDetail = 6.0f;
	const float kTexDetail = 3.0f;
}

PlanetCube::PlanetCube(PlanetTileProvider * tile_provider, scythe::Renderer * renderer, scythe::Shader * shader,
	scythe::CameraManager * camera, scythe::Frustum * frustum, const scythe::Vector3& planet_position, float radius)
	: renderer_(renderer)
	, shader_(shader)
	, camera_(camera)
	, frustum_(frustum)
	, grid_size_(17)
	, planet_position_(planet_position)
	, radius_(radius)
	, service_(tile_provider)
	, frame_counter_(0)
	, lod_freeze_(false)
	, tree_freeze_(false)
	, preprocess_(true)
{
	for (int i = 0; i < kNumFaces; ++i)
		faces_[i] = new PlanetTree(this, i);
	tile_ = new PlanetTileMesh(renderer, grid_size_);
}
PlanetCube::~PlanetCube()
{
	for (int i = 0; i < kNumFaces; ++i)
		delete faces_[i];
	delete tile_;
	service_.StopService();
}
bool PlanetCube::Initialize()
{
	scythe::VertexFormat * object_vertex_format;
	{
		scythe::VertexAttribute attributes[] = {
			scythe::VertexAttribute(scythe::VertexAttribute::kVertex, 3),
		};
		renderer_->AddVertexFormat(object_vertex_format, attributes, _countof(attributes));
	}
	tile_->Create();
	if (!tile_->MakeRenderable(object_vertex_format, nullptr, false))
		return false;
	// Run service
	service_.RunService();
	return true;
}
void PlanetCube::SetParameters(float fovy_in_radians, int screen_height)
{
	float fov = 2.0f * tan(0.5f * fovy_in_radians);

	float geo_detail = std::max(1.0f, kGeoDetail);
	lod_params_.geo_factor = screen_height / (geo_detail * fov);

	float tex_detail = std::max(1.0f, kTexDetail);
	lod_params_.tex_factor = screen_height / (tex_detail * fov);
}
void PlanetCube::Update()
{
	// Update LOD state.
	if (!lod_freeze_)
	{
		lod_params_.camera_position = *camera_->position() - planet_position_;
		lod_params_.camera_front = camera_->GetForward();
		lod_params_.camera_distance = lod_params_.camera_position.Length();
	}

	if (preprocess_)
	{
		PreprocessTree();
		preprocess_ = false;
	}
	// Handle delayed requests (for rendering new tiles).
	HandleRenderRequests();

	if (!tree_freeze_)
	{
		// Prune the LOD tree
		PruneTree();

		// Update LOD requests.
		HandleInlineRequests();
	}
}
void PlanetCube::Render()
{
	renderer_->PushMatrix();
	renderer_->Translate(planet_position_);

	// Model matrix should be taken after all transforms took place
	scythe::Matrix4 projection_view_matrix;
	frustum_->GetMatrix(&projection_view_matrix);
	scythe::Matrix4 mvp = projection_view_matrix * renderer_->model_matrix();

	shader_->Bind();
	shader_->UniformMatrix4fv("u_projection_view_model", mvp);

	for (int i = 0; i < kNumFaces; ++i)
		faces_[i]->Render();

	++frame_counter_;

	shader_->Unbind();

	renderer_->PopMatrix();
}
void PlanetCube::SplitQuadTreeNode(PlanetTreeNode* node)
{
	// Parent is no longer an open node, now has at least one child.
	if (node->parent_)
		open_nodes_.erase(node->parent_);
	// This node is now open.
	open_nodes_.insert(node);
	// Create children.
	for (int i = 0; i < 4; ++i)
	{
		PlanetTreeNode* child = new PlanetTreeNode(node->owner_);
		node->AttachChild(child, i);
	}
}
void PlanetCube::MergeQuadTreeNode(PlanetTreeNode* node)
{
	// Delete children.
	for (int i = 0; i < 4; ++i)
	{
		if (node->children_[i]->children_[0] ||
			node->children_[i]->children_[1] ||
			node->children_[i]->children_[2] ||
			node->children_[i]->children_[3]) {
			printf("Lickety split. Faulty merge.\n");
			assert(false);
		}
		node->DetachChild(i);
	}
	// This node is now closed.
	open_nodes_.erase(node);
	if (node->parent_)
	{
		// Check to see if any siblings are split.
		for (int i = 0; i < 4; ++i)
			if (node->parent_->children_[i]->IsSplit())
				return;
		// If not, the parent is now open.
		open_nodes_.insert(node->parent_);
	}
}
void PlanetCube::Request(PlanetTreeNode* node, int type, bool priority)
{
	RequestQueue& request_queue = (type == REQUEST_MAPTILE) ? render_requests_ : inline_requests_;
	if (priority)
		request_queue.push_front(RequestType(node, type));
	else
		request_queue.push_back(RequestType(node, type));
}
void PlanetCube::Unrequest(PlanetTreeNode* node)
{
	RequestQueue* request_queues[] = { &render_requests_, &inline_requests_ };
	for (int q = 0; q < 2; ++q)
	{
		RequestQueue::iterator i = (*request_queues[q]).begin(), j;
		while (i != (*request_queues[q]).end())
		{
			if ((*i).node == node)
			{
				// Remove item at front of queue.
				if (i == (*request_queues[q]).begin())
				{
					(*request_queues[q]).erase(i);
					i = (*request_queues[q]).begin();
					continue;
				}
				else // Remove item mid-queue.
				{
					j = i;
					--i;
					(*request_queues[q]).erase(j);
				}
			}
			++i;
		}
	}
}
void PlanetCube::HandleRequests(RequestQueue& requests)
{
	// Ensure we only use up x time per frame.
	int weights[] = { 10, 10, 1, 2 };
	bool(PlanetCube::*handlers[4])(PlanetTreeNode*) = {
		&PlanetCube::HandleRenderable,
		&PlanetCube::HandleMapTile,
		&PlanetCube::HandleSplit,
		&PlanetCube::HandleMerge
	};
	int limit = 10;
	bool sorted = false;

	while (!requests.empty())
	{
		RequestType request = *requests.begin();
		PlanetTreeNode* node = request.node;

		// If not a root level task.
		if (node->parent_)
		{
			// Verify job limits.
			if (limit <= 0) return;
			limit -= weights[request.type];
		}

		requests.pop_front();

		// Call handler.
		if ((this->*handlers[request.type])(node))
		{
			// Job was completed. We can re-sort the priority queue.
			if (!sorted)
			{
				requests.sort(RequestComparePriority());
				sorted = true;
			}
		}
	}
}
void PlanetCube::HandleRenderRequests()
{
	// Process tasks that have been done on a service thread
	if (!preprocess_)
		ProcessDoneTasks();

	// Then handle requests
	HandleRequests(render_requests_);
}
void PlanetCube::HandleInlineRequests()
{
	HandleRequests(inline_requests_);
}
void PlanetCube::ProcessDoneTasks()
{
	PlanetService::TaskList done_tasks;
	if (service_.GetDoneTasks(done_tasks))
	{
		PlanetTask * task;
		while (!done_tasks.empty())
		{
			task = done_tasks.front();
			done_tasks.pop_front();
			ProcessTask(task);
			service_.ReleaseTask(task);
		}
	}
}
void PlanetCube::ProcessTask(const PlanetTask * task)
{
	PlanetTreeNode * node = nullptr;
	auto it = key_node_map_.find(task->key());
	if (it != key_node_map_.end())
		node = it->second;
	else
		return;

	switch (task->type())
	{
	case PlanetTaskType::kAlbedo:
		{
			const PlanetAlbedoTask * albedo_task = dynamic_cast<const PlanetAlbedoTask*>(task);
			node->OnAlbedoTaskCompleted(*albedo_task->image(), albedo_task->completed());
		}
		break;
	default:
		break;
	}
}
bool PlanetCube::HandleRenderable(PlanetTreeNode* node)
{
	// Determine max relative LOD depth between grid and tile
	int maxLODRatio = (GetTextureSize() - 1) / (grid_size_ - 1);
	int maxLOD = 0;
	while (maxLODRatio > 1) {
		maxLODRatio >>= 1;
		maxLOD++;
	}

	// See if we can find a maptile to derive from.
	PlanetTreeNode * ancestor = node;
	while (!ancestor->has_map_tile_ && ancestor->parent_) { ancestor = ancestor->parent_; };

	// See if map tile found is in acceptable LOD range (ie. gridsize <= texturesize).
	if (ancestor->has_map_tile_)
	{
		int relativeLOD = node->lod_ - ancestor->lod_;
		if (relativeLOD <= maxLOD)
		{
			// Replace existing renderable.
			node->RefreshRenderable(&ancestor->map_tile_);
			node->request_renderable_ = false;
		}
	}

	// If no renderable was created, try creating a map tile.
	if (node->request_renderable_ && !node->has_map_tile_ && !node->request_map_tile_)
	{
		// Request a map tile for this node's LOD level.
		node->request_map_tile_ = true;
		Request(node, REQUEST_MAPTILE, true);
	}
	return true;
}
bool PlanetCube::HandleMapTile(PlanetTreeNode* node)
{
	node->request_map_tile_ = false;
	bool has_albedo = node->map_tile_.HasAlbedoTexture();
	if (!has_albedo)
	{
		RequestTexture(node);
	}
	if (preprocess_)
	{
		// At preprocess stage we just need to enqueue tasks
		return false;
	}
	bool tile_filled = has_albedo /*&& has_heightmap*/;
	if (tile_filled)
	{
		// Map tile is complete and can be used as ancestor
		node->CreateMapTile();

		// Request a new renderable to match.
		node->RefreshRenderable(&node->map_tile_);
		node->request_renderable_ = false;

		// See if any child renderables use the old map tile.
		if (node->has_renderable_)
		{
			PlanetMapTile * old_tile = node->renderable_.GetMapTile();
			RefreshMapTile(node, old_tile, &node->map_tile_);
		}

		return true;
	}
	else // ! tile_filled
	{
		// Repeat this request until is done
		node->request_map_tile_ = true;
		Request(node, REQUEST_MAPTILE, false);
		return false;
	}
}
bool PlanetCube::HandleSplit(PlanetTreeNode* node)
{
	if (node->lod_ < GetLodLimit())
	{
		SplitQuadTreeNode(node);
		node->request_split_ = false;
	}
	else
	{
		// request_split_ is stuck on, so will not be requested again.
	}
	return true;
}
bool PlanetCube::HandleMerge(PlanetTreeNode* node)
{
	MergeQuadTreeNode(node);
	node->request_merge_ = false;
	return true;
}
void PlanetCube::RequestTexture(PlanetTreeNode* node)
{
	if (!node->request_albedo_)
	{
		node->request_albedo_ = true;
		service_.AddAlbedoTask(node->GetKey());
	}
}
void PlanetCube::PruneTree()
{
	NodeHeap heap;

	NodeSet::iterator open_node = open_nodes_.begin();
	while (open_node != open_nodes_.end())
	{
		heap.push(*open_node);
		++open_node;
	};

	while (!heap.empty())
	{
		PlanetTreeNode* old_node = heap.top();
		heap.pop();
		if (!old_node->page_out_ &&
			!old_node->request_merge_ &&
			(GetFrameCounter() - old_node->last_opened_ > 100))
		{
			old_node->renderable_.SetFrameOfReference();
			// Make sure node's children are too detailed rather than just invisible.
			if (old_node->renderable_.IsFarAway() ||
				(old_node->renderable_.IsInLODRange() && old_node->renderable_.IsInMIPRange())
				)
			{
				old_node->request_merge_ = true;
				Request(old_node, REQUEST_MERGE, true);
				return;
			}
			else
			{
				old_node->last_opened_ = GetFrameCounter();
			}
		}
	}
}
void PlanetCube::PreprocessTree()
{
	do
	{
		HandleRenderRequests();
		HandleInlineRequests();
		Render();
	}
	while (!render_requests_.empty() || !inline_requests_.empty());
}
void PlanetCube::RefreshMapTile(PlanetTreeNode* node, PlanetMapTile* old_tile, PlanetMapTile* new_tile)
{
	for (int i = 0; i < 4; ++i)
	{
		PlanetTreeNode* child = node->children_[i];
		if (child && child->has_renderable_ && child->renderable_.GetMapTile() == old_tile)
		{
			// Refresh renderable relative to the map tile.
			child->RefreshRenderable(new_tile);

			// Recurse
			RefreshMapTile(child, old_tile, new_tile);
		}
	}
}
const float PlanetCube::radius() const
{
	return radius_;
}
const int PlanetCube::grid_size() const
{
	return grid_size_;
}
scythe::Matrix3 PlanetCube::GetFaceTransform(int face)
{
	switch (face)
	{
	default:
	case 0:
		return scythe::Matrix3(	0, 0, 1,
								0, 1, 0,
								1, 0, 0
							);
	case 1:
		return scythe::Matrix3(	0, 0,-1,
								0, 1, 0,
								-1, 0, 0
							);
	case 2:
		return scythe::Matrix3(	1, 0, 0,
								0, 0, 1,
								0, 1, 0
							);
	case 3:
		return scythe::Matrix3(	1, 0, 0,
								0, 0,-1,
								0,-1, 0
							);
	case 4:
		return scythe::Matrix3(	-1, 0, 0,
								0, 1, 0,
								0, 0, 1
								);
	case 5:
		return scythe::Matrix3(	1, 0, 0,
								0, 1, 0,
								0, 0,-1
							);
	}
}
int PlanetCube::GetFrameCounter() const
{
	return frame_counter_;
}
const int PlanetCube::GetLodLimit() const
{
	return 12;
}
const int PlanetCube::GetTextureSize() const
{
	return 257;
}
bool PlanetCube::RequestComparePriority::operator()(const RequestType& a, const RequestType& b) const
{
	return (a.node->GetPriority() > b.node->GetPriority());
}