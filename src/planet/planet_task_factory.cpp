#include "planet_task_factory.h"

#include "planet_albedo_task.h"

#include "image/image.h"
#include "common/sc_assert.h"
#include "math/common_math.h"

#include <new>

static size_t CalculateMaximumTaskSize()
{
	size_t size = 0U;
	size = scythe::Max(size, sizeof(PlanetAlbedoTask));
	return size;
}

PlanetTaskFactory::PlanetTaskFactory(PlanetTileProvider * tile_provider)
: tile_provider_(tile_provider)
, task_allocator_(32)
, image_allocator_(32)
, max_task_size_(CalculateMaximumTaskSize())
{

}
PlanetTaskFactory::~PlanetTaskFactory()
{

}
PlanetTask * PlanetTaskFactory::CreateAlbedoTask(const PlanetKey& key)
{
	// Allocate image
	scythe::Image * image = (scythe::Image *) image_allocator_.Allocate(sizeof(scythe::Image));
	new (image) scythe::Image();
	// TODO: make possible custom allocation of image data
	image->Allocate(256, 256, scythe::Image::Format::kRGB8);

	// Allocate task
	PlanetTask * task = (PlanetTask *) task_allocator_.Allocate(max_task_size_);
	new (task) PlanetAlbedoTask(key, tile_provider_, image);	

	return task;
}
void PlanetTaskFactory::DestroyTask(PlanetTask * task)
{
	switch (task->type())
	{
	case PlanetTaskType::kAlbedo:
		{
			PlanetAlbedoTask * albedo_task = dynamic_cast<PlanetAlbedoTask *>(task);
			scythe::Image * image = albedo_task->image();
			image->~Image();
			image_allocator_.Free(image);
			albedo_task->~PlanetAlbedoTask();
			task_allocator_.Free(task);
		}
		break;
	default:
		SC_ASSERT(!"Unimplemlemented type for task in PlanetTaskFactory::DestroyTask");
		break;
	}
}