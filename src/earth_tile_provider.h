#ifndef __EARTH_TILE_PROVIDER_H__
#define __EARTH_TILE_PROVIDER_H__

#include "planet/planet_tile_provider.h"

/*
 * Defines Earth tile provider
 */
class EarthTileProvider final : public PlanetTileProvider {
public:
	EarthTileProvider();

	bool Initialize();
	void Deinitialize();

	void SetTarget(scythe::Image * image) final;
	bool RenderTile(const PlanetKey& key) final;

private:

	struct saim_instance * saim_instance_;
};

#endif