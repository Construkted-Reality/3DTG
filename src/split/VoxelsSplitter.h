/**
 * based on voxel grid polygonization:
 * http://paulbourke.net/geometry/polygonise/ 
 */
#ifndef __VOXELSSPLITTER_H__
#define __VOXELSSPLITTER_H__

#include <glm/glm.hpp>
#include <glm/gtx/rotate_vector.hpp>
#include <vector>
#include <map>
#include <iostream>
#include <functional>
#include <algorithm>
#include <math.h>

#include <stb/stb_image_resize.h>

#include "./../loaders/Loader.h"
#include "./../simplify/simplifier.h"
#include "./../helpers/IdGenerator.h"
#include "./../helpers/triangleBox.h"

#include "./splitter.h"

typedef std::function<void (GroupObject object, IdGenerator::ID targetId, IdGenerator::ID parentId, unsigned int level)> ResultCallback;

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

class Voxel {
  public:
    std::vector<VoxelFacePtr> faces;

    std::vector<VoxelFaceTriangle> resultTriangles;

    glm::vec3 voxelVertices[8];

    glm::ivec3 position;
    glm::vec3 units;
    float geometricError = 0.0f;

    glm::vec3 averageNormal;// Used to delete wrong triangles
    //void filterTriangles();

    Voxel(glm::ivec3 position, glm::vec3 units);

    glm::vec2 getClosestUV(glm::vec3 p);

    bool has(VoxelFacePtr &face);
    bool intersects(glm::vec3 from, glm::vec3 to);
    void computeError();

    virtual ~Voxel();
};

typedef std::shared_ptr<Voxel> VoxelPtr;

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

    glm::vec3 intLinear(glm::vec3 p1, glm::vec3 p2, float valp1, float valp2);

    std::vector<VoxelFaceTriangle> getVertices(unsigned int x, unsigned int y, unsigned int z);

    VoxelFaceQuad getQuad(
      Vector3f a, Vector3f b, Vector3f c, Vector3f d,
      Vector3f n1, Vector3f n2,
      Vector2f t1, Vector2f t2, Vector2f t3, Vector2f t4
    );
};

class VoxelsSplitter {
  protected:
    IdGenerator IDGen;
  public:
    // ResultCallback onLodSave;
    VoxelsSplitter();

    ResultCallback onSave;

    unsigned int polygonsLimit = 2048;

    VoxelGrid grid;

    bool split(GroupObject target, IdGenerator::ID parentId, unsigned int decimationLevel, bool divideVertical);
    bool split(GroupObject target);

    GroupObject decimate(GroupObject target);
    GroupObject halfMesh(GroupObject target, bool divideVertical);

    // void createVoxel(MeshObject &mesh, glm::ivec3 cell);
    // void createVoxelPlane(MeshObject &mesh, glm::ivec3 cell, glm::vec3 normal);

    // void init();
    // void clear();
    // void free();
};

#endif // __VOXELSSPLITTER_H__