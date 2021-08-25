#ifndef __PLANET_MAP_TILE_H__
#define __PLANET_MAP_TILE_H__

namespace scythe {
	class Texture;
	class Image;
} // namespace scythe

class PlanetTreeNode;

class PlanetMapTile {
public:
	PlanetMapTile();
	PlanetMapTile(PlanetTreeNode * node);
	~PlanetMapTile();

	void Create(PlanetTreeNode * node);
	void Destroy();

	void SetNode(PlanetTreeNode * node);
	PlanetTreeNode * GetNode();
	void BindTexture();

	bool HasAlbedoTexture() const;

	void SetAlbedoImage(const scythe::Image& image);

private:
	PlanetTreeNode * node_;
	scythe::Texture * albedo_texture_;
};

#endif