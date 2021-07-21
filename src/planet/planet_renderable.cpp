#include "planet_renderable.h"
#include "planet_tree.h"
#include "planet_cube.h"
#include "planet_map_tile.h"

#include "math/frustum.h"
#include "math/constants.h"
#include "math/matrix3.h"

#include <cmath>
#include <algorithm>
#include <assert.h>

PlanetRenderable::PlanetRenderable(PlanetTreeNode * node, PlanetMapTile * map_tile)
	: node_(node)
	, map_tile_(map_tile)
	, child_distance_(0.0f)
{
	AnalyzeTerrain();
	InitDisplacementMapping();
}
PlanetRenderable::~PlanetRenderable()
{

}
void PlanetRenderable::SetFrameOfReference()
{
	const float planet_radius = node_->owner_->cube_->radius();
	LodParams& params = node_->owner_->cube_->lod_params_;
	scythe::Frustum * frustum = node_->owner_->cube_->frustum_;

	// Bounding box clipping.
	is_clipped_ = !frustum->Intersects(bounding_box_);

	// Spherical distance map clipping.
	float point_dot_n = params.camera_position & surface_normal_;
	float cos_camera_angle = point_dot_n / params.camera_distance;
	// We should exclude collinear cases
	if (cos_camera_angle > 0.99f)
	{
		// Always visible
		is_far_away_ = false;
	}
	else if (cos_camera_angle < -0.9f)
	{
		// Always invisible
		is_far_away_ = true;
	}
	else if (cos_camera_angle > cos_sector_angle_)
	{
		// Always visible
		is_far_away_ = false;
	}
	else
	{
		// Normal case, we have to compute visibility
		scythe::Vector3 side, normal;
		side = params.camera_position - point_dot_n * surface_normal_;
		side.Normalize();
		normal = surface_normal_ * cos_sector_angle_ + side * sin_sector_angle_;
		is_far_away_ = !((params.camera_position & normal) > planet_radius);
	}
	is_clipped_ = is_clipped_ || is_far_away_;

	// Get vector from center to camera and normalize it.
	scythe::Vector3 position_offset = params.camera_position - center_;
	scythe::Vector3 view_direction = position_offset;

	// Find the offset between the center of the grid and the grid point closest to the camera (rough approx).
	const float reference_length = scythe::kPi * 0.375f * planet_radius / (float)(1 << node_->lod_);
	scythe::Vector3 reference_offset = view_direction - ((view_direction & surface_normal_) * surface_normal_);
	if (reference_offset.LengthSquared() > reference_length * reference_length)
	{
		reference_offset.Normalize();
		reference_offset *= reference_length;
	}

	// Find the position offset to the nearest point to the camera (approx).
	scythe::Vector3 near_position_offset = position_offset - reference_offset;
	float near_position_distance = near_position_offset.Length();
	scythe::Vector3 to_camera = near_position_offset / near_position_distance;
	scythe::Vector3 nearest_point_normal = reference_offset + center_;
	nearest_point_normal.Normalize();

	// Determine LOD priority.
	lod_priority_ = -(to_camera & params.camera_front);

	is_in_lod_range_ = GetLodDistance() * params.geo_factor < near_position_distance;

	// Calculate texel resolution relative to near grid-point (approx).
	float cos_angle = nearest_point_normal & to_camera; // tile incination angle
	float face_size = cos_angle * planet_radius * scythe::kPi * 0.5f; // Curved width/height of texture cube face on the sphere
	float cube_side_pixels = static_cast<float>(256 << map_tile_->GetNode()->lod_);
	float texel_size = face_size / cube_side_pixels; // Size of a single texel in world units

	is_in_mip_range_ = texel_size * params.tex_factor < near_position_distance;
}
const bool PlanetRenderable::IsInLODRange() const
{
	return is_in_lod_range_;
}
const bool PlanetRenderable::IsClipped() const
{
	return is_clipped_;
}
const bool PlanetRenderable::IsInMIPRange() const
{
	return is_in_mip_range_;
}
const bool PlanetRenderable::IsFarAway() const
{
	return is_far_away_;
}
float PlanetRenderable::GetLodPriority() const
{
	return lod_priority_;
}
float PlanetRenderable::GetLodDistance()
{
	if (distance_ > child_distance_)
		return distance_;
	else
		return child_distance_;
}
void PlanetRenderable::SetChildLodDistance(float lod_distance)
{
	child_distance_ = lod_distance;
}
PlanetMapTile * PlanetRenderable::GetMapTile()
{
	return map_tile_;
}
void PlanetRenderable::AnalyzeTerrain()
{
	const int grid_size = node_->owner_->cube_->grid_size();
	const scythe::Vector3& planet_position = node_->owner_->cube_->planet_position_;
	const float planet_radius = node_->owner_->cube_->radius();
	scythe::Matrix3 face_transform = PlanetCube::GetFaceTransform(node_->owner_->face_);

	// Calculate scales, offsets for tile position on cube face.
	const float inv_scale = 2.0f / (float)(1 << node_->lod_);
	const float position_x = -1.f + inv_scale * node_->x_;
	const float position_y = -1.f + inv_scale * node_->y_;

	// Keep track of extents.
	scythe::Vector3 min = scythe::Vector3(1e8), max = scythe::Vector3(-1e8);
	center_.Set(0.0f, 0.0f, 0.0f);

	// Lossy representation of heightmap
	lod_difference_ = 0.0f;

	// Calculate LOD error of sphere.
	float angle = scythe::kPi / (grid_size << std::max<int>(0, node_->lod_ - 1));
	float sphere_error = (1 - cosf(angle)) * 1.4f * planet_radius;
	distance_ = sphere_error;

	// Process vertex data for regular grid.
	for (int j = 0; j < grid_size; j++)
	{
		for (int i = 0; i < grid_size; i++)
		{
			float x = (float)i / (float)(grid_size - 1);
			float y = (float)j / (float)(grid_size - 1);

			scythe::Vector3 sphere_point(x * inv_scale + position_x, y * inv_scale + position_y, 1.0f);
			sphere_point.Normalize();
			sphere_point = face_transform * sphere_point;
			sphere_point *= planet_radius;

			center_ += sphere_point;

			min.MakeMinimum(sphere_point);
			max.MakeMaximum(sphere_point);
		}
	}

	// Calculate center.
	center_ = center_ / (float)(grid_size * grid_size);
	surface_normal_ = center_;
	surface_normal_.Normalize();

	// Set bounding box (it should be in global coordinates)
	min += planet_position;
	max += planet_position;
	bounding_box_.Set(min, max);

	// Calculate sector angles
	scythe::Vector3 corner_points[4];
	corner_points[0].Set(position_x, position_y, 1.0f); // (0,0)
	corner_points[1].Set(position_x, inv_scale + position_y, 1.0f); // (0,1)
	corner_points[2].Set(inv_scale + position_x, position_y, 1.0f); // (1,0)
	corner_points[3].Set(inv_scale + position_x, inv_scale + position_y, 1.0f); // (1,1)
	float cos_angle = 1.0f;
	for (unsigned int i = 0; i < _countof(corner_points); ++i)
	{
		scythe::Vector3& corner_point = corner_points[i];
		corner_point.Normalize();
		corner_point = face_transform * corner_point;
		float dot = corner_point & surface_normal_;
		if (dot < cos_angle)
			cos_angle = dot;
	}
	// Because angle is always less than Pi/2 thus we may easy compute sin(angle)
	cos_sector_angle_ = cos_angle;
	sin_sector_angle_ = sqrtf(1.0f - cos_angle * cos_angle);
}
void PlanetRenderable::InitDisplacementMapping()
{
	// Calculate scales, offsets for tile position on cube face.
	const float inv_scale = 2.0f / (1 << node_->lod_);
	const float position_x = -1.f + inv_scale * node_->x_;
	const float position_y = -1.f + inv_scale * node_->y_;

	// Correct for GL texture mapping at borders.
	const float uv_correction = 0.0f; //.05f / (mMap->getWidth() + 1);

	// Calculate scales, offset for tile position in map tile.
	int relative_lod = node_->lod_ - map_tile_->GetNode()->lod_;
	const float inv_tex_scale = 1.0f / (1 << relative_lod) * (1.0f - uv_correction);
	const float texture_x = inv_tex_scale * (node_->x_ - (map_tile_->GetNode()->x_ << relative_lod)) + uv_correction;
	const float texture_y = inv_tex_scale * (node_->y_ - (map_tile_->GetNode()->y_ << relative_lod)) + uv_correction;

	// Set shader parameters
	stuv_scale_.x = inv_scale;
	stuv_scale_.y = inv_scale;
	stuv_scale_.z = inv_tex_scale;
	stuv_scale_.w = inv_tex_scale;
	stuv_position_.x = position_x;
	stuv_position_.y = position_y;
	stuv_position_.z = texture_x;
	stuv_position_.w = texture_y;

	// Set color/tint
	//color_.x = cosf(map_tile_->GetNode()->lod_ * 0.70f) * .35f + .85f;
	//color_.y = cosf(map_tile_->GetNode()->lod_ * 1.71f) * .35f + .85f;
	//color_.z = cosf(map_tile_->GetNode()->lod_ * 2.64f) * .35f + .85f;
	//color_.w = 1.0f;
	color_ = scythe::Vector4(1.0f);
}