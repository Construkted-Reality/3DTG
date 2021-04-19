#ifndef __LOADER_H__
#define __LOADER_H__

#include <vector>
#include <string>
#include <variant>
#include <functional>
#include <memory>
#include <cstdlib>



class Mesh;
class Group;
typedef std::shared_ptr<Mesh> MeshObject;
typedef std::shared_ptr<Group> GroupObject;
typedef std::function<void (MeshObject)> TraverseMeshCallback;
typedef std::function<void (GroupObject)> TraverseGroupCallback;

class Material {
  std::string name = "";
};

class Group {
  public:
    std::vector<GroupObject> children;
    std::vector<MeshObject> meshes;
    std::string name = "";

    void traverse(TraverseMeshCallback fn);
    void traverseGroup(TraverseGroupCallback fn);
};

struct Vector3f {
  float x, y, z;
};

struct Vector3ui {
  unsigned int x, y, z;
};

struct Vector2f {
  float x, y;
};

struct Face {
  unsigned int positionIndices[3]{0, 0, 0};
  unsigned int normalIndices[3]{0, 0, 0};
  unsigned int uvIndices[3]{0, 0, 0};
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