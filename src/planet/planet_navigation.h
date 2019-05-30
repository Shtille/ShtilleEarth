#pragma once
#ifndef __SHT_GEO_PLANET_NAVIGATION_H__
#define __SHT_GEO_PLANET_NAVIGATION_H__

#include "../../math/vector.h"
#include "../../math/matrix.h"

// Forward declarations
namespace sht {
    namespace utility {
        class CameraManager;
    }
}

namespace sht {
	namespace geo {

		class PlanetNavigation {
		public:
			PlanetNavigation(utility::CameraManager * camera_manager, float planet_radius, float farest_distance, float nearest_distance);
			~PlanetNavigation();

            void InstantZoomIn();
            void InstantZoomOut();
			void SmoothZoomIn();
			void SmoothZoomOut();
            
            void InstantRotation(float angle_x);
            void SmoothRotation(float angle_x);
            
            void ObtainZNearZFar(float * znear, float * zfar) const;
            
            void PanBegin(float screen_x, float screen_y,
                          const math::Vector4& viewport, const math::Matrix4& proj, const math::Matrix4& view);
            void PanMove(float screen_x, float screen_y,
                         const math::Vector4& viewport, const math::Matrix4& proj, const math::Matrix4& view);
            void PanEnd();
            
        protected:
            math::Vector3 MakePoint(float distance);
            void ProcessZoomInstant();
            void ProcessZoomSmooth();

		private:
            utility::CameraManager * camera_manager_;
			int num_scales_; //!< number of scales
            int current_scale_index_; //!< current scale index
			float * distances_; //!< array of distances
            math::Vector3 pan_point_; //!< point that stay always under mouse during paning
            bool is_pan_mode_; //!< is pan mode enabled
		};

	} // namespace geo
} // namespace sht

#endif