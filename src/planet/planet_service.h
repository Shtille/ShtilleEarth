#ifndef __PLANET_SERVICE_H__
#define __PLANET_SERVICE_H__

#include "planet_key.h"
#include "planet_task.h"
#include "planet_task_factory.h"
#include "planet_tile_provider.h"

#include "common/non_copyable.h"
#include "image/image.h"
#include "memory/stl_pool_allocator.h"

#include <list>
#include <mutex>
#include <condition_variable>
#include <thread>

class PlanetService : public scythe::NonCopyable {
public:
	typedef std::list< PlanetTask * > TaskList; /*, scythe::StlPoolAllocator<PlanetTask *>*/

	PlanetService(PlanetTileProvider * tile_provider);
	~PlanetService();

	void RunService();
	void StopService();

	void AddAlbedoTask(const PlanetKey& key);

	/**
	 * Releases memory occupied by task.
	 * Should be called to free the task.
	 * 
	 * @param[in] task 	The task.
	 */
	void ReleaseTask(PlanetTask * task);

	/**
	 * Removes all tasks that match the provided key
	 * 
	 * @param[in] key 	The key.
	 */
	void RemoveTasks(const PlanetKey& key);

	/**
	 * Swaps done tasks list with provided list
	 * 
	 * @param[in] list 	The task list to be filled.
	 * @return 			Returns true if there is any done task and false otherwise.
	 */
	bool GetDoneTasks(TaskList& list);

private:

	void ThreadFunc();

	PlanetTaskFactory task_factory_;

	TaskList tasks_;
	TaskList done_tasks_;

	std::mutex mutex_;
	std::condition_variable condition_variable_;
	std::thread thread_;

	bool finishing_;
};

#endif