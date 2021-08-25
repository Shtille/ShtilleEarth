#include "planet_albedo_task.h"
#include "planet_tile_provider.h"

PlanetAlbedoTask::PlanetAlbedoTask(const PlanetKey& key, PlanetTileProvider * tile_provider, scythe::Image * image)
: PlanetTask(key, PlanetTaskType::kAlbedo)
, tile_provider_(tile_provider)
, image_(image)
{

}
void PlanetAlbedoTask::Execute()
{
	tile_provider_->SetTarget(image_);
	completed_ = tile_provider_->RenderTile(key_);
}
scythe::Image * PlanetAlbedoTask::image()
{
	return image_;
}
const scythe::Image * PlanetAlbedoTask::image() const
{
	return image_;
}