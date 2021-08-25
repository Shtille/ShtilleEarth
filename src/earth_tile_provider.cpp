#include "earth_tile_provider.h"

#include "image/image.h"

#include "saim.h"

EarthTileProvider::EarthTileProvider()
: saim_instance_(nullptr)
{

}
bool EarthTileProvider::Initialize()
{
	int result;
	saim_instance_ = saim_init(
		"", // path
		nullptr, // provider info
		0, // flags
		1, // service count
		&result); // error
	if (result != 0)
		return false;
	// Some library settings
	saim_set_bitmap_cache_size(saim_instance_, 100);
	return true;
}
void EarthTileProvider::Deinitialize()
{
	saim_cleanup(saim_instance_);
}
void EarthTileProvider::SetTarget(scythe::Image * image)
{
	saim_set_target(saim_instance_, image->pixels(), image->width(), image->height(), image->bpp());
}
bool EarthTileProvider::RenderTile(const PlanetKey& key)
{
	return saim_render_mapped_cube(saim_instance_, key.face(), key.lod(), key.x(), key.y()) == 0;
}