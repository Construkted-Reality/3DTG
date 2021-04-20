#ifndef __LOADER_H__
#define __LOADER_H__

#include <vector>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <memory>
#include <cstdlib>

#include "./../utils.h"


class Mesh;
class Group;
typedef std::shared_ptr<Mesh> MeshObject;
typedef std::shared_ptr<Group> GroupObject;
typedef std::function<void (MeshObject)> TraverseMeshCallback;
typedef std::function<void (GroupObject)> TraverseGroupCallback;

struct Vector3f {
  float x = 0.0f, y = 0.0f, z = 0.0f;
};

struct Vector3ui {
  unsigned int x = 0, y = 0, z = 0;
};

struct Vector2f {
  float x = 0.0f, y = 0.0f;
};

struct Face {
  unsigned int positionIndices[3]{0, 0, 0};
  unsigned int normalIndices[3]{0, 0, 0};
  unsigned int uvIndices[3]{0, 0, 0};
};

struct BBoxf {
  Vector3f min, max;
  void extend(Vector3f position);
  void extend(BBoxf box);
  Vector3f size();
  bool intersect(Vector3f point);
  bool intersect(BBoxf box);
  void translate(float x, float y, float z);
};

struct Material {
  std::string name = "";

  std::string diffuseMap = "";
  Vector3f color;
};

typedef std::map<std::string, Material> MaterialMap;

class Group {
  public:
    std::vector<GroupObject> children;
    std::vector<MeshObject> meshes;
    std::string name = "";

    BBoxf boundingBox;

    void traverse(TraverseMeshCallback fn);
    void traverseGroup(TraverseGroupCallback fn);
    void computeBoundingBox();
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
};

class Loader {
  public:
    GroupObject object = GroupObject(new Group());
    Loader();

    virtual void parse(const char* path) {};
    virtual ~Loader() {};
};

#endif