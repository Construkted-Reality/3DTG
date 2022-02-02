#ifndef __SPLIT_VOXELS_STRUCTS_H__
#define __SPLIT_VOXELS_STRUCTS_H__

#include <string>
#include <vector>
#include <glm/glm.hpp>

#include "./../../loaders/Loader.h"

struct VoxelFaceVertex {
  Vector3f position;
  Vector3f normal;
  Vector2f uv;
  int index = -1;// Assume that we have a constant grid with less than (MAX_INT / 36) cells count
};

struct VoxelFaceTriangle {
  VoxelFaceVertex a;
  VoxelFaceVertex b;
  VoxelFaceVertex c;
  Vector3f normal;

  VoxelFaceVertex& operator[] (size_t i);
};

inline VoxelFaceVertex& VoxelFaceTriangle::operator[] (size_t i)
{
  switch (i) {
    case 0: return this->a;
    case 1: return this->b;
    case 2: return this->c;
    default: throw "Index out of range";
  }
};

struct VoxelFaceQuad {
  VoxelFaceTriangle t1;
  VoxelFaceTriangle t2;
};

struct LinkedPosition {
  VoxelFaceVertex vertex;
  std::vector<VoxelFaceTriangle> linkedTriangles;
};

struct VoxelFace {
  VoxelFaceVertex vertices[3];
  std::string materialName;

  bool hasNormals = false;
  bool hasUVs = false;
};

typedef std::shared_ptr<VoxelFace> VoxelFacePtr;

struct VoxelBox {
  glm::vec3 min;
  glm::vec3 max;
};

#endif // __SPLIT_VOXELS_STRUCTS_H__