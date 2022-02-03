#ifndef __SPLITBASE_H__
#define __SPLITBASE_H__


#include <iostream>
#include <memory>
#include <map>
#include <functional>

#include "./../Options.h"
#include "./../loaders/Loader.h"
#include "./callback.h"
#include "./voxel/VoxelGrid.h"
#include "./Pool.h"

class SplitInterface {
  public: 
    ResultCallback onSave;

    typedef std::function<std::shared_ptr<SplitInterface>()> SplitCreator;
    static std::map<std::string, SplitCreator> creatorList;

    static std::shared_ptr<SplitInterface> create(std::string type);
    static void addCreator(const std::string type, SplitCreator creator);

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