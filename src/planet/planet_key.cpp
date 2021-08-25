#include "planet_key.h"

// #include <cstring>

PlanetKey::PlanetKey(int face, int lod, int x, int y)
: face_(face)
, lod_(lod)
, x_(x)
, y_(y)
{

}
const int PlanetKey::face() const
{
	return face_;
}
const int PlanetKey::lod() const
{
	return lod_;
}
const int PlanetKey::x() const
{
	return x_;
}
const int PlanetKey::y() const
{
	return y_;
}
bool PlanetKey::operator <(const PlanetKey& other) const
{
	// return std::memcmp(this, &other, sizeof(PlanetKey)) < 0;
	if (face_ == other.face_)
	{
		if (lod_ == other.lod_)
		{
			if (x_ == other.x_)
				return y_ < other.y_;
			else
				return x_ < other.x_;
		}
		else
			return lod_ < other.lod_;
	}
	else
		return face_ < other.face_;
}
bool PlanetKey::operator ==(const PlanetKey& other) const
{
	return face_ == other.face_ && 
	       lod_ == other.lod_ &&
	       x_ == other.x_ &&
	       y_ == other.y_;
}