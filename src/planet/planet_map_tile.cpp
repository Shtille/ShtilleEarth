#include "planet_map_tile.h"
#include "planet_map.h"

#include "graphics/renderer.h"

PlanetMapTile::PlanetMapTile(PlanetMap * map, PlanetTreeNode * node, scythe::Texture * albedo_texture)
	: map_(map)
	, node_(node)
	, albedo_texture_(albedo_texture)
{
}
PlanetMapTile::~PlanetMapTile()
{
	map_->renderer()->DeleteTexture(albedo_texture_);
}
PlanetTreeNode * PlanetMapTile::GetNode()
{
	return node_;
}
void PlanetMapTile::BindTexture()
{
	map_->renderer()->ChangeTexture(albedo_texture_, 0U);
}