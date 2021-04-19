#include "./ObjExporter.h"

void ObjExporter::save(std::string directory, GroupObject object) {
  std::string exportModelPath = utils::concatPath(directory, "model.obj");
  std::cout << "Output Directory: " << directory.c_str() << std::endl; 

  if (!utils::folder_exists(directory)) {
    mkdir(directory.c_str());
  }

  std::fstream fs;
  fs.open (exportModelPath, std::fstream::out);

  int lastVertex = 1;
  int lastNormal = 1;
  int lastUV = 1;

  object->traverse([&](MeshObject object){
    fs << "o " << object->name.c_str() << std::endl;
    for (Vector3f &position : object->position) // access by reference to avoid copying
    {
      fs << "v " << position.x << " " << position.y << " " << position.z << std::endl;
    }

    if (object->hasNormals) {
      for (Vector3f &normal : object->normal) // access by reference to avoid copying
      {
        fs << "vn " << normal.x << " " << normal.y << " " << normal.z << std::endl;
      }
    }

    if (object->hasUVs) {
      for (Vector2f &uv : object->uv) // access by reference to avoid copying
      {
        fs << "vt " << uv.x << " " << uv.y << std::endl;
      }
    }

    for (Face &face : object->faces) // access by reference to avoid copying
    {
      fs << "f "
      << face.positionIndices[0] + lastVertex << "/" << face.uvIndices[0] + lastUV << "/" << face.normalIndices[0] + lastNormal << " "
      << face.positionIndices[1] + lastVertex << "/" << face.uvIndices[1] + lastUV << "/" << face.normalIndices[1] + lastNormal << " "
      << face.positionIndices[2] + lastVertex << "/" << face.uvIndices[2] + lastUV << "/" << face.normalIndices[2] + lastNormal << std::endl;
    }

    lastVertex += object->position.size();
    lastUV += object->uv.size();
    lastNormal += object->normal.size();
  });


  fs.close();


  /*
  this->object->traverse([&](MeshObject object){
    std::cout << "Mesh name: " << object->name.c_str() << std::endl;
  });
  */
}