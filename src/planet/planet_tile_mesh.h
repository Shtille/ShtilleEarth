#ifndef __PLANET_TILE_MESH_H__
#define __PLANET_TILE_MESH_H__

#include "model/mesh_part.h"

//! Planet tile mesh class
class PlanetTileMesh final : public scythe::MeshPart {
public:
	explicit PlanetTileMesh(scythe::Renderer * renderer, int grid_size);
	virtual ~PlanetTileMesh();

	void Create();

private:
	const int grid_size_;
};

#endif