#include "earth_service.h"

#include "saim.h"

EarthService::EarthService()
{
}
EarthService::~EarthService()
{
}
bool EarthService::Initialize()
{
	if (saim_init("", nullptr, 0) != 0)
		return false;
	// Create image object
	image_.Allocate(256, 256, scythe::Image::Format::kRGB8);
	// Some library settings
	saim_set_bitmap_cache_size(100);
	saim_set_target(image_.pixels(), image_.width(), image_.height(), image_.bpp());
	return true;
}
void EarthService::Deinitialize()
{
	saim_cleanup();
}
bool EarthService::Execute()
{
	// Render parameters may change, so obtain current ones
	int face, lod, x, y;
	ObtainTileParameters(&face, &lod, &x, &y);

	// Render
	return saim_render_mapped_cube(face, lod, x, y) == 0;
}