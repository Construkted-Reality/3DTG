#include "./splitter.h"

std::vector<GroupObject> splitter::splitObject(GroupObject baseObject, int x = 1, int y = 1, int z = 1) {
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
          /*
          int index = 0;
          for (int j = 1024 - 1; j >= 0; --j)
          {
            for (int i = 0; i < 1024; ++i)
            {
              float r = (float)i / (float)1024;
              float g = (float)j / (float)1024;
              float b = 0.2f;
              int ir = int(255.99 * r);
              int ig = int(255.99 * g);
              int ib = int(255.99 * b);

              nextMesh->material.diffuseMapImage.data[index++] = 0;
              nextMesh->material.diffuseMapImage.data[index++] = 0;
              nextMesh->material.diffuseMapImage.data[index++] = 0;
            }
          }
          */

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
          }
        });

        group->computeUVBox();
        result.push_back(group);
      }
    }
  }

  return result;
};