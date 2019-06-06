#ifndef __PLANET_MAP_TILE_H__
#define __PLANET_MAP_TILE_H__

namespace scythe {
	class Texture;
} // namespace scythe

class PlanetMap;
class PlanetTreeNode;

class PlanetMapTile {
public:
	PlanetMapTile(PlanetMap * map, PlanetTreeNode * node, scythe::Texture * albedo_texture);
	~PlanetMapTile();

	PlanetTreeNode * GetNode();
	void BindTexture();

private:
	PlanetMap * map_; // owner
	PlanetTreeNode * node_;
	scythe::Texture * albedo_texture_;
};

#endif