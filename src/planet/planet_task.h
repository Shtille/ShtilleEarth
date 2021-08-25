#ifndef __PLANET_TASK_H__
#define __PLANET_TASK_H__

#include "planet_task_type.h"
#include "planet_key.h"

#include "common/non_copyable.h"

/*
 * Defines planet task (interface)
 */
class PlanetTask : public scythe::NonCopyable {
public:
	PlanetTask(const PlanetKey& key, PlanetTaskType type);
	virtual ~PlanetTask();

	const PlanetKey& key() const;
	const PlanetTaskType type() const;
	const bool completed() const;

	virtual void Execute() = 0;

protected:
	PlanetKey key_;
	PlanetTaskType type_;
	bool completed_;
};

#endif