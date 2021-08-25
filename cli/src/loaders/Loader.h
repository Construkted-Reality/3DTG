#ifndef __LOADER_H__
#define __LOADER_H__

#include <vector>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <memory>
#include <cstdlib>
#include <cmath>


#include "./../helpers/stb_image.h"

#include "./../utils.h"


class Mesh;
class Group;
class Vertex;
class Triangle;
typedef std::shared_ptr<Vertex> VertexPtr;
typedef std::shared_ptr<Triangle> TrianglePtr;
typedef std::shared_ptr<Mesh> MeshObject;
typedef std::shared_ptr<Group> GroupObject;
typedef std::function<void (MeshObject)> TraverseMeshCallback;
typedef std::function<void (GroupObject)> TraverseGroupCallback;

struct Vector3f {
  float x = 0.0f, y = 0.0f, z = 0.0f;
  float dot(Vector3f vector);
  float length();
  float distanceTo(Vector3f vector);
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

    float minCost = 0.0f;
    float totalCost = 0.0f;
    unsigned int costCount = 0;

    unsigned int id = 0;
    unsigned int positionId = 0;

    void addUniqueNeighbor(VertexPtr vertex);
    void removeIfNonNeighbor(VertexPtr vertex);
    bool hasNeighbor(VertexPtr vertex);
    void removeTriangle(TrianglePtr triangle);
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

struct BBoxf {
  Vector3f min, max;
  void extend(Vector3f position);
  void extend(float x, float y, float z);
  void extend(BBoxf box);
  Vector3f size();
  bool intersect(Vector3f point);
  bool intersect(Vector2f point);
  bool intersect(BBoxf box);
  void translate(float x, float y, float z);
  void fromPoint(float x, float y, float z);
};

struct Image {
  int width;
  int height;
  int channels;
  unsigned char *data = NULL;

  void free();
};

struct Material {
  std::string name = "";

  std::string diffuseMap = "";
  Image diffuseMapImage;

  Vector3f color;
};

typedef std::map<std::string, Material> MaterialMap;

class Group {
  public:
    std::vector<GroupObject> children;
    std::vector<MeshObject> meshes;
    std::string name = "";

    BBoxf boundingBox;
    BBoxf uvBox;

    void traverse(TraverseMeshCallback fn);
    void traverseGroup(TraverseGroupCallback fn);
    void computeBoundingBox();
    void computeUVBox();

    void free();
};

class Mesh : public Group {
  public:
    Material material;
    std::vector<Vector3f> position;
    std::vector<Vector3f> normal;
    std::vector<Vector2f> uv;

    std::vector<Face> faces;

    bool hasNormals = false;
    bool hasUVs = false;

    void finish();

    void remesh(std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv);
    void free();
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