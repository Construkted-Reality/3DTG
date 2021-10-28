#ifndef __TILEBOUNDINGVOLUME_H__
#define __TILEBOUNDINGVOLUME_H__

#include <memory>
#include <vector>

#include <glm/vec3.hpp>


struct TileBoundingBox {
  glm::vec3 center;
  glm::vec3 xHalf;
  glm::vec3 yHalf;
  glm::vec3 zHalf;

  std::vector<float> toArray();
};

struct TileBoundingSphere {
  glm::vec3 center;
  float radius;

  std::vector<float> toArray();
};

struct TileBoundingRegion {
  float west, south, east, north, minHeight, maxHeight;

  std::vector<float> toArray();
};

struct TileBoundingVolume {
  std::shared_ptr<TileBoundingBox> box = NULL;
  std::shared_ptr<TileBoundingSphere> sphere = NULL;
  std::shared_ptr<TileBoundingRegion> region = NULL;
};


#endif // __TILEBOUNDINGVOLUME_H__