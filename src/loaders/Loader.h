#ifndef __LOADER_H__
#define __LOADER_H__

#include <vector>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <algorithm>
#include <memory>
#include <cstdlib>
#include <cmath>

#include <glm/glm.hpp>
#include <glm/gtx/closest_point.hpp>
#include <stb/stb_image.h>


#include "./../utils.h"

#define M_PI 3.14159265358979323846  /* pi */


class Mesh;
class Group;
class Vertex;
class Triangle;
class Material;

typedef std::shared_ptr<Vertex> VertexPtr;
typedef std::shared_ptr<Triangle> TrianglePtr;
typedef std::shared_ptr<Mesh> MeshObject;
typedef std::shared_ptr<Group> GroupObject;
typedef std::shared_ptr<Material> MaterialObject;
typedef std::function<void (MeshObject)> TraverseMeshCallback;
typedef std::function<void (GroupObject)> TraverseGroupCallback;



struct Vec3Result {
  glm::vec3 data;
  glm::vec2 uv;
  bool hasData = true;
};

namespace math {
  float triangleIntersection(glm::vec3 origin, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
  Vec3Result clothestTrianglePointOld(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);
  Vec3Result clothestTrianglePoint(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c);

  float deltaX(glm::vec3 &a, glm::vec3 &b, float x);
  float deltaY(glm::vec3 &a, glm::vec3 &b, float y);
  float deltaZ(glm::vec3 &a, glm::vec3 &b, float z);

  glm::vec3 lerp(glm::vec3 &a, glm::vec3 &b, float dt);

  glm::vec3 lerpToX(glm::vec3 &a, glm::vec3 &b, float x);
  glm::vec3 lerpToY(glm::vec3 &a, glm::vec3 &b, float y);
  glm::vec3 lerpToZ(glm::vec3 &a, glm::vec3 &b, float z);
};

struct Face {
  unsigned int positionIndices[3]{0, 0, 0};
  unsigned int normalIndices[3]{0, 0, 0};
  unsigned int uvIndices[3]{0, 0, 0};
};


class Vertex {
  public:
    Vertex();
    Vertex(glm::vec3 vector);

    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;

    std::vector<TrianglePtr> faces;
    std::vector<VertexPtr> neighbors;

    float collapseCost = 0.0f;
    VertexPtr collapseNeighbor = NULL;

    float geometricError = 0.0f;

    float minCost = 0.0f;
    float totalCost = 0.0f;
    unsigned int costCount = 0;

    unsigned int id = 0;
    unsigned int positionId = 0;

    void addUniqueNeighbor(VertexPtr vertex);
    void removeIfNonNeighbor(VertexPtr vertex);
    bool hasNeighbor(VertexPtr vertex);
    void removeTriangle(TrianglePtr triangle);

    // bool operator < (const Vertex& str) const
    // {
    //   return (this->collapseCost < str.collapseCost);
    // }
};

class Triangle : public std::enable_shared_from_this<Triangle> {
  public:
    Triangle(VertexPtr v1, VertexPtr v2, VertexPtr v3, Face f);
    void init();

    unsigned int id = 0;

    VertexPtr v1;
    VertexPtr v2;
    VertexPtr v3;

    std::vector<glm::vec2> faceVertexUvs;

    Face face;
    glm::vec3 normal;

    void computeNormal();
    bool hasVertex(VertexPtr vertex);
    void replaceVertex(VertexPtr oldVertex, VertexPtr newVertex);
};



struct BBoxf {
  glm::vec3 min, max;
  void extend(glm::vec3 position);
  void extend(float x, float y, float z);
  void extend(BBoxf box);
  glm::vec3 size();
  bool intersect(glm::vec3 point);
  bool intersect(glm::vec2 point);
  bool intersect(BBoxf box);
  bool intersectTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3);
  BBoxf clone();
  void translate(float x, float y, float z);
  void fromPoint(float x, float y, float z);
  glm::vec3 getCenter();
  glm::vec3 getSize();
  void setCenter(glm::vec3 point);
  void setSize(glm::vec3 size);
};

struct Image {
  int width;
  int height;
  int channels;
  unsigned char *data = NULL;

  void free();
};

class Material {
  public:
    std::string name = "";
    std::string baseName = "";

    std::string diffuseMap = "";
    Image diffuseMapImage;
    std::map<int, Image> mipMaps;

    glm::vec3 color;

    MaterialObject clone(bool deep);
};

typedef std::map<std::string, MaterialObject> MaterialMap;

class Group {
  public:
    std::vector<GroupObject> children;
    std::vector<MeshObject> meshes;
    std::string name = "";

    BBoxf boundingBox;
    BBoxf uvBox;

    float geometricError = 0.0f;

    void traverse(TraverseMeshCallback fn);
    void traverseGroup(TraverseGroupCallback fn);
    void computeBoundingBox();
    void computeUVBox();
    void computeGeometricError();

    void free(bool deep = true);

    GroupObject clone();
};

class Mesh : public Group {
  public:
    MaterialObject material;
    std::vector<glm::vec3> position;
    std::vector<glm::vec3> normal;
    std::vector<glm::vec2> uv;

    std::vector<Face> faces;

    bool hasNormals = false;
    bool hasUVs = false;

    float geometricError = 0.0f;

    void finish();

    void remesh(std::vector<glm::vec3> &position, std::vector<glm::vec3> &normal, std::vector<glm::vec2> &uv);
    void triangulate();
    void free(bool deep = true);
    void computeBoundingBox();
    void computeUVBox();

    void pushTriangle(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 n, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3);
    void pushQuad(glm::vec3 a, glm::vec3 b, glm::vec3 c, glm::vec3 d, glm::vec3 n1, glm::vec3 n2, glm::vec2 t1, glm::vec2 t2, glm::vec2 t3, glm::vec2 t4);

    MeshObject clone();
    Mesh();
};

class Loader {
  public:
    GroupObject object = GroupObject(new Group());
    Loader();

    void free();

    virtual void parse(const char* path) {};
    virtual ~Loader() {};
};

#endif