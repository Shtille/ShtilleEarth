#ifndef __PLANET_TASK_FACTORY_H__
#define __PLANET_TASK_FACTORY_H__

#include "planet_key.h"
#include "planet_task.h"

#include "common/non_copyable.h"
#include "memory/pool_allocator.h"

class PlanetTileProvider;

class PlanetTaskFactory : public scythe::NonCopyable {
public:
	PlanetTaskFactory(PlanetTileProvider * tile_provider);
	~PlanetTaskFactory();

	PlanetTask * CreateAlbedoTask(const PlanetKey& key);

	void DestroyTask(PlanetTask * task);

private:
	PlanetTileProvider * tile_provider_;
	scythe::PoolAllocator task_allocator_;
	scythe::PoolAllocator image_allocator_;
	const size_t max_task_size_;
};

#endif