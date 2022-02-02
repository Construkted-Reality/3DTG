#include "voxel.h"

void Voxel::computeError() {
  this->geometricError = 0.0f;
  if (this->faces.size() > 0) {
    for (VoxelFaceTriangle &triangle : this->resultTriangles) {
      float triangleError = 0.0f;
      for(unsigned int i = 0; i < 3; i++) {

        glm::vec3 dist(0.0f);
        for (VoxelFacePtr &face : this->faces) {
          Vec3Result trPoint = math::clothestTrianglePoint(
            triangle[i].position.toGLM(), 
            face->vertices[0].position.toGLM(),
            face->vertices[1].position.toGLM(),
            face->vertices[2].position.toGLM()
          );

          if (trPoint.hasData) {
            dist += trPoint.data - triangle[i].position.toGLM();
            //triangleError += glm::length(trPoint.data - triangle[i].position.toGLM());
          }
        }

        
        triangleError += glm::length(dist) / std::max(1.0f, (float) this->faces.size());
      }

      this->geometricError += triangleError / 3.0f;
    }

    if (this->resultTriangles.size() > 0) {
      this->geometricError /= (float) this->resultTriangles.size();
    }
  }
};

Voxel::Voxel(glm::ivec3 position, glm::vec3 units, glm::vec3 offset) {
  this->position = position;
  this->units = units;

  this->voxelVertices[0] = glm::vec3(this->position.x,     this->position.y,     this->position.z)     * this->units;
  this->voxelVertices[1] = glm::vec3(this->position.x + 1, this->position.y,     this->position.z)     * this->units;
  this->voxelVertices[2] = glm::vec3(this->position.x + 1, this->position.y + 1, this->position.z)     * this->units;
  this->voxelVertices[3] = glm::vec3(this->position.x,     this->position.y + 1, this->position.z)     * this->units;
  this->voxelVertices[4] = glm::vec3(this->position.x,     this->position.y,     this->position.z + 1) * this->units;
  this->voxelVertices[5] = glm::vec3(this->position.x + 1, this->position.y,     this->position.z + 1) * this->units;
  this->voxelVertices[6] = glm::vec3(this->position.x + 1, this->position.y + 1, this->position.z + 1) * this->units;
  this->voxelVertices[7] = glm::vec3(this->position.x,     this->position.y + 1, this->position.z + 1) * this->units;

  for (unsigned int i = 0; i < 8; i++) {
    this->voxelVertices[i] = glm::vec3((float) this->voxelVertices[i].x, (float) this->voxelVertices[i].y, (float) this->voxelVertices[i].z);
    this->voxelVertices[i] += offset;
  }
};

bool Voxel::has(VoxelFacePtr &face) {
  std::vector<VoxelFacePtr>::iterator it = std::find(std::begin(this->faces), std::end(this->faces), face);

  return (it != this->faces.end());
};

Voxel::~Voxel() {
  this->faces.clear();
  this->resultTriangles.clear();
};

glm::vec2 Voxel::getClosestUV(glm::vec3 p) {
  glm::vec2 result;
  float dist = 999999.0f;

  for (VoxelFacePtr &facePtr : this->faces) {
    for (unsigned int i = 0; i < 3; i++) {
      VoxelFaceVertex &voxelFaceVertex = facePtr->vertices[i];

      float dt = voxelFaceVertex.position.distanceTo(Vector3f::fromGLM(p));
      if (dt <= dist) {
        dist = dt;
        result = voxelFaceVertex.uv.toGLM();
      }
    }
  }

  return result;
};

bool Voxel::intersects(glm::vec3 from, glm::vec3 to) {
  return true;
};
