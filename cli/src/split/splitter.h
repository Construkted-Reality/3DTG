#ifndef __SPLITTER_H__
#define __SPLITTER_H__

#include <vector>
#include <iostream>
#include "./../loaders/Loader.h"

namespace splitter {
  std::vector<GroupObject> splitObject(GroupObject baseObject, int x, int y, int z);
};

#endif