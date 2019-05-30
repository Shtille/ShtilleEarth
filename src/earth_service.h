#pragma once
#ifndef __EARTH_SERVICE_H__
#define __EARTH_SERVICE_H__

#include "../../sht/geo/include/planet_service.h"

class EarthService final : public sht::geo::PlanetService {
public:
	EarthService();
	~EarthService();

private:
	bool Initialize() final;
	void Deinitialize() final;
	bool Execute() final;
};

#endif