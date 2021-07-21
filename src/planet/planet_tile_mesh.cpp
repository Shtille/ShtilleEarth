#include "planet_tile_mesh.h"

PlanetTileMesh::PlanetTileMesh(scythe::Renderer * renderer, int grid_size)
	: scythe::MeshPart(renderer)
	, grid_size_(grid_size)
{
	primitive_mode_ = scythe::PrimitiveType::kTriangles;
}
PlanetTileMesh::~PlanetTileMesh()
{

}
void PlanetTileMesh::Create()
{
	const unsigned int uGridSize = static_cast<unsigned int>(grid_size_);
	const int iGridSizeMinusOne = grid_size_ - 1;
	const float fGridSizeMinusOne = (float) iGridSizeMinusOne;

	vertices_.resize(uGridSize * uGridSize + uGridSize * 4);
	indices_.resize(((uGridSize - 1) * (uGridSize - 1) + 4 * (uGridSize - 1)) * 6);

	volatile unsigned int index = 0;
	// Output vertex data for regular grid.
	for (int j = 0; j < grid_size_; ++j)
	{
		for (int i = 0; i < grid_size_; ++i)
		{
			float x = (float) i / fGridSizeMinusOne;
			float y = (float) j / fGridSizeMinusOne;

			vertices_[index].position.x = x;
			vertices_[index].position.y = y;
			vertices_[index].position.z = 0.0f;
			++index;
		}
	}
	// Output vertex data for x skirts.
	for (int j = 0; j < grid_size_; j += iGridSizeMinusOne)
	{
		for (int i = 0; i < grid_size_; ++i)
		{
			float x = (float) i / fGridSizeMinusOne;
			float y = (float) j / fGridSizeMinusOne;

			vertices_[index].position.x = x;
			vertices_[index].position.y = y;
			vertices_[index].position.z = -1.0f;
			++index;
		}
	}
	// Output vertex data for y skirts.
	for (int i = 0; i < grid_size_; i += iGridSizeMinusOne)
	{
		for (int j = 0; j < grid_size_; ++j)
		{
			float x = (float) i / fGridSizeMinusOne;
			float y = (float) j / fGridSizeMinusOne;

			vertices_[index].position.x = x;
			vertices_[index].position.y = y;
			vertices_[index].position.z = -1.0f;
			++index;
		}
	}

	unsigned int * pIndex = &indices_[0];

	index = 0;
	volatile unsigned int skirt_index = 0;
	// Output indices for regular surface.
	for (int j = 0; j < iGridSizeMinusOne; ++j)
	{
		for (int i = 0; i < iGridSizeMinusOne; ++i)
		{
			*pIndex++ = index;
			*pIndex++ = index + uGridSize;
			*pIndex++ = index + 1;

			*pIndex++ = index + uGridSize;
			*pIndex++ = index + uGridSize + 1;
			*pIndex++ = index + 1;

			++index;
		}
		++index;
	}
	// Output indices for x skirts.
	index = 0;
	skirt_index = uGridSize * uGridSize;
	for (int i = 0; i < iGridSizeMinusOne; ++i)
	{
		*pIndex++ = index;
		*pIndex++ = index + 1;
		*pIndex++ = skirt_index;

		*pIndex++ = skirt_index;
		*pIndex++ = index + 1;
		*pIndex++ = skirt_index + 1;

		index++;
		skirt_index++;
	}
	index = uGridSize * (uGridSize - 1);
	skirt_index = uGridSize * (uGridSize + 1);
	for (int i = 0; i < iGridSizeMinusOne; ++i)
	{
		*pIndex++ = index;
		*pIndex++ = skirt_index;
		*pIndex++ = index + 1;

		*pIndex++ = skirt_index;
		*pIndex++ = skirt_index + 1;
		*pIndex++ = index + 1;

		index++;
		skirt_index++;
	}
	// Output indices for y skirts.
	index = 0;
	skirt_index = uGridSize * (uGridSize + 2);
	for (int i = 0; i < iGridSizeMinusOne; ++i)
	{
		*pIndex++ = index;
		*pIndex++ = skirt_index;
		*pIndex++ = index + uGridSize;

		*pIndex++ = skirt_index;
		*pIndex++ = skirt_index + 1;
		*pIndex++ = index + uGridSize;

		index += uGridSize;
		skirt_index++;
	}
	index = (uGridSize - 1);
	skirt_index = uGridSize * (uGridSize + 3);
	for (int i = 0; i < iGridSizeMinusOne; ++i)
	{
		*pIndex++ = index;
		*pIndex++ = index + uGridSize;
		*pIndex++ = skirt_index;

		*pIndex++ = skirt_index;
		*pIndex++ = index + uGridSize;
		*pIndex++ = skirt_index + 1;

		index += uGridSize;
		skirt_index++;
	}
}