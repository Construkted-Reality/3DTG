#include "./ObjExporter.h"

void ObjExporter::saveMaterial(std::string directory, std::string fileName, MaterialMap materialMap) {
  std::string exportMaterialPath = utils::concatPath(directory, fileName + ".mtl");

  if (!utils::folder_exists(directory)) {
    mkdir(directory.c_str());
  }

  std::fstream fs;
  fs.open (exportMaterialPath, std::fstream::out);

  std::for_each(
    materialMap.begin(),
    materialMap.end(),
    [&](std::pair<std::string, Material> it) {
      // std::cout << "Material name: " << it.first << std::endl;//  + it.second
      // std::cout << "Saving..." << std::endl;

      fs << "newmtl " << it.first.c_str() << std::endl;

      fs << "Kd " << it.second.color.x << " " << it.second.color.y <<  " " << it.second.color.z << std::endl;
      
      if (it.second.diffuseMapImage.data != NULL) {
        fs << "map_Kd " << it.second.diffuseMap << std::endl;

        std::string outputImage = utils::concatPath(directory, it.second.name + ".jpg");

        stbi_write_jpg(
          outputImage.c_str(),
          it.second.diffuseMapImage.width,
          it.second.diffuseMapImage.height,
          it.second.diffuseMapImage.channels,
          it.second.diffuseMapImage.data,
          80
        );
      }

      fs << std::endl;
    }
  );

  fs.close();
};

void ObjExporter::save(std::string directory, std::string fileName, GroupObject object) {
  std::string exportModelPath = utils::concatPath(directory, fileName + ".obj");
  std::string materialFileName = fileName + ".mtl";

  if (!utils::folder_exists(directory)) {
    utils::mkdir(directory.c_str());
  }

  std::fstream fs;
  fs.open (exportModelPath, std::fstream::out);

  int lastVertex = 1;
  int lastNormal = 1;
  int lastUV = 1;

  MaterialMap materialMap;
  // std::cout << "Export to .OBJ has been started" << std::endl;

  fs << "# geometricError " << object->geometricError << std::endl;

  fs << "mtllib " << materialFileName.c_str() << std::endl;

  object->traverse([&](MeshObject object){
    if (object->material.name != "") {
      materialMap[object->material.name] = object->material;
      fs << "usemtl " << object->material.name.c_str() << std::endl;
    }

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
      if (object->hasUVs && object->hasNormals) {
        fs << "f "
        << face.positionIndices[0] + lastVertex << "/" << face.uvIndices[0] + lastUV << "/" << face.normalIndices[0] + lastNormal << " "
        << face.positionIndices[1] + lastVertex << "/" << face.uvIndices[1] + lastUV << "/" << face.normalIndices[1] + lastNormal << " "
        << face.positionIndices[2] + lastVertex << "/" << face.uvIndices[2] + lastUV << "/" << face.normalIndices[2] + lastNormal << std::endl;
      } else if (object->hasNormals) {
        fs << "f "
        << face.positionIndices[0] + lastVertex << "/" << "/" << face.normalIndices[0] + lastNormal << " "
        << face.positionIndices[1] + lastVertex << "/" << "/" << face.normalIndices[1] + lastNormal << " "
        << face.positionIndices[2] + lastVertex << "/" << "/" << face.normalIndices[2] + lastNormal << std::endl;
      } else if (object->hasUVs) {
        fs << "f "
        << face.positionIndices[0] + lastVertex << "/" << face.uvIndices[0] + lastUV << " "
        << face.positionIndices[1] + lastVertex << "/" << face.uvIndices[1] + lastUV << " "
        << face.positionIndices[2] + lastVertex << "/" << face.uvIndices[2] + lastUV << std::endl;
      } else {
        fs << "f "
        << face.positionIndices[0] + lastVertex << " "
        << face.positionIndices[1] + lastVertex << " "
        << face.positionIndices[2] + lastVertex << std::endl;
      }
    }

    lastVertex += object->position.size();
    lastUV += object->uv.size();
    lastNormal += object->normal.size();
  });

  fs.close();

  // std::cout << "Materials for saving: " << materialMap.size() << std::endl;
  this->saveMaterial(directory, fileName, materialMap);
}