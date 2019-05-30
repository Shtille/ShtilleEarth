#include "../include/planet_navigation.h"
#include "../include/constants.h"
#include "../../utility/include/camera.h"
#include "../../math/sht_math.h"
#include <cmath>

namespace {
    const float kAnimationTime = 1.0f; // seconds
}

namespace sht {
	namespace geo {

		PlanetNavigation::PlanetNavigation(utility::CameraManager * camera_manager, float planet_radius, float farest_distance, float nearest_distance)
        : camera_manager_(camera_manager)
        , current_scale_index_(0)
        , is_pan_mode_(false)
		{
			num_scales_ = static_cast<int>(log(farest_distance/nearest_distance)/log(2)) + 1;
			distances_ = new float[num_scales_];
			float distance = farest_distance;
			for (int i = 0; i < num_scales_; ++i)
			{
				distances_[i] = distance;
				distance *= 0.5f;
			}
            // Make static camera at farest point
            camera_manager_->MakeFree(MakePoint(farest_distance), kEarthPosition);
		}
		PlanetNavigation::~PlanetNavigation()
		{
			delete[] distances_;
		}
        void PlanetNavigation::InstantZoomIn()
        {
            if (current_scale_index_ + 1 < num_scales_)
            {
                ++current_scale_index_;
                ProcessZoomInstant();
            }
        }
        void PlanetNavigation::InstantZoomOut()
        {
            if (current_scale_index_ > 0)
            {
                --current_scale_index_;
                ProcessZoomInstant();
            }
        }
		void PlanetNavigation::SmoothZoomIn()
		{
            if (current_scale_index_ + 1 < num_scales_)
            {
                ++current_scale_index_;
                ProcessZoomSmooth();
            }
		}
		void PlanetNavigation::SmoothZoomOut()
		{
            if (current_scale_index_ > 0)
            {
                --current_scale_index_;
                ProcessZoomSmooth();
            }
		}
        void PlanetNavigation::InstantRotation(float angle_x)
        {
            // Reset any camera animation first
            camera_manager_->PathClear();
            camera_manager_->Clear();
            camera_manager_->RotateAroundTargetInX(angle_x);
        }
        void PlanetNavigation::SmoothRotation(float angle_x)
        {
            const math::Vector3& pos = *camera_manager_->position();
            const math::Quaternion& orient = *camera_manager_->orientation();
            math::Quaternion new_orient = orient * math::Quaternion(UNIT_X, angle_x);
            new_orient.Normalize();
            camera_manager_->Clear();
            auto first_camera = camera_manager_->AddAsCurrent();
            auto second_camera = camera_manager_->Add(pos, new_orient, kEarthPosition);
            camera_manager_->PathClear();
            camera_manager_->PathSetStart(first_camera, 0.0f);
            camera_manager_->PathAdd(second_camera, kAnimationTime);
        }
        void PlanetNavigation::PanBegin(float screen_x, float screen_y,
                                  const math::Vector4& viewport, const math::Matrix4& proj, const math::Matrix4& view)
        {
            math::Vector2 screen_point(screen_x, screen_y);
            math::Vector3 ray;
            math::ScreenToRay(screen_point, viewport, proj, view, ray);
            const math::Vector3& origin = *camera_manager_->position();
            is_pan_mode_ = math::RaySphereIntersection(origin, ray, kEarthPosition, kEarthRadius, pan_point_);
            // Clear all animations
            camera_manager_->PathClear();
            camera_manager_->Clear();
        }
        void PlanetNavigation::PanMove(float screen_x, float screen_y,
                                 const math::Vector4& viewport, const math::Matrix4& proj, const math::Matrix4& view)
        {
            if (!is_pan_mode_)
                return;
            
            math::Vector2 screen_point(screen_x, screen_y);
            math::Vector3 ray;
            math::ScreenToRay(screen_point, viewport, proj, view, ray);
            const math::Vector3& origin = *camera_manager_->position();
            math::Vector3 new_point;
            if (math::RaySphereIntersection(origin, ray, kEarthPosition, kEarthRadius, new_point))
            {
                math::Vector3 first_vector, second_vector;
                float dot_product;
                // Forward should vector should point down, up and side will be tangent to sphere
                // We will rotate in two planes
                math::Vector3 normal1 = camera_manager_->GetUp();
                math::Vector3 normal2 = camera_manager_->GetSide();
                math::Vector3 distance1 = (new_point - pan_point_) & normal1;
                math::Vector3 projected_point = new_point - distance1 * normal1;
                
                math::Vector3 plane_point1 = (pan_point_ & normal1) * normal1 + kEarthPosition;
                first_vector = pan_point_ - plane_point1;
                first_vector.Normalize();
                second_vector = projected_point - plane_point1;
                second_vector.Normalize();
                dot_product = first_vector & second_vector;
                if (dot_product > 1.0f)
                    dot_product = 1.0f;
                float angle1 = acosf(dot_product);
                if (((projected_point - pan_point_) & normal2) < 0.0f)
                    angle1 = -angle1;
                
                math::Vector3 plane_point2 = (new_point & normal2) * normal2 + kEarthPosition;
                first_vector = projected_point - plane_point2;
                first_vector.Normalize();
                second_vector = new_point - plane_point2;
                second_vector.Normalize();
                dot_product = first_vector & second_vector;
                if (dot_product > 1.0f)
                    dot_product = 1.0f;
                float angle2 = acosf(dot_product);
                if (((projected_point - new_point) & normal1) < 0.0f)
                    angle2 = -angle2;

                math::Quaternion transform1(UNIT_Y, angle1);
                math::Quaternion transform2(UNIT_Z, angle2);
                math::Quaternion total_transform = transform1 * transform2;
                math::Quaternion inverse_transform = total_transform.GetInverse();

                math::Quaternion new_orient = *camera_manager_->orientation() * inverse_transform;
                new_orient.Normalize();
                const math::Vector3& cam_pos = *camera_manager_->position();
                float distance = cam_pos.Distance(kEarthPosition);
                math::Vector3 new_pos = kEarthPosition - distance * new_orient.Direction();
                camera_manager_->MakeFreeTargeted(new_pos, new_orient, kEarthPosition);
            }
        }
        void PlanetNavigation::PanEnd()
        {
            is_pan_mode_ = false;
        }
        void PlanetNavigation::ObtainZNearZFar(float * znear, float * zfar) const
        {
            const sht::math::Vector3* cam_pos = camera_manager_->position();
            const sht::math::Vector3 to_earth = kEarthPosition - *cam_pos;
            const sht::math::Vector3 camera_direction = camera_manager_->GetDirection(); // normalized
            const float cam_distance = to_earth & camera_direction;
            *znear = cam_distance - kEarthRadius;
            *zfar = cam_distance + kEarthRadius;
        }
        math::Vector3 PlanetNavigation::MakePoint(float distance)
        {
            return math::Vector3(kEarthRadius + distance, 0.0f, 0.0f);
        }
        void PlanetNavigation::ProcessZoomInstant()
        {
            float distance = distances_[current_scale_index_];
            const math::Vector3& pos = *camera_manager_->position();
            const math::Quaternion& orient = *camera_manager_->orientation();
            math::Vector3 from_earth = pos - kEarthPosition;
            from_earth.Normalize();
            math::Vector3 new_pos = kEarthPosition + (kEarthRadius + distance) * from_earth;
            camera_manager_->PathClear();
            camera_manager_->Clear();
            camera_manager_->MakeFreeTargeted(new_pos, orient, kEarthPosition);
        }
        void PlanetNavigation::ProcessZoomSmooth()
        {
            // Add two cameras: at current position and at specified distance from Earth
            float distance = distances_[current_scale_index_];
            const math::Vector3& pos = *camera_manager_->position();
            const math::Quaternion& orient = *camera_manager_->orientation();
            math::Vector3 from_earth = pos - kEarthPosition;
            from_earth.Normalize();
            math::Vector3 new_pos = kEarthPosition + (kEarthRadius + distance) * from_earth;
            camera_manager_->Clear();
            auto first_camera = camera_manager_->AddAsCurrent();
            auto second_camera = camera_manager_->Add(new_pos, orient, kEarthPosition);
            camera_manager_->PathClear();
            camera_manager_->PathSetStart(first_camera, 0.0f);
            camera_manager_->PathAdd(second_camera, kAnimationTime);
        }

	}
}