/**
 * based on voxel grid polygonization:
 * http://paulbourke.net/geometry/polygonise/ 
 */
#ifndef __VOXELSSPLITTER_H__
#define __VOXELSSPLITTER_H__

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <algorithm>
#include <math.h>

#include <stb/stb_image_resize.h>

#include <memory>

// Local
#include "./../loaders/Loader.h"
#include "./callback.h"
#include "./voxel/VoxelGrid.h"
#include "./Pool.h"
#include "./uvsplit.h"

#include "./SplitBase.h"



class VoxelSplitTask {
  public:
    GroupObject target;

    IdGenerator::ID targetId;
    IdGenerator::ID parentID;
    unsigned int decimationLevel;

    ResultCallback callback;

    int textureLodLevel;
};

typedef PoolFnTemplate<std::shared_ptr<VoxelSplitTask>, GridRef> VoxelPoolFn;



struct GridSettings {
  float isoLevel;
  glm::ivec3 gridResolution;
};


class VoxelsSplitter : public SplitBase<VoxelPoolFn> {
  protected:
    IdGenerator IDGen;
  public:
    VoxelsSplitter();

    // ResultCallback onSave;
    // SplitPool pool;

    unsigned int polygonsLimit = 2048;
    GridSettings gridSettings;

    bool split(GroupObject target, IdGenerator::ID parentId, unsigned int decimationLevel, bool divideVertical);
    bool split(GroupObject target);

    bool processLod(std::shared_ptr<VoxelSplitTask> task, GridRef grid);

    GroupObject decimate(GroupObject target, GridRef grid);
    GroupObject halfMesh(GroupObject target, bool divideVertical);
    

    static const std::string Type;
    static std::shared_ptr<SplitInterface> create();
};

#endif // __VOXELSSPLITTER_H__