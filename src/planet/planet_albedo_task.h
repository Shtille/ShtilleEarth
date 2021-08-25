#ifndef __PLANET_ALBEDO_TASK_H__
#define __PLANET_ALBEDO_TASK_H__

#include "planet_task.h"

// Forward declarations
class PlanetTileProvider;
namespace scythe {
	class Image;
}

/*
 * Defines albedo task
 */
class PlanetAlbedoTask final : public PlanetTask {
public:
	PlanetAlbedoTask(const PlanetKey& key, PlanetTileProvider * tile_provider, scythe::Image * image);

	void Execute() final;

	scythe::Image * image();
	const scythe::Image * image() const;

private:
	PlanetTileProvider * tile_provider_;
	scythe::Image * image_;
};

#endif