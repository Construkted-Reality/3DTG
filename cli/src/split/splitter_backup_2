#include "./splitter.h"


void splitter::initBVH(GroupObject &group, int level = 0, bool shouldDivideVertical = false) {
  if (level < 8) {
    Vector3f size = group->boundingBox.size();

    if (shouldDivideVertical) {
      GroupObject groupTop = GroupObject(new Group());
      GroupObject groupBottom = GroupObject(new Group());

      groupTop->boundingBox.min = group->boundingBox.min.clone();
      groupTop->boundingBox.max = group->boundingBox.max.clone();
      groupTop->boundingBox.max.y -= size.y * 0.5f;

      groupBottom->boundingBox.min = group->boundingBox.min.clone();
      groupBottom->boundingBox.max = group->boundingBox.max.clone();
      groupBottom->boundingBox.min.y += size.y * 0.5f;

      group->children.push_back(groupTop);
      group->children.push_back(groupBottom);

      splitter::initBVH(groupTop, level + 1, !shouldDivideVertical);
      splitter::initBVH(groupBottom, level + 1, !shouldDivideVertical);
    } else {
      GroupObject groupLeft = GroupObject(new Group());
      GroupObject groupRight = GroupObject(new Group());

      groupLeft->boundingBox.min = group->boundingBox.min.clone();
      groupLeft->boundingBox.max = group->boundingBox.max.clone();
      groupLeft->boundingBox.max.x -= size.x * 0.5f;

      groupRight->boundingBox.min = group->boundingBox.min.clone();
      groupRight->boundingBox.max = group->boundingBox.max.clone();
      groupRight->boundingBox.min.x += size.x * 0.5f;

      group->children.push_back(groupLeft);
      group->children.push_back(groupRight);

      splitter::initBVH(groupLeft, level + 1, !shouldDivideVertical);
      splitter::initBVH(groupRight, level + 1, !shouldDivideVertical);
    }
  } else if (level == 8) {
    MeshObject mesh = MeshObject(new Mesh());
    mesh->name = 
      std::string("uv_") +
      std::to_string(group->boundingBox.min.x) + std::string(",") + std::to_string(group->boundingBox.min.y) +
      std::string("_") +
      std::to_string(group->boundingBox.max.x) + std::string(",") + std::to_string(group->boundingBox.max.y);
    mesh->material.name = mesh->name;
    group->meshes.push_back(mesh);
  }
};

GroupObject splitter::splitUV(GroupObject baseObject) {
  baseObject->computeUVBox();

  // Split meshes by material name
  std::map<std::string, GroupObject> meshMaterialMap;
  //std::map<std::string, Material> materialMap;

  bool lastGroup = false;
  GroupObject currentGroup;

  bool ix, iy, iz;

  baseObject->traverse([&](MeshObject mesh){
    if (mesh->material.name != "") {
      GroupObject group;

      // Create group if doesn't exist
      if (meshMaterialMap.find(mesh->material.name) == meshMaterialMap.end()) {
        group = GroupObject(new Group());
        group->meshes.push_back(MeshObject(new Mesh()));

        group->meshes[0]->material = mesh->material;
        group->meshes[0]->material.diffuseMapImage = mesh->material.diffuseMapImage;

        group->boundingBox.min.set(0.0f, 0.0f, 0.0f);
        group->boundingBox.max.set(1.0f, 1.0f, 0.0f);

        meshMaterialMap[mesh->material.name] = group;
        //materialMap[mesh->material.name] = mesh->material;

        // Init full BVH tree with 3 inherite subtrees
        splitter::initBVH(group);
      } else {
        group = meshMaterialMap[mesh->material.name];
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

  GroupObject resultGroup = GroupObject(new Group());
  resultGroup->name = baseObject->name;

  int meshIndex = 0;
  for (std::map<std::string, GroupObject>::iterator it = meshMaterialMap.begin(); it != meshMaterialMap.end(); ++it) {
    it->second->traverse([&](MeshObject mesh){
      //std::cout << "Mesh \"" << mesh->name << "\" has: " << mesh->faces.size() << " faces" << std::endl;
      mesh->hasNormals = it->second->meshes[0]->normal.size() > 0;
      mesh->hasUVs = it->second->meshes[0]->uv.size() > 0;
      mesh->remesh(it->second->meshes[0]->position, it->second->meshes[0]->normal, it->second->meshes[0]->uv);
      mesh->finish();

      mesh->material = it->second->meshes[0]->material;
      mesh->material.name += std::to_string(meshIndex);
      mesh->material.diffuseMap = mesh->material.name + ".jpg";
      //mesh->material.diffuseMapImage = it->second->meshes[0]->material.diffuseMapImage;

      meshIndex++;

      if (mesh->hasUVs) {
        BBoxf uvBox = mesh->uvBox;
        // uvBox.fromPoint(mesh->uv[0].x, mesh->uv[0].y, 0.0f);

        // for (long long unsigned int i = 1; i < mesh->uv.size(); i++) {
        //   uvBox.extend(mesh->uv[i].x, mesh->uv[i].y, 0.0f);
        // }

        int minX, minY, maxX, maxY;

        //Material meshMaterial = materialMap[it->first];

        minX = floor(uvBox.min.x * it->second->meshes[0]->material.diffuseMapImage.width);
        minY = floor(uvBox.min.y * it->second->meshes[0]->material.diffuseMapImage.height);
        maxX = ceil(uvBox.max.x * it->second->meshes[0]->material.diffuseMapImage.width);
        maxY = ceil(uvBox.max.y * it->second->meshes[0]->material.diffuseMapImage.height);

        //std::cout << "Calculated Box min x/y: " << minX << "/" << minY << " max x/y: " << maxX << "/" << maxY << std::endl;
        //std::cout << "Real Box min x/y: " << uvBox.min.x << "/" << uvBox.min.y << " max x/y: " << uvBox.max.x << "/" << uvBox.max.y << std::endl;

        float offsetX1 = (float) minX / (float) it->second->meshes[0]->material.diffuseMapImage.width;
        float offsetX2 = (float) maxX / (float) it->second->meshes[0]->material.diffuseMapImage.width;

        float offsetY1 = (float) minY / (float) it->second->meshes[0]->material.diffuseMapImage.height;
        float offsetY2 = (float) maxY / (float) it->second->meshes[0]->material.diffuseMapImage.height;

        float UVWidth = offsetX2 - offsetX1;
        float UVHeight = offsetY2 - offsetY1;

        for (long long unsigned int i = 0; i < mesh->uv.size(); i++) {
          // uvBox.extend(mesh->uv[i].x, mesh->uv[i].y, 0.0f);
          mesh->uv[i].x = (mesh->uv[i].x - offsetX1) / UVWidth;
          mesh->uv[i].y = (mesh->uv[i].y - offsetY1) / UVHeight;
        }
        
        int textureWidth = maxX - minX;
        int textureHeight = maxY - minY;

        // Image &diffuse = mesh->material.diffuseMapImage;
        Image diffuse;

        diffuse.channels = 3;
        diffuse.width = textureWidth;
        diffuse.height = textureHeight;
        diffuse.data = new unsigned char[textureWidth * textureHeight * 3];
        
        
        for (int j = 0; j < textureWidth; j++)
        {
          for (int i = 0; i < textureHeight; i++)
          {
            int textureY = textureHeight - i - 1;
            int originPointer = ((it->second->meshes[0]->material.diffuseMapImage.height - 1 - i - minY) * it->second->meshes[0]->material.diffuseMapImage.width * diffuse.channels) + ((j + minX) * diffuse.channels);
            int targetPointer = ((textureHeight - 1 - i) * diffuse.width * diffuse.channels) + (j * diffuse.channels);

            diffuse.data[targetPointer] = it->second->meshes[0]->material.diffuseMapImage.data[originPointer];
            diffuse.data[targetPointer + 1] = it->second->meshes[0]->material.diffuseMapImage.data[originPointer + 1];
            diffuse.data[targetPointer + 2] = it->second->meshes[0]->material.diffuseMapImage.data[originPointer + 2];
          }
        }

        mesh->material.diffuseMapImage = diffuse;
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

  for (std::map<std::string, GroupObject>::iterator it = meshMaterialMap.begin(); it != meshMaterialMap.end(); ++it) {
    it->second->meshes[0]->free();
    it->second->meshes.clear();
  }

  // baseObject->traverse([](MeshObject mesh) {
  //   mesh->free();
  // });

  return resultGroup;
};

std::vector<GroupObject> splitter::splitObject(GroupObject baseObject, GroupCallback fn, int x = 1, int y = 1, int z = 1) {
  std::vector<GroupObject> result;

  BBoxf box = baseObject->boundingBox;
  Vector3f size = box.size();

  size.x *= 1.0 / float(x);
  size.y *= 1.0 / float(y);
  size.z *= 1.0 / float(z);

  BBoxf partialBox;
  partialBox.min.x = box.min.x;
  partialBox.min.y = box.min.y;
  partialBox.min.z = box.min.z;

  partialBox.max.x = box.min.x + size.x;
  partialBox.max.y = box.min.y + size.y;
  partialBox.max.z = box.min.z + size.z;

  BBoxf uvBox;
  bool uvBoxInitialized = false;

  std::map<std::string, bool> vectorOverlap;

  for (int dx = 0; dx < x; dx++) {
    for (int dy = 0; dy < y; dy++) {
      for (int dz = 0; dz < z; dz++) {
        GroupObject group = GroupObject(new Group());
        group->name = std::to_string(dx) + "_" + std::to_string(dy) + "_" + std::to_string(dz);

        partialBox.translate(box.min.x + size.x * float(dx), box.min.y + size.y * float(dy), box.min.z + size.z * float(dz));

        baseObject->traverse([&](MeshObject mesh) {
          MeshObject nextMesh = MeshObject(new Mesh());
          nextMesh->name = mesh->name;
          nextMesh->material = mesh->material;

          // int width = nextMesh->material.diffuseMapImage.width;
          // int height = nextMesh->material.diffuseMapImage.height;
          // unsigned char *data = new unsigned char[width * height * nextMesh->material.diffuseMapImage.channels];
          
          // nextMesh->material.diffuseMapImage.width = 1024;
          // nextMesh->material.diffuseMapImage.height = 1024;
          // nextMesh->material.diffuseMapImage.data = new unsigned char[1024 * 1024 * 3];
          
          // int index = 0;
          // for (int j = 1024 - 1; j >= 0; --j)
          // {
          //   for (int i = 0; i < 1024; ++i)
          //   {
          //     float r = (float)i / (float)1024;
          //     float g = (float)j / (float)1024;
          //     float b = 0.2f;
          //     int ir = int(255.99 * r);
          //     int ig = int(255.99 * g);
          //     int ib = int(255.99 * b);

          //     nextMesh->material.diffuseMapImage.data[index++] = ir;
          //     nextMesh->material.diffuseMapImage.data[index++] = ig;
          //     nextMesh->material.diffuseMapImage.data[index++] = ib;
          //   }
          // }
          

          std::map<unsigned int, Vector3f> positionMap;
          std::map<unsigned int, Vector3f> normalMap;
          std::map<unsigned int, Vector2f> uvMap;

          std::map<unsigned int, unsigned int> positionDestMap;
          std::map<unsigned int, unsigned int> normalDestMap;
          std::map<unsigned int, unsigned int> uvDestMap;

          for (Face &face : mesh->faces) // access by reference to avoid copying
          {
            bool intersectsA = partialBox.intersect(mesh->position[face.positionIndices[0]]);
            bool intersectsB = partialBox.intersect(mesh->position[face.positionIndices[1]]);
            bool intersectsC = partialBox.intersect(mesh->position[face.positionIndices[2]]);

            bool intersects = intersectsA || intersectsB || intersectsC;
            if (intersects) {
              vectorOverlap.clear();
              vectorOverlap["A"] = false;
              vectorOverlap["B"] = false;
              vectorOverlap["C"] = false;

              if (intersectsA) vectorOverlap["A"] = true;
              if (intersectsB) vectorOverlap["B"] = true;
              if (intersectsC) vectorOverlap["C"] = true;

              Face nextFace = face;
              nextMesh->faces.push_back(nextFace);

              positionMap[face.positionIndices[0]] = mesh->position[face.positionIndices[0]];
              positionMap[face.positionIndices[1]] = mesh->position[face.positionIndices[1]];
              positionMap[face.positionIndices[2]] = mesh->position[face.positionIndices[2]];

              if (mesh->hasNormals) {
                normalMap[face.normalIndices[0]] = mesh->normal[face.normalIndices[0]];
                normalMap[face.normalIndices[1]] = mesh->normal[face.normalIndices[1]];
                normalMap[face.normalIndices[2]] = mesh->normal[face.normalIndices[2]];
              }

              if (mesh->hasUVs) {
                uvMap[face.uvIndices[0]] = mesh->uv[face.uvIndices[0]];
                uvMap[face.uvIndices[1]] = mesh->uv[face.uvIndices[1]];
                uvMap[face.uvIndices[2]] = mesh->uv[face.uvIndices[2]];
              }
            }
          }

          unsigned int lastPositionIndex = 0;
          unsigned int lastNormalIndex = 0;
          unsigned int lastUVIndex = 0;

          for (std::map<unsigned int, Vector3f>::iterator it = positionMap.begin(); it != positionMap.end(); ++it) {
            nextMesh->position.push_back(it->second);
            positionDestMap[it->first] = lastPositionIndex;

            lastPositionIndex++;
          }

          if (mesh->hasNormals) {
            for (std::map<unsigned int, Vector3f>::iterator it = normalMap.begin(); it != normalMap.end(); ++it) {
              nextMesh->normal.push_back(it->second);
              normalDestMap[it->first] = lastNormalIndex;

              lastNormalIndex++;
            }
          }

          if (mesh->hasUVs) {
            for (std::map<unsigned int, Vector2f>::iterator it = uvMap.begin(); it != uvMap.end(); ++it) {
              nextMesh->uv.push_back(it->second);
              uvDestMap[it->first] = lastUVIndex;

              lastUVIndex++;

              if (!uvBoxInitialized) {
                uvBoxInitialized = true;
                nextMesh->uvBox.fromPoint(it->second.x, it->second.y, 0.0f);
              } else {
                nextMesh->uvBox.extend(it->second.x, it->second.y, 0.0f);
              }
            }
          }

          for (Face &face : nextMesh->faces) // access by reference to avoid copying
          {
            face.positionIndices[0] = positionDestMap[face.positionIndices[0]];
            face.positionIndices[1] = positionDestMap[face.positionIndices[1]];
            face.positionIndices[2] = positionDestMap[face.positionIndices[2]];

            if (mesh->hasNormals) {
              face.normalIndices[0] = normalDestMap[face.normalIndices[0]];
              face.normalIndices[1] = normalDestMap[face.normalIndices[1]];
              face.normalIndices[2] = normalDestMap[face.normalIndices[2]];
            }

            if (mesh->hasUVs) {
              face.uvIndices[0] = uvDestMap[face.uvIndices[0]];
              face.uvIndices[1] = uvDestMap[face.uvIndices[1]];
              face.uvIndices[2] = uvDestMap[face.uvIndices[2]];
            }
          }

          if (nextMesh->position.size() > 0) {
            std::cout << "Mesh is correct: " << nextMesh->name << ", vertices count: " << nextMesh->position.size() << ", material name: " << nextMesh->material.name << std::endl;
            nextMesh->finish();
            group->meshes.push_back(nextMesh);

            uvBoxInitialized = false;

            fn(splitter::splitUV(group));

            // group->meshes.clear();
            // group->children.clear();
          }
        });
      }
    }
  }

  return result;
};