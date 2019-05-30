#pragma once
#ifndef __SHT_GEO_PLANET_MAP_TILE_H__
#define __SHT_GEO_PLANET_MAP_TILE_H__

namespace sht {
	namespace graphics {
		class Texture;
	} // namespace graphics
} // namespace sht

namespace sht {
	namespace geo {

		class PlanetMap;
		class PlanetTreeNode;

		class PlanetMapTile {
		public:
			PlanetMapTile(PlanetMap * map, PlanetTreeNode * node, graphics::Texture * albedo_texture);
			~PlanetMapTile();

			PlanetTreeNode * GetNode();
			void BindTexture();

		private:
			PlanetMap * map_; // owner
			PlanetTreeNode * node_;
			graphics::Texture * albedo_texture_;
		};

	} // namespace geo
} // namespace sht

#endif