#include "planet_map_tile.h"

#include "planet_tree.h"

#include "graphics/renderer.h"

PlanetMapTile::PlanetMapTile()
: node_(nullptr)
, albedo_texture_(nullptr)
{
}
PlanetMapTile::PlanetMapTile(PlanetTreeNode * node)
: node_(node)
, albedo_texture_(nullptr)
{
}
PlanetMapTile::~PlanetMapTile()
{
	Destroy();
}
void PlanetMapTile::Create(PlanetTreeNode * node)
{
	node_ = node;
}
void PlanetMapTile::Destroy()
{
	scythe::Renderer * renderer = node_->GetRenderer();
	if (albedo_texture_)
	{
		renderer->DeleteTexture(albedo_texture_);
		albedo_texture_ = nullptr;
	}
}
void PlanetMapTile::SetNode(PlanetTreeNode * node)
{
	node_ = node;
}
PlanetTreeNode * PlanetMapTile::GetNode()
{
	return node_;
}
void PlanetMapTile::BindTexture()
{
	scythe::Renderer * renderer = node_->GetRenderer();
	renderer->ChangeTexture(albedo_texture_, 0U);
}
bool PlanetMapTile::HasAlbedoTexture() const
{
	return albedo_texture_ != nullptr;
}
void PlanetMapTile::SetAlbedoImage(const scythe::Image& image)
{
	if (albedo_texture_)
	{
		// TODO:
		//albedo_texture_->SetData(0, 0, image.width(), image.height(), image.pixels());
	}
	else
	{
		scythe::Renderer * renderer = node_->GetRenderer();
		renderer->AddTextureFromImage(albedo_texture_, image, scythe::Texture::Wrap::kClampToEdge);
	}
}