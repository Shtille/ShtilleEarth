#include "planet_task.h"

PlanetTask::PlanetTask(const PlanetKey& key, PlanetTaskType type)
: key_(key)
, type_(type)
, completed_(false)
{
	
}
PlanetTask::~PlanetTask()
{
	
}
const PlanetKey& PlanetTask::key() const
{
	return key_;
}
const PlanetTaskType PlanetTask::type() const
{
	return type_;
}
const bool PlanetTask::completed() const
{
	return completed_;
}