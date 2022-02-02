#ifndef __VOXELGRID_H__
#define __VOXELGRID_H__

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>

#include "voxel.h"
#include "./../../helpers/triangleBox.h"

class VoxelGrid {
  public:
    VoxelPtr*** data;

    glm::ivec3 gridResolution = glm::ivec3(64, 64, 64);
    glm::vec3 gridOffset;
    glm::vec3 units;

    float isoLevel = 1.0f;
    float isoDelta = 0.00001;

    glm::vec2 facesBox = glm::vec2(0.0f, 0.0f);

    static const unsigned int edgeTable[256];
	  static const int triTable[256][16];

    void init();
    void clear();
    void free();

    VoxelPtr get(unsigned int x, unsigned int y, unsigned int z);
    bool has(unsigned int x, unsigned int y, unsigned int z);
    bool hasTriangles(unsigned int x, unsigned int y, unsigned int z);
    bool triangleIntersectsCell(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::ivec3 cell);
    bool pointInCell(glm::vec3 p, glm::ivec3 cell);
    void set(VoxelPtr &ptr, unsigned int x, unsigned int y, unsigned int z);
    void rasterize(GroupObject &src, GroupObject &dest);
    void voxelize(MeshObject &mesh);
    void build(MeshObject &mesh, std::map<std::string, MeshObject> &materialMeshMap);
    void build(VoxelFaceTriangle &triangle, VoxelFaceVertex &vertex, std::vector<LinkedPosition> &list, unsigned int x, unsigned int y, unsigned int z);

    bool firstOfX(int cellX, int cellY, int cellZ);
    bool firstOfZ(int cellX, int cellY, int cellZ);

    bool lastOfX(int cellX, int cellY, int cellZ);
    bool lastOfZ(int cellX, int cellY, int cellZ);

    glm::ivec3 vecToGrid(float x, float y, float z);
    glm::vec3 gridToVec(unsigned int x, unsigned int y, unsigned int z);

    float getIntValue(unsigned int x, unsigned int y, unsigned int z);
    float getIntValue(unsigned int x, unsigned int y, unsigned int z, unsigned int index);

    bool isFirst(unsigned int x, unsigned int y, unsigned int z, glm::ivec3 n);

    bool isOutOfGrid(unsigned int x, unsigned int y, unsigned int z);
    bool hasNeighbor(unsigned int x, unsigned int y, unsigned int z, unsigned int index);
    VoxelPtr getNeighbor(unsigned int x, unsigned int y, unsigned int z, unsigned int index);

    glm::vec3 getVoxelVertex(unsigned int x, unsigned int y, unsigned int z, unsigned int index);
    glm::vec2 getClosestUV(glm::ivec3 voxelPos, glm::vec3 pos);

    void getClosestUV(VoxelFaceTriangle &triangle, glm::ivec3 voxelPos);

    glm::vec3 intLinear(glm::vec3 p1, glm::vec3 p2, float valp1, float valp2);

    std::vector<VoxelFaceTriangle> getVertices(unsigned int x, unsigned int y, unsigned int z);

    VoxelFaceQuad getQuad(
      Vector3f a, Vector3f b, Vector3f c, Vector3f d,
      Vector3f n1, Vector3f n2,
      Vector2f t1, Vector2f t2, Vector2f t3, Vector2f t4
    );
};

typedef std::shared_ptr<VoxelGrid> GridRef;

#endif // __VOXELGRID_H__