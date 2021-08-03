#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <math.h>

#include "./../loaders/Loader.h"

typedef std::function<void (GroupObject)> GroupCallback;

namespace splitter {
  void splitObject(GroupObject baseObject, GroupCallback fn);
  bool splitObject(GroupObject baseObject, GroupCallback fn, bool isVertical);
  GroupObject splitUV(GroupObject baseObject);
  void initBVH(GroupObject &group, int level, bool shouldDivideVertical);
};

#endif