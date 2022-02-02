#ifndef __POOL_H__
#define __POOL_H__


#include <memory>
#include <vector>

// Threads
#include <chrono>
#include <iostream>
#include <future>
#include <thread>

// Regular
#include "callback.h"
#include "./voxel/voxelgrid.h"



template<typename... Args>
using PoolFnTemplate = bool(Args... args);// std::shared_ptr<SplitTask>, 

// typedef PoolFnTemplate<std::shared_ptr<SplitTask>, GridRef> PoolFn;


// typedef std::function<bool (std::shared_ptr<SplitTask> task, GridRef grid)> PackagedSplit;
// typedef std::function<PoolFn> PackagedSplit;


template <typename PoolFunction>
class SplitProcess {
  public:
    typedef std::function<PoolFunction> PackagedSplit;

    bool finished = false;

    std::future<bool> future;
    std::thread thread;

    template<typename... Args>
    SplitProcess(PackagedSplit taskFn, Args... args) {
      std::packaged_task< PoolFunction > task = std::packaged_task< PoolFunction >(taskFn);
      this->future = task.get_future();
      this->thread = std::thread(move(task), args...);
    };

    // SplitProcess(PackagedSplit taskFn, std::shared_ptr<SplitTask> taskObject, GridRef grid) {
    //   std::packaged_task< PoolFunction > task = std::packaged_task< PoolFunction >(taskFn);
    //   this->future = task.get_future();
    //   this->thread = std::thread(move(task), taskObject, grid);
    // };

    // virtual ~SplitProcess() = default;

    bool isReady() {
      return this->future.wait_for(std::chrono::milliseconds(50)) == std::future_status::ready;
    };

    void finish() {
      // std::cout << "Joining the task: " << this->id << std::endl;
      this->thread.join();
    };
};

template <typename PoolFunction>
class SplitPool {
  public:
    typedef std::shared_ptr<SplitProcess<PoolFunction>> ProcessRef;

    std::vector<ProcessRef> splitResult;

    unsigned int threadsUsed = 1;
    unsigned int threadsAvailable = 0;

    unsigned int currentTaskId = 0;

    SplitPool() {
      this->threadsAvailable = std::thread::hardware_concurrency();
    };

    virtual ~SplitPool() {
      for (ProcessRef &task : splitResult) {
        task->finish();
      }

      splitResult.clear();
    };

    bool hasSlot() {
      return this->threadsUsed < this->threadsAvailable;
    };

    template<typename... Args>
    void create(Args... args) {
      std::cout << "Task id:" << (this->currentTaskId++) << std::endl;
      // std::cout << "Push task to the pool" << std::endl;
      ProcessRef result = std::make_shared<SplitProcess<PoolFunction>>(args...);
      this->splitResult.push_back(result);
    };

    // void create(PackagedSplit taskFn, std::shared_ptr<SplitTask> taskObject, GridRef grid) {
    //   std::cout << "Split id: " << (this->currentTaskId++) << std::endl;
    //   // std::cout << "Push task to the pool" << std::endl;
    //   ProcessRef result = std::make_shared<SplitProcess<PoolFunction>>(taskFn, taskObject, grid);
    //   this->splitResult.push_back(result);
    // };

    void finish() {
      std::cout << "Finishing all the threads" << std::endl;

      std::vector<ProcessRef> tasksToFinish;

      while (this->splitResult.size() > 0) {
        for (ProcessRef &task : this->splitResult) {
          if (task->isReady()) {
            tasksToFinish.push_back(task);
          }
        }

        for (ProcessRef &task : tasksToFinish) {
          task->finish();
          this->splitResult.erase(std::remove(this->splitResult.begin(), this->splitResult.end(), task), this->splitResult.end());
        }

        tasksToFinish.clear();

        std::this_thread::sleep_for(std::chrono::milliseconds(300));
      }

      std::cout << "All the threads finished" << std::endl;
    };

    void waitForSlot() {
      if (this->splitResult.size() >= this->threadsAvailable) {
        // std::cout << "Wait untill some task is finished" << std::endl;

        std::vector<ProcessRef> tasksToFinish;

        while (true) {
          for (ProcessRef &task : this->splitResult) {
            if (task->isReady()) {
              tasksToFinish.push_back(task);
            }
          }

          if (tasksToFinish.size() > 0) {
            break;
          }

          std::this_thread::sleep_for(std::chrono::milliseconds(300));
        }

        // std::cout << "Tasks ready to finish: " << tasksToFinish.size() << std::endl;

        for (ProcessRef &task : tasksToFinish) {
          task->finish();
          this->splitResult.erase(std::remove(this->splitResult.begin(), this->splitResult.end(), task), this->splitResult.end());
        }

        tasksToFinish.clear();
      }
    };
};

#endif // __POOL_H__