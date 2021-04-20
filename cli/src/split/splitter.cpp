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

  for (int dx = 0; dx < x; dx++) {
    for (int dy = 0; dy < y; dy++) {
      for (int dz = 0; dz < z; dz++) {
        GroupObject group = GroupObject(new Group());
        group->name = std::to_string(dx) + "_" + std::to_string(dy) + "_" + std::to_string(dz);

        partialBox.translate(box.min.x + size.x * float(dx), box.min.y + size.y * float(dy), box.min.z + size.z * float(dz));

        baseObject->traverse([&](MeshObject mesh){
          MeshObject nextMesh = MeshObject(new Mesh());
          nextMesh->name = mesh->name;
          nextMesh->material = mesh->material;
          unsigned int currentIndex = 0;
          for (unsigned int i = 0; i < mesh->position.size(); i += 3) // access by reference to avoid copying
          {
            bool intersects = partialBox.intersect(mesh->position[i]) || partialBox.intersect(mesh->position[i + 1]) || partialBox.intersect(mesh->position[i + 2]);
            if (intersects) {


              Face face;

              nextMesh->position.push_back(mesh->position[i]);
              nextMesh->position.push_back(mesh->position[i + 1]);
              nextMesh->position.push_back(mesh->position[i + 2]);

              face.positionIndices[0] = currentIndex;
              face.positionIndices[1] = currentIndex + 1;
              face.positionIndices[2] = currentIndex + 2;

              face.normalIndices[0] = currentIndex;
              face.normalIndices[1] = currentIndex + 1;
              face.normalIndices[2] = currentIndex + 2;

              face.uvIndices[0] = currentIndex;
              face.uvIndices[1] = currentIndex + 1;
              face.uvIndices[2] = currentIndex + 2;

              if (mesh->hasNormals) {
                nextMesh->normal.push_back(mesh->normal[i]);
                nextMesh->normal.push_back(mesh->normal[i + 1]);
                nextMesh->normal.push_back(mesh->normal[i + 2]);
              }

              if (mesh->hasUVs) {
                nextMesh->uv.push_back(mesh->uv[i]);
                nextMesh->uv.push_back(mesh->uv[i + 1]);
                nextMesh->uv.push_back(mesh->uv[i + 2]);
              }

              nextMesh->faces.push_back(face);
              currentIndex += 3;
            }
          }

          if (nextMesh->position.size() > 0) {
            std::cout << "Mesh is correct: " << nextMesh->name << ", vertices count: " << nextMesh->position.size() << ", material name: " << nextMesh->material.name << std::endl;
            nextMesh->finish();
            group->meshes.push_back(nextMesh);
          }
        });

        result.push_back(group);
      }
    }
  }

  return result;
};