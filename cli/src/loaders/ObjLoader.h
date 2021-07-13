#ifndef __OBJLOADER_H__
#define __OBJLOADER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>


#include "Loader.h"

class ObjLoader : public Loader {
  public:
    void parse(const char* path);
    void finishMesh(GroupObject &group, MeshObject &mesh, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv);
    MaterialMap loadMaterials(const char* path);
};

#endif 