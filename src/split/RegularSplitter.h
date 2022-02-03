#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <algorithm>
#include <math.h>

#include <stb/stb_image_resize.h>

#include "./../loaders/Loader.h"
#include "./../simplify/simplifier.h"
#include "./../helpers/IdGenerator.h"

#include "./SplitBase.h"
#include "./uvsplit.h"


class RegularSplitTask {
  public:
    GroupObject target;

    IdGenerator::ID targetId;
    IdGenerator::ID parentID;
    unsigned int decimationLevel;

    int uvModifier;

    ResultCallback callback;
};

// typedef PoolFnTemplate<std::shared_ptr<VoxelSplitTask>, GridRef> VoxelPoolFn;

typedef PoolFnTemplate<std::shared_ptr<RegularSplitTask>> RegularPoolFn;

class RegularSplitter : public SplitBase<RegularPoolFn> {
  public:
    IdGenerator IDGen;
    unsigned int polygonLimit = 2048;

    bool split(GroupObject baseObject);
    // bool splitObjectOld(GroupObject baseObject, unsigned int polygonLimit, GroupCallback fn, GroupCallback lodFn, IdGenerator::ID parent, bool isVertical);
    bool splitObject(GroupObject baseObject, unsigned int polygonLimit, unsigned int splitLevel, IdGenerator::ID parent, bool isVertical);
    void straightLine(GroupObject &baseObject, bool isVertical, bool isLeft, float xValue, float zValue);
    void straightLineX(MeshObject &mesh, Face &face, bool isLeft, float xValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces);
    void straightLineZ(MeshObject &mesh, Face &face, bool isLeft, float zValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces);


    bool processLod(std::shared_ptr<RegularSplitTask> task);

    static const std::string Type;
    static std::shared_ptr<SplitInterface> create();
};

#endif