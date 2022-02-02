#ifndef __UVSPLIT_H__
#define __UVSPLIT_H__


#include <stb/stb_image_resize.h>

#include "./../loaders/Loader.h"

namespace utils {
  namespace graphics {
    GroupObject splitUV(GroupObject &baseObject, int level = 0);
    void textureLOD(GroupObject &baseObject, int level = 0);
    void createBVH(GroupObject &group, int level = 0, int maxLevel = 8, bool shouldDivideVertical = false);
  }
}

#endif // __UVSPLIT_H__