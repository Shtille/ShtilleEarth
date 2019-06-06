#include "planet_map.h"
#include "planet_map_tile.h"
#include "planet_tree.h"
#include "planet_service.h"

#include "graphics/renderer.h"

PlanetMap::PlanetMap(PlanetService * albedo_service, scythe::Renderer * renderer)
	: albedo_service_(albedo_service)
	, renderer_(renderer)
	, albedo_texture_(nullptr)
	, step_(0)
{
}
PlanetMap::~PlanetMap()
{
	Deinitialize();
}
bool PlanetMap::Initialize()
{
	if (!albedo_service_->Initialize())
		return false;
	// Run services
	albedo_service_->RunService();
	return true;
}
void PlanetMap::Deinitialize()
{
	albedo_service_->StopService();
	albedo_service_->Deinitialize();
}
void PlanetMap::ResetTile()
{
	if (step_ > 1)
		renderer_->DeleteTexture(albedo_texture_);
	step_ = 0;
}
bool PlanetMap::PrepareTile(PlanetTreeNode * node)
{
	bool step_done = true;
	switch (step_)
	{
	case 0:
		// Albedo texture generation step
		step_done = albedo_service_->CheckRegionStatus(node->owner_->face_, node->lod_, node->x_, node->y_);
		break;
	case 1:
		// Albedo texture creation step
		renderer_->AddTextureFromImage(albedo_texture_, albedo_service_->image(), scythe::Texture::Wrap::kClampToEdge);
		break;
	// case 3: Height texture generation step
	}
	if (step_done)
		++step_;
	return step_ > 1;
}
PlanetMapTile * PlanetMap::FinalizeTile(PlanetTreeNode * node)
{
	step_ = 0;
	return new PlanetMapTile(this, node, albedo_texture_);
}
scythe::Renderer * PlanetMap::renderer()
{
	return renderer_;
}