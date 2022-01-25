/**
 * ParallelCompute.cpp
 *
 * Compute a function result in parallel.
 *
 * Author: Dennis J. McWherter, Jr.
 */

#include <chrono>
#include <iostream>
#include <future>
#include <thread>

#include <mutex>

#include <vector>
#include <map>
#include <functional>
#include <algorithm>

// Random
#include <cstdlib>
#include <ctime>

using namespace std;

typedef std::function<void (unsigned int index)> ResultCallback;
class Processor {
  public:
    ResultCallback onResult;
    const unsigned int maxIndex = 999999999;

    int currentValue = 0;
    bool isRunning = false;
    // std::recursive_mutex mutex;

    // packaged_task<void()> currentTask;

    Processor() {
      // this->currentTask = packaged_task<void()>(&Processor::process);
    };

    void run() {
      this->isRunning = true;

      // std::thread t1(&Processor::process, this);
      // t1.join();
    };

    unsigned int increment(unsigned int value) {
      return value + 1;
    };

    bool process(unsigned int limit) {
      for (unsigned int i = 0 ; i < limit ; ++i) {
        this->currentValue = this->increment(this->currentValue);
      }
      
      // std::lock_guard<std::recursive_mutex> guard(mutex);
      this->onResult(this->currentValue);

      return true;
    };
};


int myComputation(int x) {
  for (unsigned i = 0 ; i < 999999999 ; ++i) {
    x++;
  }
  return x;
}

int main() {
  Processor processor1;
  Processor processor2;

  processor1.onResult = [](unsigned int index){
    cout << "Current index (1): " << index << endl;
  };
  processor2.onResult = [](unsigned int index){
    cout << "Current index (2): " << index << endl;
  };

  // processor1.run();
  // processor2.run();

  // Create promises
  packaged_task< bool(unsigned int) > task1(bind(&Processor::process, &processor1, placeholders::_1));
  packaged_task< bool(unsigned int) > task2(bind(&Processor::process, &processor2, placeholders::_1));

  // Get futures
  future<bool> val1 = task1.get_future();
  future<bool> val2 = task2.get_future();

  // Schedule promises
  // thread t1(move(task1), 0);
  // thread t2(move(task2), 5);

  thread t1(move(task1), 999999999);
  thread t2(move(task2), 9999999);

  cout << "Threads were started?" << endl;

  // Print status while we wait
  bool s1 = false, s2 = false;
  do {
    s1 = val1.wait_for(chrono::milliseconds(50)) == future_status::ready;
    s2 = val2.wait_for(chrono::milliseconds(50)) == future_status::ready;
    cout<< "Value 1 is " << (s1 ? "" : "not ") << "ready" << endl;
    cout<< "Value 2 is " << (s2 ? "" : "not ") << "ready" << endl;
    this_thread::sleep_for(chrono::milliseconds(300));
  } while (!s1 || !s2);

  cout << "Threads has been finished" << endl;

  // Cleanup threads-- we could obviously block and wait for our threads to finish if we don't want to print status.
  t1.join();
  t2.join();

  

  unsigned int n = std::thread::hardware_concurrency();
  std::cout << n << " concurrent threads are supported.\n";

  return 0;
};

/*
int oldmain() {
  // Create promises
  packaged_task<int(int)> task1(&myComputation);
  packaged_task<int(int)> task2(&myComputation);

  // Get futures
  future<int> val1 = task1.get_future();
  future<int> val2 = task2.get_future();

  // Schedule promises
  thread t1(move(task1), 0);
  thread t2(move(task2), 5);

  // Print status while we wait
  bool s1 = false, s2 = false;
  do {
    s1 = val1.wait_for(chrono::milliseconds(50)) == future_status::ready;
    s2 = val2.wait_for(chrono::milliseconds(50)) == future_status::ready;
    cout<< "Value 1 is " << (s1 ? "" : "not ") << "ready" << endl;
    cout<< "Value 2 is " << (s2 ? "" : "not ") << "ready" << endl;
    this_thread::sleep_for(chrono::milliseconds(300));
  } while (!s1 || !s2);

  // Cleanup threads-- we could obviously block and wait for our threads to finish if we don't want to print status.
  t1.join();
  t2.join();

  // Final result
  cout<< "Value 1: " << val1.get() << endl;
  cout<< "Value 2: " << val2.get() << endl;
  return 0;
}
*/