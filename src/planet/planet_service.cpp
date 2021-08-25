#include "planet_service.h"

#include <functional>
#include <new>

/**
 * Predicate for mathing all task with specified key
 */
class KeyMatchPredicate {
public:
	KeyMatchPredicate(const PlanetKey& key)
	: key_(key)
	{

	}

	bool operator()(const PlanetTask * task)
	{
		return task->key() == key_;
	}
private:
	PlanetKey key_;
};

PlanetService::PlanetService(PlanetTileProvider * tile_provider)
: task_factory_(tile_provider)
, finishing_(false)
{
}
PlanetService::~PlanetService()
{
	StopService();
}
void PlanetService::RunService()
{
	finishing_ = false;
	thread_ = std::thread(std::bind(&PlanetService::ThreadFunc, this));
}
void PlanetService::StopService()
{
	if (thread_.joinable())
	{
		{//---
			std::lock_guard<std::mutex> guard(mutex_);
			finishing_ = true;
			condition_variable_.notify_one();
		}//---
		thread_.join();
	}
}
void PlanetService::AddAlbedoTask(const PlanetKey& key)
{
	PlanetTask * task = task_factory_.CreateAlbedoTask(key);
	{//---
		std::lock_guard<std::mutex> guard(mutex_);
		tasks_.push_back(task);
	}//---
	// Wake up our thread if it sleeps
	condition_variable_.notify_one();
}
void PlanetService::ReleaseTask(PlanetTask * task)
{
	task_factory_.DestroyTask(task);
}
void PlanetService::RemoveTasks(const PlanetKey& key)
{
	KeyMatchPredicate predicate(key);
	{//---
		std::lock_guard<std::mutex> guard(mutex_);
		tasks_.remove_if(predicate);
	}//---
}
bool PlanetService::GetDoneTasks(TaskList& list)
{
	{//---
		std::lock_guard<std::mutex> guard(mutex_);
		if (done_tasks_.empty())
			return false;
		else
		{
			done_tasks_.swap(list);
			return true;
		}
	}//---
}
void PlanetService::ThreadFunc()
{
	PlanetTask * task = nullptr;
	bool finishing = false;
	for (;;)
	{
		{//---
			std::lock_guard<std::mutex> guard(mutex_);
			// Store processed task
			if (task != nullptr)
			{
				done_tasks_.push_back(task);
			}
			// Get new task
			if (!tasks_.empty())
			{
				task = tasks_.front();
				tasks_.pop_front();
			}
			else
				task = nullptr;
			// Fetch flags
			finishing = finishing_;
		}//---

		if (finishing)
			break;

		if (task != nullptr)
		{
			task->Execute();
		}
		else
		{
			std::unique_lock<std::mutex> guard(mutex_);
			while (!finishing_ && tasks_.empty())
				condition_variable_.wait(guard);
			continue;
		}
	}
}