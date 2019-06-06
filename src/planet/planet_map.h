#ifndef __PLANET_MAP_H__
#define __PLANET_MAP_H__

namespace scythe {
	class Renderer;
	class Texture;
} // namespace scythe

class PlanetTreeNode;
class PlanetMapTile;
class PlanetService;

class PlanetMap {
public:
	PlanetMap(PlanetService * albedo_service, scythe::Renderer * renderer);
	~PlanetMap();

	bool Initialize();

	void ResetTile();
	bool PrepareTile(PlanetTreeNode * node);
	PlanetMapTile * FinalizeTile(PlanetTreeNode * node);

	scythe::Renderer * renderer();

private:

	void Deinitialize();

	PlanetService * albedo_service_;		//!< service for filling albedo texture data
	scythe::Renderer * renderer_;			//!< pointer to renderer object
	scythe::Texture * albedo_texture_;
	int step_;
};

#endif