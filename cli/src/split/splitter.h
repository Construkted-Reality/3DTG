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
  std::vector<GroupObject> splitObject(GroupObject baseObject, GroupCallback fn, int x, int y, int z);
  GroupObject splitUV(GroupObject baseObject);
  void initBVH(GroupObject &group, int level, bool shouldDivideVertical);
};

#endif