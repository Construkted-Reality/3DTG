#ifndef __TILECONTENT_H__
#define __TILECONTENT_H__

#include <memory>
#include <string>

#include "./TileBoundingVolume.h"

struct TileContent {
  std::shared_ptr<TileBoundingVolume> boundingVolume = NULL;
  std::string uri;// Required
};

#endif // __TILECONTENT_H__