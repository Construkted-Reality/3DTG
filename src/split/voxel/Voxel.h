#ifndef __VOXEL_H__
#define __VOXEL_H__

#include <memory>
#include <vector>

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "./structs.h"

class Voxel {
  public:
    std::vector<VoxelFacePtr> faces;

    std::vector<VoxelFaceTriangle> resultTriangles;

    glm::vec3 voxelVertices[8];

    glm::ivec3 position;
    glm::vec3 units;
    float geometricError = 0.0f;

    glm::vec3 averageNormal;

    Voxel(glm::ivec3 position, glm::vec3 units, glm::vec3 offset);

    glm::vec2 getClosestUV(glm::vec3 p);

    bool has(VoxelFacePtr &face);
    bool intersects(glm::vec3 from, glm::vec3 to);
    void computeError();

    virtual ~Voxel();
};

typedef std::shared_ptr<Voxel> VoxelPtr;

#endif // __VOXEL_H__