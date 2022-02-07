#include "./uvsplit.h"


GroupObject utils::graphics::splitUV(GroupObject &baseObject, int level) {
  baseObject->computeUVBox();

  // Split meshes by material name
  std::map<std::string, GroupObject> meshMaterialMap;
  //std::map<std::string, Material> materialMap;

  bool lastGroup = false;
  GroupObject currentGroup;

  bool ix, iy, iz;

  // std::cout << "UV split has been started" << std::endl;

  baseObject->traverse([&](MeshObject mesh){
    // std::cout << "Mesh material name: " << mesh->material.name << std::endl;
    if (mesh->material->name != "") {
      GroupObject group;

      // Create group if doesn't exist
      if (meshMaterialMap.find(mesh->material->name) == meshMaterialMap.end()) {
        group = GroupObject(new Group());
        group->meshes.push_back(MeshObject(new Mesh()));

        // std::cout << "Cloning a material" << std::endl;
        group->meshes[0]->material = mesh->material;//->clone(true);
        //group->meshes[0]->material.diffuseMapImage = mesh->material.diffuseMapImage;

        group->boundingBox.min = glm::vec3(0.0f, 0.0f, 0.0f);
        group->boundingBox.max = glm::vec3(1.0f, 1.0f, 0.0f);

        meshMaterialMap[mesh->material->name] = group;
        //materialMap[mesh->material.name] = mesh->material;

        // Init full BVH tree with 3 inherite subtrees
        // std::cout << "BVH generation has been started" << std::endl;
        createBVH( group, 0, 8);//(int) (8 - std::floor(level / 4))
        // std::cout << "BVH generation has been finished" << std::endl;
      } else {
        group = meshMaterialMap[mesh->material->name];
      }

      group->meshes[0]->position.insert(
        std::end(group->meshes[0]->position),
        std::begin(mesh->position),
        std::end(mesh->position)
      );

      group->meshes[0]->normal.insert(
        std::end(group->meshes[0]->normal),
        std::begin(mesh->normal),
        std::end(mesh->normal)
      );

      group->meshes[0]->uv.insert(
        std::end(group->meshes[0]->uv),
        std::begin(mesh->uv),
        std::end(mesh->uv)
      );

      for (Face &face : mesh->faces) // access by reference to avoid copying
      {
        lastGroup = false;
        currentGroup = group;

        // Get latest BVH subgroup from top to down
        while (!lastGroup) {
          if (currentGroup->children.size() == 0) {
            lastGroup = true;

            currentGroup->meshes[0]->faces.push_back(face);
          } else {
            ix = currentGroup->children[0]->boundingBox.intersect(mesh->uv[face.uvIndices[0]]);
            iy = currentGroup->children[0]->boundingBox.intersect(mesh->uv[face.uvIndices[1]]);
            iz = currentGroup->children[0]->boundingBox.intersect(mesh->uv[face.uvIndices[2]]);

            if (ix || iy || iz) {
              currentGroup = currentGroup->children[0];
            } else {
              currentGroup = currentGroup->children[1];
            }
          }
        }
      }
    }
  });

  // std::cout << "Mesh split by material has been finished" << std::endl;

  GroupObject resultGroup = GroupObject(new Group());
  resultGroup->name = baseObject->name;

  // std::cout << "Texture split has been staretd" << std::endl;
  // std::cout << "Splitting" << std::endl;

  int meshIndex = 0;
  for (std::map<std::string, GroupObject>::iterator it = meshMaterialMap.begin(); it != meshMaterialMap.end(); ++it) {
    it->second->traverse([&](MeshObject mesh){
      // std::cout << "Current mesh material name: " << mesh->material.name << std::endl;
      // std::cout << "Mesh \"" << mesh->name << "\" has: " << mesh->faces.size() << " faces" << std::endl;

      // std::cout << "Mesh info assign" << std::endl;
      // std::cout << "Group data: " << it->second->meshes.size() << " meshes" << std::endl;
      // std::cout << it->second->meshes[0]->position.size()  << "v, " << it->second->meshes[0]->normal.size()  << "n, " << it->second->meshes[0]->uv.size()  << "t" << std::endl;

      mesh->hasNormals = it->second->meshes[0]->normal.size() > 0;
      mesh->hasUVs = it->second->meshes[0]->uv.size() > 0;
      mesh->remesh(it->second->meshes[0]->position, it->second->meshes[0]->normal, it->second->meshes[0]->uv);

      // std::cout << "Material info assign" << std::endl;
      // std::cout << "Defining a material name" << std::endl;
      mesh->material = it->second->meshes[0]->material;//->clone(false);
      std::string nextName = mesh->material->name + std::to_string(meshIndex);
      // mesh->material->name += std::to_string(meshIndex);
      // mesh->material->diffuseMap = mesh->material->name + ".jpg";
      //mesh->material.diffuseMapImage = it->second->meshes[0]->material.diffuseMapImage;

      // std::cout << "Info assigning has been finished" << std::endl;

      meshIndex++;

      if (mesh->hasUVs) {
        // std::cout << "Can split" << std::endl;
        // std::cout << "Calculating BBOX has been started" << std::endl;
        mesh->computeUVBox();
        BBoxf uvBox = mesh->uvBox;
        // uvBox.fromPoint(mesh->uv[0].x, mesh->uv[0].y, 0.0f);

        // for (long long unsigned int i = 1; i < mesh->uv.size(); i++) {
        //   uvBox.extend(mesh->uv[i].x, mesh->uv[i].y, 0.0f);
        // }

        unsigned int minX, minY, maxX, maxY;

        //Material meshMaterial = materialMap[it->first];

        minX = floor(uvBox.min.x * it->second->meshes[0]->material->diffuseMapImage.width);
        minY = floor(uvBox.min.y * it->second->meshes[0]->material->diffuseMapImage.height);
        maxX = ceil(uvBox.max.x * it->second->meshes[0]->material->diffuseMapImage.width);
        maxY = ceil(uvBox.max.y * it->second->meshes[0]->material->diffuseMapImage.height);

        //std::cout << "Calculated Box min x/y: " << minX << "/" << minY << " max x/y: " << maxX << "/" << maxY << std::endl;
        //std::cout << "Real Box min x/y: " << uvBox.min.x << "/" << uvBox.min.y << " max x/y: " << uvBox.max.x << "/" << uvBox.max.y << std::endl;

        float offsetX1 = (float) minX / (float) it->second->meshes[0]->material->diffuseMapImage.width;
        float offsetX2 = (float) maxX / (float) it->second->meshes[0]->material->diffuseMapImage.width;

        float offsetY1 = (float) minY / (float) it->second->meshes[0]->material->diffuseMapImage.height;
        float offsetY2 = (float) maxY / (float) it->second->meshes[0]->material->diffuseMapImage.height;

        float UVWidth = offsetX2 - offsetX1;
        float UVHeight = offsetY2 - offsetY1;

        UVWidth = std::max(UVWidth, 0.00001f);
        UVHeight = std::max(UVHeight, 0.00001f);

        for (long long unsigned int i = 0; i < mesh->uv.size(); i++) {
          // uvBox.extend(mesh->uv[i].x, mesh->uv[i].y, 0.0f);
          mesh->uv[i].x = (mesh->uv[i].x - offsetX1) / UVWidth;
          mesh->uv[i].y = (mesh->uv[i].y - offsetY1) / UVHeight;
        }

        unsigned int maxWidth = it->second->meshes[0]->material->diffuseMapImage.width;
        unsigned int maxHeight = it->second->meshes[0]->material->diffuseMapImage.height;
        
        unsigned int textureWidth = std::min(std::max(maxX - minX, (unsigned int) 1), maxWidth);
        unsigned int textureHeight = std::min(std::max(maxY - minY, (unsigned int) 1), maxHeight);

        // std::cout << "Calculating BBOX has been finished" << std::endl;
        // std::cout << "Width: " << textureWidth << ", height: " << textureHeight << std::endl;
        // std::cout << "Initial width: " << maxWidth << ", height: " << maxHeight << std::endl;
        // std::cout << "Min X: " << minX << ", Y: " << minY << std::endl;
        // std::cout << "Lod level: " << level << std::endl;


        // std::cout << "Texture copying has been started" << std::endl;
        // Image &diffuse = mesh->material.diffuseMapImage;
        Image diffuse;

        diffuse.channels = 3;
        diffuse.width = textureWidth;
        diffuse.height = textureHeight;
        // diffuse.data = new unsigned char[textureWidth * textureHeight * 3];
        unsigned char* data = new unsigned char[textureWidth * textureHeight * 3];

        // std::cout << "Texture copying has been finished" << std::endl;
        
        // std::cout << "Texture clipping has been started" << std::endl;
        size_t originLimit = it->second->meshes[0]->material->diffuseMapImage.width * it->second->meshes[0]->material->diffuseMapImage.height * 3;
        size_t targetLimit = textureWidth * textureHeight * 3;


        unsigned int o_X, o_Y, t_X, t_Y;
        unsigned int originPointer, targetPointer;
        for (int i = 0; i < textureHeight; i++)
        {
          for (int j = 0; j < textureWidth; j++)
            {

            // unsigned int originPointer = ((it->second->meshes[0]->material->diffuseMapImage.height - 1 - i - minY) * it->second->meshes[0]->material->diffuseMapImage.width * diffuse.channels) + ((j + minX) * diffuse.channels);
            // unsigned int targetPointer = ((textureHeight - 1 - i) * diffuse.width * diffuse.channels) + (j * diffuse.channels);

            o_X = (minX + j);// Origin X
            o_Y = maxHeight - 1 - (minY + i);// Origin Y (inverted as stb loads images with inverted Y)

            t_X = j;// Target X
            t_Y = textureHeight - 1 - i;// Target Y (inverted as stb saves images with inverted Y)

            unsigned int originPointer = (o_Y * maxWidth * diffuse.channels) + (o_X * diffuse.channels);
            unsigned int targetPointer = (t_Y * diffuse.width * diffuse.channels) + (t_X * diffuse.channels);

            // std::cout << "origin: " << originPointer << ", target: " << targetPointer << std::endl;
            // std::cout << "Limit A: " << ((maxWidth * maxHeight * 3) - 1) << ", B: " << ((textureWidth * textureHeight * 3) - 1) <<std::endl;

            // if (originPointer < 0 || originPointer >= originLimit) {
            //   std::cout << "Origin: " << originPointer << std::endl;
            //   std::cout << "Limit: " << ((maxWidth * maxHeight * 3) - 1) << std::endl;
            // }

            // if (targetPointer < 0 || targetPointer >= targetLimit) {
            //   std::cout << "Target: " << targetPointer << std::endl;
            //   std::cout << "Limit: " << ((textureWidth * textureHeight * 3) - 1) <<std::endl;
            // }

            data[targetPointer] = it->second->meshes[0]->material->diffuseMapImage.data[originPointer];
            data[targetPointer + 1] = it->second->meshes[0]->material->diffuseMapImage.data[originPointer + 1];
            data[targetPointer + 2] = it->second->meshes[0]->material->diffuseMapImage.data[originPointer + 2];
          }
        }

        // std::cout << "Texture copying has been finished" << std::endl;
        // std::cout << "Asigning copyed texture" << std::endl;

        if (level > 0) {
          // std::cout << "Texture lod generation has been started" << std::endl;
          unsigned int simplifiedTextureWidth = std::max(textureWidth / (level * 2), (unsigned int) 1);
          unsigned int simplifiedTextureHeight = std::max(textureHeight / (level * 2), (unsigned int) 1);

          diffuse.data = new unsigned char[simplifiedTextureWidth * simplifiedTextureHeight * 3];
          diffuse.width = simplifiedTextureWidth;
          diffuse.height = simplifiedTextureHeight;

          stbir_resize_uint8( data, textureWidth, textureHeight, 0, diffuse.data, simplifiedTextureWidth, simplifiedTextureHeight, 0, 3);
          delete [] data;

          // std::cout << "Texture lod generation has been finished" << std::endl;
        } else {
          // std::cout << "No texture lods" << std::endl;
          diffuse.data = data;
        }

        // std::cout << "Texture clipping has been finished" << std::endl;

        mesh->material = mesh->material->clone(true);
        mesh->material->name = nextName;
        mesh->material->diffuseMap = mesh->material->name + ".jpg";
        mesh->material->diffuseMapImage = diffuse;

        // std::cout << "Asigning copyed texture finished" << std::endl;
      } else {
        // std::cout << "Can not be split" << std::endl;
        mesh->material = it->second->meshes[0]->material;
      }

      // std::cout << "Mesh \"" << mesh->name << "\" has: " << mesh->faces.size() << " faces, " << mesh->position.size() << " vertices" << std::endl;

      if (mesh->faces.size() > 0) {
        // std::cout << "Mesh is ready for saving: " << mesh->name << std::endl;
        resultGroup->meshes.push_back(mesh);
        resultGroup->computeUVBox();
        resultGroup->computeBoundingBox();
      }
    });

    // std::cout << "Group finished: " << it->first << std::endl;
    //it->second->meshes[0]->material = NULL;
  }

  // std::cout << "All groups are finished" << std::endl;
  // std::cout << "Cleaning the cached data" << std::endl;
  for (std::map<std::string, GroupObject>::iterator it = meshMaterialMap.begin(); it != meshMaterialMap.end(); ++it) {
    it->second->meshes[0]->free();
    it->second->meshes.clear();
  }

  meshMaterialMap.clear();

  // std::cout << "UV split has been finished" << std::endl;

  return resultGroup;
};

void utils::graphics::textureLOD(GroupObject &baseObject, int level) {
  baseObject->traverse([&](MeshObject mesh){
    if (mesh->material->name != "") {
      if (level > 0) {
        MaterialObject nextMaterial = mesh->material->clone(true);
        if (mesh->material->mipMaps.count(level) > 0) {
          nextMaterial->diffuseMapImage = mesh->material->mipMaps[level];
          mesh->material = nextMaterial;
        } else {
          Image diffuse;

          diffuse.channels = mesh->material->diffuseMapImage.channels;

          int textureWidth = mesh->material->diffuseMapImage.width;
          int textureHeight = mesh->material->diffuseMapImage.height;
          
          int simplifiedTextureWidth = std::max(textureWidth / (level * 2), 1);
          int simplifiedTextureHeight = std::max(textureHeight / (level * 2), 1);

          diffuse.data = new unsigned char[simplifiedTextureWidth * simplifiedTextureHeight * 3];
          diffuse.width = simplifiedTextureWidth;
          diffuse.height = simplifiedTextureHeight;

          stbir_resize_uint8( mesh->material->diffuseMapImage.data, textureWidth, textureHeight, 0, diffuse.data, simplifiedTextureWidth, simplifiedTextureHeight, 0, 3);
          // delete [] data;

          mesh->material->mipMaps[level] = diffuse;// Save to the old ref

          nextMaterial->diffuseMapImage = diffuse;
          mesh->material = nextMaterial;
        }
      }
    }
  });
};

void utils::graphics::createBVH(GroupObject &group, int level, int maxLevel, bool shouldDivideVertical) {
  if (level < maxLevel) {
    glm::vec3 size = group->boundingBox.size();

    if (shouldDivideVertical) {
      GroupObject groupTop = GroupObject(new Group());
      GroupObject groupBottom = GroupObject(new Group());

      groupTop->boundingBox.min = group->boundingBox.min;
      groupTop->boundingBox.max = group->boundingBox.max;
      groupTop->boundingBox.max.y -= size.y * 0.5f;

      groupBottom->boundingBox.min = group->boundingBox.min;
      groupBottom->boundingBox.max = group->boundingBox.max;
      groupBottom->boundingBox.min.y += size.y * 0.5f;

      group->children.push_back(groupTop);
      group->children.push_back(groupBottom);

      createBVH(groupTop, level + 1, maxLevel, !shouldDivideVertical);
      createBVH(groupBottom, level + 1, maxLevel, !shouldDivideVertical);
    } else {
      GroupObject groupLeft = GroupObject(new Group());
      GroupObject groupRight = GroupObject(new Group());

      groupLeft->boundingBox.min = group->boundingBox.min;
      groupLeft->boundingBox.max = group->boundingBox.max;
      groupLeft->boundingBox.max.x -= size.x * 0.5f;

      groupRight->boundingBox.min = group->boundingBox.min;
      groupRight->boundingBox.max = group->boundingBox.max;
      groupRight->boundingBox.min.x += size.x * 0.5f;

      group->children.push_back(groupLeft);
      group->children.push_back(groupRight);

      createBVH(groupLeft, level + 1, maxLevel, !shouldDivideVertical);
      createBVH(groupRight, level + 1, maxLevel, !shouldDivideVertical);
    }
  } else if (level > 0) {
    MeshObject mesh = MeshObject(new Mesh());
    mesh->name = 
      std::string("uv_") +
      std::to_string(group->boundingBox.min.x) + std::string(",") + std::to_string(group->boundingBox.min.y) +
      std::string("_") +
      std::to_string(group->boundingBox.max.x) + std::string(",") + std::to_string(group->boundingBox.max.y);
    mesh->material->name = mesh->name;
    group->meshes.push_back(mesh);
  } else {// No BVH
    GroupObject selfGroup = GroupObject(new Group());

    selfGroup->boundingBox.min = group->boundingBox.min;
    selfGroup->boundingBox.max = group->boundingBox.max;

    MeshObject mesh = MeshObject(new Mesh());
    mesh->name = 
      std::string("uv_") +
      std::to_string(group->boundingBox.min.x) + std::string(",") + std::to_string(group->boundingBox.min.y) +
      std::string("_") +
      std::to_string(group->boundingBox.max.x) + std::string(",") + std::to_string(group->boundingBox.max.y);
    mesh->material->name = mesh->name;


    selfGroup->meshes.push_back(mesh);
    group->children.push_back(selfGroup);
  }
};

