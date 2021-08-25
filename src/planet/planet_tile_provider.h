#ifndef __PLANET_TILE_PROVIDER_H__
#define __PLANET_TILE_PROVIDER_H__

#include "planet_key.h"

#include "common/non_copyable.h"

namespace scythe {
	class Image;
}

/**
 * Defines planet tile (albedo) provider interface
 */
class PlanetTileProvider : public scythe::NonCopyable {
public:
	PlanetTileProvider() = default;
	virtual ~PlanetTileProvider() = default;

	/*
	 * Sets target image to render into.
	 *
	 * @param{in} image		The image.
	 */
	virtual void SetTarget(scythe::Image * image) = 0;

	/*
	 * Renders one tile to target image buffer.
	 *
	 * @param{in} key	The tile key.
	 *
	 * @return Returns true if rendering was fully completed
	 *         and false otherwise.
	 */
	virtual bool RenderTile(const PlanetKey& key) = 0;
};

#endif