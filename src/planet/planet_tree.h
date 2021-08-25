#ifndef __PLANET_TREE_H__
#define __PLANET_TREE_H__

#include "planet_key.h"
#include "planet_map_tile.h"
#include "planet_renderable.h"

#include "math/vector3.h"
#include "common/non_copyable.h"
#include "image/image.h"

// Forward declarations
class PlanetTree;
class PlanetCube;

namespace scythe {
	class Renderer;
} // namespace scythe

//! Planet tree node class
class PlanetTreeNode : public scythe::NonCopyable {
	friend class PlanetCube;
	friend class PlanetTree;
	friend class PlanetRenderable;
	friend class PlanetMapTile;
	friend class PlanetTreeNodeCompareLastOpened;
public:
	enum Slot { TOP_LEFT, TOP_RIGHT, BOTTOM_LEFT, BOTTOM_RIGHT };

	explicit PlanetTreeNode(PlanetTree * tree);
	virtual ~PlanetTreeNode();

	const float GetPriority() const;
	const PlanetKey GetKey() const;

	scythe::Renderer * GetRenderer();

	bool IsSplit();

	void AttachChild(PlanetTreeNode * child, int position);
	void DetachChild(int position);

	void AddToNodeMap();
	void RemoveFromNodeMap();

	void PropagateLodDistances();

	void CreateMapTile();
	void DestroyMapTile();

	void CreateRenderable(PlanetMapTile* map_tile);
	void DestroyRenderable();
	void RefreshRenderable(PlanetMapTile * map_tile);

	bool WillRender();
	int Render();
	void RenderSelf();

	void OnAlbedoTaskCompleted(const scythe::Image& image, bool completed);

private:
	PlanetTree * owner_; //!< owner face tree

	PlanetMapTile map_tile_;
	PlanetRenderable renderable_;

	int lod_; //!< level of detail
	int x_;
	int y_;

	int last_rendered_;
	int last_opened_;

	bool has_map_tile_;
	bool has_renderable_;

	bool has_children_;
	bool page_out_;

	bool request_page_out_;
	bool request_map_tile_;
	bool request_renderable_;
	bool request_split_;
	bool request_merge_;

	bool request_albedo_;

	int parent_slot_;
	static constexpr int kNumChildren = 4;
	PlanetTreeNode * parent_;
	PlanetTreeNode * children_[kNumChildren];
};

//! Planet tree class
class PlanetTree : public scythe::NonCopyable {
	friend class PlanetTreeNode;
	friend class PlanetRenderable;
	friend class PlanetMapTile;
public:
	explicit PlanetTree(PlanetCube * cube, int face);
	virtual ~PlanetTree();

	void Render();

private:
	PlanetCube * cube_;
	PlanetTreeNode * root_;
	int face_;
};

// Quadtree node comparison for LOD age.
class PlanetTreeNodeCompareLastOpened {
public:
	bool operator()(const PlanetTreeNode* a, const PlanetTreeNode* b) const {
		return (a->last_opened_ > b->last_opened_);
	}
};

#endif