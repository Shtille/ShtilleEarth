#pragma once
#ifndef __SHT_GEO_PLANET_TILE_MESH_H__
#define __SHT_GEO_PLANET_TILE_MESH_H__

#include "../../graphics/include/model/model.h"

namespace sht {
	namespace geo {

		//! Planet tile mesh class
		class PlanetTileMesh final : public graphics::Model {
		public:
			explicit PlanetTileMesh(graphics::Renderer * renderer, int grid_size);
			virtual ~PlanetTileMesh();

			void Create();

		private:
			const int grid_size_;
		};

	} // namespace geo
} // namespace sht

#endif