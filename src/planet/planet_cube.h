#ifndef __PLANET_CUBE_H__
#define __PLANET_CUBE_H__

#include "planet_tree.h"

#include "common/non_copyable.h"
#include "camera.h"

#include <list>
#include <set>
#include <queue>

namespace scythe {
		class Renderer;
		class Shader;
		class Frustum;
}

// Forward declarations
class PlanetTileMesh;
class PlanetMap;
class PlanetMapTile;
class PlanetService;

struct LodParams {
	int limit;

	scythe::Vector3 camera_position;	//!< position of camera in geocentric coordinate system
	scythe::Vector3 camera_front;		//!< forward direction vector of camera
	float camera_distance;			//!< length of camera_position vector (to not compute per each tile)

	float geo_factor;
	float tex_factor;
};

//! Planet rendering class
class PlanetCube final : public scythe::NonCopyable {
	friend class PlanetTreeNode;
	friend class PlanetRenderable;

	enum {
		REQUEST_RENDERABLE,
		REQUEST_MAPTILE,
		REQUEST_SPLIT,
		REQUEST_MERGE,
	};

	struct RequestType {
		PlanetTreeNode* node;
		int type;

		RequestType(PlanetTreeNode* node, int type) : node(node), type(type) {};
	};

	class RequestComparePriority {
	public:
		bool operator()(const RequestType& a, const RequestType& b) const;
	};

	typedef std::list<RequestType> RequestQueue;
	typedef std::set<PlanetTreeNode*> NodeSet;
	typedef std::priority_queue<PlanetTreeNode*, std::vector<PlanetTreeNode*>, PlanetTreeNodeCompareLastOpened> NodeHeap;

public:
	PlanetCube(PlanetService * albedo_service, scythe::Renderer * renderer, scythe::Shader * shader,
		scythe::CameraManager * camera, scythe::Frustum * frustum, const scythe::Vector3& planet_position, float radius);
	~PlanetCube();

	//! Data video memory objects creation and other things that may fail
	bool Initialize();

	void SetParameters(float fovy_in_radians, int screen_height);

	void Update();
	void Render();

	const float radius() const;
	const int grid_size() const;

	const int GetLodLimit() const;
	const int GetTextureSize() const;

	static scythe::Matrix3 GetFaceTransform(int face);
	int GetFrameCounter() const;

protected:
	void SplitQuadTreeNode(PlanetTreeNode* node);
	void MergeQuadTreeNode(PlanetTreeNode* node);
	void Request(PlanetTreeNode* node, int type, bool priority = false);
	void Unrequest(PlanetTreeNode* node);
	void HandleRequests(RequestQueue& requests);
	void HandleRenderRequests();
	void HandleInlineRequests();

	bool HandleRenderable(PlanetTreeNode* node);
	bool HandleMapTile(PlanetTreeNode* node);
	bool HandleSplit(PlanetTreeNode* node);
	bool HandleMerge(PlanetTreeNode* node);

	void PruneTree();
	void PreprocessTree();
	void RefreshMapTile(PlanetTreeNode* node, PlanetMapTile* tile);

private:
	scythe::Renderer * renderer_;
	scythe::Shader * shader_;			//!< pointer to shader object
	scythe::CameraManager * camera_;	//!< pointer to camera object
	scythe::Frustum * frustum_;			//!< pointer to frustum object

	static constexpr int kNumFaces = 6;
	PlanetTree * faces_[kNumFaces];

	const int grid_size_;
	scythe::Vector3 planet_position_; //!< planet position
	const float radius_; //!< planet radius
	PlanetTileMesh * tile_;
	PlanetMap * map_;

	RequestQueue inline_requests_;
	RequestQueue render_requests_;
	NodeSet open_nodes_;

	LodParams lod_params_;

	int frame_counter_;
	bool lod_freeze_;
	bool tree_freeze_;
	bool preprocess_;
};

#endif