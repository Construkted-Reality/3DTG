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

struct Vector3f {
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float dot(Vector3f vector);
  float length();
  float lengthSq();
  float angleTo(Vector3f vector);
  float distanceTo(Vector3f vector);
  bool angleLess90(Vector3f vector);
  bool equals(Vector3f vector);
  void normalize();
  void divideScalar(float value);
  void multiplyScalar(float value);
  void add(Vector3f vector);
  void sub(Vector3f vector);
  void sub(Vector3f vectorA, Vector3f vectorB);
  void set(float x, float y, float z);
  void set(Vector3f vector);
  void cross(Vector3f vector);
  void lerp(Vector3f a, Vector3f b, float delta);
  void lerpToX(Vector3f a, Vector3f b, float x);
  void lerpToY(Vector3f a, Vector3f b, float y);
  void lerpToZ(Vector3f a, Vector3f b, float z);
  glm::vec3 toGLM();
  static Vector3f fromGLM(glm::vec3 vec);
  static float deltaX(Vector3f a, Vector3f b, float x);
  static float deltaY(Vector3f a, Vector3f b, float y);
  static float deltaZ(Vector3f a, Vector3f b, float z);
  Vector3f clone();
  Vector3f intersectPlane(Vector3f planePoint, Vector3f planeNormal, Vector3f lineBegin, Vector3f lineDirection);
};

struct Vector3ui {
  unsigned int x = 0, y = 0, z = 0;
};

struct Vector2f {
  float x = 0.0f, y = 0.0f;
  void set(float x, float y);
  void set(Vector2f vector);
  glm::vec2 toGLM();
  static Vector2f fromGLM(glm::vec2 vec);
};

struct Face {
  unsigned int positionIndices[3]{0, 0, 0};
  unsigned int normalIndices[3]{0, 0, 0};
  unsigned int uvIndices[3]{0, 0, 0};
};


class Vertex {
  public:
    Vertex();
    Vertex(Vector3f vector);

    Vector3f position;
    Vector3f normal;
    Vector2f uv;

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

    std::vector<Vector2f> faceVertexUvs;

    Face face;
    Vector3f normal;

    void computeNormal();
    bool hasVertex(VertexPtr vertex);
    void replaceVertex(VertexPtr oldVertex, VertexPtr newVertex);
};

namespace math {
  float triangleIntersection(glm::vec3 origin, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2);
  float triangleIntersection(Vector3f origin, Vector3f dir, Vector3f v0, Vector3f v1, Vector3f v2);
};

struct BBoxf {
  Vector3f min, max;
  void extend(Vector3f position);
  void extend(float x, float y, float z);
  void extend(BBoxf box);
  Vector3f size();
  bool intersect(Vector3f point);
  bool intersect(Vector2f point);
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

    Vector3f color;

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
    std::vector<Vector3f> position;
    std::vector<Vector3f> normal;
    std::vector<Vector2f> uv;

    std::vector<Face> faces;

    bool hasNormals = false;
    bool hasUVs = false;

    float geometricError = 0.0f;

    void finish();

    void remesh(std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv);
    void triangulate();
    void free(bool deep = true);
    void computeBoundingBox();
    void computeUVBox();

    void pushTriangle(Vector3f a, Vector3f b, Vector3f c, Vector3f n, Vector2f t1, Vector2f t2, Vector2f t3);
    void pushQuad(Vector3f a, Vector3f b, Vector3f c, Vector3f d, Vector3f n1, Vector3f n2, Vector2f t1, Vector2f t2, Vector2f t3, Vector2f t4);

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