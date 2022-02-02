#ifndef __SPLITBASE_H__
#define __SPLITBASE_H__


#include <iostream>

#include "./../loaders/Loader.h"
#include "./callback.h"
#include "./voxel/voxelgrid.h"
#include "./pool.h"

class SplitInterface {
  public: 
    ResultCallback onSave;

    virtual ~SplitInterface() = default;

    virtual bool split(GroupObject target) = 0;
    virtual void finish() = 0;
};

template <typename PoolItemType>
class SplitBase : public SplitInterface {
  public:    
    SplitPool<PoolItemType> pool;

    void finish() {
      std::cout << "Finishing thread pool" << std::endl;
      this->pool.finish();
    };
};

#endif // __SPLITBASE_H__