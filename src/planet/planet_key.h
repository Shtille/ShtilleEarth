#ifndef __PLANET_KEY_H__
#define __PLANET_KEY_H__

#include <functional>

/**
 * Defines key class
 */
class PlanetKey {
public:
	PlanetKey(int face, int lod, int x, int y);

	const int face() const;
	const int lod() const;
	const int x() const;
	const int y() const;

	bool operator <(const PlanetKey& other) const;
	bool operator ==(const PlanetKey& other) const;

private:
	int face_;
	int lod_;
	int x_;
	int y_;
};

#endif