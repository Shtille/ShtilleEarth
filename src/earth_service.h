#ifndef __EARTH_SERVICE_H__
#define __EARTH_SERVICE_H__

#include "planet/planet_service.h"

class EarthService final : public PlanetService {
public:
	EarthService();
	~EarthService();

private:
	bool Initialize() final;
	void Deinitialize() final;
	bool Execute() final;
};

#endif