#include "ObjLoader.h"

MaterialMap ObjLoader::loadMaterials(const char* path) {
  MaterialMap materialMap;
  std::ifstream input;
  // open the file stream
  input.open(path);
  // check if opening a file failed
  if (input.fail()) {
    std::cerr << "Error opeing a material file" << std::endl;
    input.close();
    
    return materialMap;
  }

  std::cout << "Materials file is opened, processing...: " << std::endl;

  std::string line;
  std::string token;
  std::stringstream ss;


  std::string lastMaterialName = "";

  while (getline(input, line))
  {
    ss.clear();
    ss.str(line);

    ss >> token;

    if (token == "newmtl") {
      ss >> lastMaterialName;

      Material currentMaterial;
      currentMaterial.name = lastMaterialName;

      materialMap[lastMaterialName] = currentMaterial;
    } else if (lastMaterialName != "") {
      if (token == "map_Kd") {
        ss >> materialMap[lastMaterialName].diffuseMap;
      } else if (token == "Kd") {
        ss >> materialMap[lastMaterialName].color.x >> materialMap[lastMaterialName].color.y >> materialMap[lastMaterialName].color.z;
      }
    }
  }

  input.close();

  std::cout << "Found " << materialMap.size() << " materials" << std::endl;

  return materialMap;
};

void ObjLoader::parse(const char* path) {
  std::ifstream input;
  // open the file stream
  input.open(path);
  // check if opening a file failed
  if (input.fail()) {
      std::cerr << "Error opeing a model file" << std::endl;
      input.close();
      exit(1);
  }

  std::cout << "Model file is opened, processing..." << std::endl;
  
  std::string line;
  std::string token;

  GroupObject currentGroup = this->object;
  MeshObject currentMesh = MeshObject(new Mesh());

  std::stringstream ss;
  std::stringstream faceReader;

  std::string faceData;
  std::vector<std::string> faces;
  std::vector<std::string> tokens;

  std::vector<Vector3f> position;
  std::vector<Vector3f> normal;
  std::vector<Vector2f> uv;

  MaterialMap materialMap;

  while (getline(input, line))
  {
    ss.clear();
    ss.str(line);

    ss >> token;

    if (token == "mtllib") { // Process objects
      std::string materialFile;
      ss >> materialFile;

      if (materialFile != "") {
        materialMap = this->loadMaterials(utils::concatPath(utils::getDirectory(path), materialFile).c_str());
      }
    } else if (token == "old one o") { // Process objects
      if (currentMesh != nullptr) {
        currentMesh->finish();
        std::cout << "Finished by o" << std::endl;
        currentGroup->meshes.push_back(currentMesh);
      }

      currentMesh = MeshObject(new Mesh());
      ss >> currentMesh->name;
    } else if (token == "usemtl") {
      if (currentMesh != nullptr && currentMesh->faces.size() > 0) {
        currentMesh->finish();
        std::cout << "Finished by the new material" << std::endl;
        currentGroup->meshes.push_back(currentMesh);
      }

      std::string meshMaterialName;
      ss >> meshMaterialName;

      std::cout << "New mtl: " << meshMaterialName << std::endl;

      currentMesh = MeshObject(new Mesh());
      currentMesh->name = currentGroup->name + "_" + meshMaterialName;

      if (materialMap.find(meshMaterialName) != materialMap.end()) {
        currentMesh->material = materialMap[meshMaterialName];
      }
    } else if (token == "g" || token == "o") { // Process groups
      std::string groupName;
      ss >> groupName;

      if (currentGroup->name != groupName) {
        GroupObject nextGroup = GroupObject(new Group());
        nextGroup->name = groupName;

        currentGroup->children.push_back(nextGroup);
        currentGroup = nextGroup;
      }
    } else if (token == "v") { // Process vertices
      Vector3f vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      position.push_back(vertex);
    } else if (token == "vn") { // Process normals
      Vector3f vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      normal.push_back(vertex);
    } else if (token == "vt") { // Process uvs
      Vector2f vertex;
      ss >> vertex.x >> vertex.y;

      uv.push_back(vertex);
    } else if (token == "f") { // Process faces
      ss.ignore(1,' '); // Ignore space after 'f'
      tokens.clear();

      while(std::getline(ss, faceData, ' '))
      {
        tokens.push_back(faceData);
      }

      int points = tokens.size();
      int* positionIndices = new int[points];
      int* uvIndices = new int[points];
      int* normalIndices = new int[points];
      
      bool hasNormals = false;
      bool hasUVs = false;

      for (int i = 0; i < points; i++) {
        faceReader.clear();
        faceReader.str(tokens[i]);
        faceData = "";
        faces.clear();

        while(std::getline(faceReader, faceData, '/'))
        {
          faces.push_back(faceData);
        }

        int components = faces.size();
        // Obj starts counting faces from 1, so we need to sub 1
        for (int k = 0; k < components; k++) {
          // if (faces[k] == "") continue;
          if (k == 0) {
            positionIndices[i] = (faces[k] == "") ? -1 : std::atoi(faces[0].c_str()) - 1;// - currentVertex;
          } else if (k == 1) {
            uvIndices[i] = (faces[k] == "") ? -1 : std::atoi(faces[1].c_str()) - 1;// - currentUV;
            hasUVs = true;
          } else if (k == 2) {
            normalIndices[i] = (faces[k] == "") ? -1 : std::atoi(faces[2].c_str()) - 1;// - currentNormal;
            hasNormals = true;
          }
        }
      }
      
      for (int t = 1; t < points - 1; t += 1) {
        Face face;

        int currentIndex = currentMesh->position.size();

        face.positionIndices[0] = currentIndex;
        face.positionIndices[1] = currentIndex + 1;
        face.positionIndices[2] = currentIndex + 2;

        face.uvIndices[0] = currentIndex;
        face.uvIndices[1] = currentIndex + 1;
        face.uvIndices[2] = currentIndex + 2;

        face.normalIndices[0] = currentIndex;
        face.normalIndices[1] = currentIndex + 1;
        face.normalIndices[2] = currentIndex + 2;

        currentMesh->boundingBox.extend(position[positionIndices[0]]);
        currentMesh->boundingBox.extend(position[positionIndices[t]]);
        currentMesh->boundingBox.extend(position[positionIndices[t + 1]]);

        currentMesh->position.push_back(position[positionIndices[0]]);
        currentMesh->position.push_back(position[positionIndices[t]]);
        currentMesh->position.push_back(position[positionIndices[t + 1]]);
        
        if (hasNormals && normalIndices[0] != -1) {
          currentMesh->normal.push_back(normal[normalIndices[0]]);
          currentMesh->normal.push_back(normal[normalIndices[t]]);
          currentMesh->normal.push_back(normal[normalIndices[t + 1]]);
        }

        if (hasUVs && uvIndices[0] != -1) {
          currentMesh->uv.push_back(uv[uvIndices[0]]);
          currentMesh->uv.push_back(uv[uvIndices[t]]);
          currentMesh->uv.push_back(uv[uvIndices[t + 1]]);
        }

        currentMesh->faces.push_back(face);
      }
    }
  }

  std::cout << "Finished before by end of func" << std::endl;
  currentMesh->finish();
  std::cout << "Finished by end of func" << std::endl;

  currentGroup->meshes.push_back(currentMesh);
  this->object->computeBoundingBox();

  // close the file stream
  input.close();
};
