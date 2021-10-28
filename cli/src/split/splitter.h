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

typedef std::function<void (GroupObject object, IdGenerator::ID targetId, IdGenerator::ID parentId, unsigned int level)> GroupCallback;

namespace splitter {
  static IdGenerator IDGen;
  void splitObject(GroupObject baseObject, unsigned int polygonLimit, GroupCallback fn, GroupCallback lodFn);
  // bool splitObjectOld(GroupObject baseObject, unsigned int polygonLimit, GroupCallback fn, GroupCallback lodFn, IdGenerator::ID parent, bool isVertical);
  bool splitObject(GroupObject baseObject, unsigned int polygonLimit, GroupCallback fn, GroupCallback lodFn, unsigned int splitLevel, IdGenerator::ID parent, bool isVertical);
  void straightLine(GroupObject &baseObject, bool isVertical, bool isLeft, float xValue, float zValue);
  void straightLineX(MeshObject &mesh, Face &face, bool isLeft, float xValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces);
  void straightLineZ(MeshObject &mesh, Face &face, bool isLeft, float zValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces);
  GroupObject splitUV(GroupObject baseObject, int level);
  void initBVH(GroupObject &group, int level, int maxLevel, bool shouldDivideVertical);
};

#endif