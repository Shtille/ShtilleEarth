#include "../include/planet_service.h"

#include <functional>

namespace sht {
	namespace geo {

		PlanetService::PlanetService()
			: face_(-1)
			, lod_(-1)
			, x_(-1)
			, y_(-1)
			, finishing_(false)
			, done_(false)
			, has_task_(false)
		{
		}
		PlanetService::~PlanetService()
		{
			volatile bool finishing;
			mutex_.lock();
			finishing = finishing_;
			mutex_.unlock();
			if (!finishing)
				StopService();
		}
		void PlanetService::RunService()
		{
			finishing_ = false;
			thread_ = std::thread(std::bind(&PlanetService::ThreadFunc, this));
		}
		void PlanetService::StopService()
		{
			{//---
				std::unique_lock<std::mutex> guard(mutex_);
				finishing_ = true;
				condition_variable_.notify_one();
			}//---
			thread_.join();
		}
		bool PlanetService::CheckRegionStatus(int face, int lod, int x, int y)
		{
			std::unique_lock<std::mutex> guard(mutex_);
			if (done_)
			{
				// Notify owner that task is done and fall into sleep
				done_ = false;
				return true;
			}
			has_task_ = true;
			face_ = face;
			lod_ = lod;
			x_ = x;
			y_ = y;
			// Wake up our thread
			condition_variable_.notify_one();
			return false;
		}
		const graphics::Image& PlanetService::image()
		{
			// We shouldn't protect image via mutex because this thread will be stopped when image is accessed
			return image_;
		}
		bool PlanetService::Initialize()
		{
			return true;
		}
		void PlanetService::Deinitialize()
		{
		}
		void PlanetService::ObtainTileParameters(int* face, int* lod, int* x, int* y)
		{
			std::lock_guard<std::mutex> guard(mutex_);
			*face = face_;
			*lod = lod_;
			*x = x_;
			*y = y_;
		}
		void PlanetService::ThreadFunc()
		{
			volatile bool finishing = false;
			volatile bool has_task = false;
			volatile bool done = false;
			for (;;)
			{
				{//---
					std::lock_guard<std::mutex> guard(mutex_);
					finishing = finishing_;
					has_task = has_task_;
				}//---

				if (finishing)
					break;

				if (!has_task)
				{
					std::unique_lock<std::mutex> guard(mutex_);
					while (!finishing_ && !has_task_)
						condition_variable_.wait(guard);
					continue;
				}

				done = Execute();

				if (done)
				{
					std::lock_guard<std::mutex> guard(mutex_);
					done_ = done;
					has_task_ = !done;
				}
			}
		}

	} // namespace geo
} // namespace sht