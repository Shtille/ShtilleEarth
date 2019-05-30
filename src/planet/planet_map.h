#pragma once
#ifndef __SHT_GEO_PLANET_MAP_H__
#define __SHT_GEO_PLANET_MAP_H__

namespace sht {
	namespace graphics {
		class Renderer;
		class Texture;
	} // namespace graphics
} // namespace sht

namespace sht {
	namespace geo {

		class PlanetTreeNode;
		class PlanetMapTile;
		class PlanetService;

		class PlanetMap {
		public:
			PlanetMap(PlanetService * albedo_service, graphics::Renderer * renderer);
			~PlanetMap();

			bool Initialize();

			void ResetTile();
			bool PrepareTile(PlanetTreeNode * node);
			PlanetMapTile * FinalizeTile(PlanetTreeNode * node);

			graphics::Renderer * renderer();

		private:

			void Deinitialize();

			PlanetService * albedo_service_;		//!< service for filling albedo texture data
			graphics::Renderer * renderer_;			//!< pointer to renderer object
			graphics::Texture * albedo_texture_;
			int step_;
		};

	} // namespace geo
} // namespace sht

#endif