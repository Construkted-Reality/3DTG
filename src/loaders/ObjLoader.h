#ifndef __OBJLOADER_H__
#define __OBJLOADER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>

#include <string>
#include <unordered_map>
#include <memory>

#include "Loader.h"
#include "./../split/pool.h"

class TextureLoadTask {
  public:
    std::string texturePath;
    Image* image;
};

// typedef PoolFnTemplate<std::shared_ptr<VoxelSplitTask>, GridRef> VoxelPoolFn;

typedef PoolFnTemplate<std::shared_ptr<TextureLoadTask>> TextureLoadPoolFn;


class ObjLoader : public Loader {
  public:
    SplitPool<TextureLoadPoolFn> pool;
    // std::unordered_map<std::string, bool> processedImages;

    void parse(const char* path);
    void finishMesh(GroupObject &group, MeshObject &mesh, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv);
    MaterialMap loadMaterials(const char* path);

    bool loadTexture(std::shared_ptr<TextureLoadTask> task);
};

#endif 