#pragma once
#ifndef __SHT_GEO_PLANET_RENDERABLE_H__
#define __SHT_GEO_PLANET_RENDERABLE_H__

#include "../../math/vector.h"
#include "../../math/bounding_box.h"

namespace sht {
	namespace geo {

		class PlanetTreeNode;
		class PlanetMapTile;

		//!
		class PlanetRenderable {
		public:
			PlanetRenderable(PlanetTreeNode * node, PlanetMapTile * map_tile);
			~PlanetRenderable();

			void SetFrameOfReference();

			const bool IsInLODRange() const;
			const bool IsClipped() const;
			const bool IsInMIPRange() const;
			const bool IsFarAway() const;
			float GetLodPriority() const;
			float GetLodDistance();
			void SetChildLodDistance(float lod_distance);

			PlanetMapTile * GetMapTile();

			// Additional params to map to shader
			math::Vector4 stuv_scale_;
			math::Vector4 stuv_position_;
			math::Vector4 color_;
			float distance_;

		private:
			void AnalyzeTerrain();
			void InitDisplacementMapping();

			const PlanetTreeNode * node_;
			PlanetMapTile* map_tile_;

			math::BoundingBox bounding_box_; //!< bounding box in world coordinates
			float cos_sector_angle_;
			float sin_sector_angle_;

			math::Vector3 center_;
			math::Vector3 surface_normal_;

			float lod_priority_; //!< priority for nodes queue processing
			float child_distance_;
			float lod_difference_;

			bool is_in_lod_range_;
			bool is_in_mip_range_;
			bool is_clipped_;
			bool is_far_away_;
		};

	} // namespace geo
} // namespace sht

#endif