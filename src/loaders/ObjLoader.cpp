#include "ObjLoader.h"


bool ObjLoader::loadTexture(std::shared_ptr<TextureLoadTask> task) {
  task->image->data = stbi_load(task->texturePath.c_str(), &task->image->width, &task->image->height, &task->image->channels, 0);

  if(task->image->data == NULL) {
    std::cerr << "Image loading error: " << task->texturePath.c_str() << std::endl;
    if (stbi_failure_reason()) std::cerr << stbi_failure_reason() << std::endl;

    return false;
  }

  return true;
};

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

  std::map<std::string, unsigned char*> imageList;

  std::map<std::string, Image> imageMap;

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

      MaterialObject currentMaterial = std::make_shared<Material>();
      currentMaterial->name = lastMaterialName;
      currentMaterial->baseName = lastMaterialName;

      materialMap[lastMaterialName] = currentMaterial;
    } else if (lastMaterialName != "") {
      if (token == "map_Kd") {
        ss >> materialMap[lastMaterialName]->diffuseMap;

        std::string imagePath = utils::concatPath(
            utils::getDirectory(path),
            materialMap[lastMaterialName]->diffuseMap
          ).c_str();


        Image &diffuseMapImage = materialMap[lastMaterialName]->diffuseMapImage;

        if ( imageMap.find(imagePath) == imageMap.end() ) {
          imageMap[imagePath] = diffuseMapImage;
          // std::cout << "Found an image:" << materialMap[lastMaterialName]->diffuseMap.c_str() << std::endl;

          std::cout << "Loading image: " << imagePath.c_str() << std::endl;
          // std::cout << "Loading..." << std::endl;

          // Image image;
          
          std::shared_ptr<TextureLoadTask> task = std::make_shared<TextureLoadTask>();
          task->image = &materialMap[lastMaterialName]->diffuseMapImage;
          task->texturePath = imagePath.c_str();

          this->pool.create(
            bind(&ObjLoader::loadTexture, this, std::placeholders::_1),
            task
          );

          this->pool.waitForSlot();
          
          /*
          materialMap[lastMaterialName]->diffuseMapImage.data = stbi_load(
            imagePath.c_str(),
            &materialMap[lastMaterialName]->diffuseMapImage.width,
            &materialMap[lastMaterialName]->diffuseMapImage.height,
            &materialMap[lastMaterialName]->diffuseMapImage.channels,
            0
          );

          if(materialMap[lastMaterialName]->diffuseMapImage.data == NULL) {
            std::cerr << "Image loading error" << std::endl;
            if (stbi_failure_reason()) std::cerr << stbi_failure_reason() << std::endl;
          }
          */

          // imageMap[imagePath] = diffuseMapImage;

          // imageList[imagePath] = stbi_load(imagePath.c_str(), &diffuseMapImage.width, &diffuseMapImage.height, &diffuseMapImage.channels, 0);

          // if(imageList[imagePath] == NULL) {
          //   std::cerr << "Image loading error" << std::endl;
          //   if (stbi_failure_reason()) std::cerr << stbi_failure_reason() << std::endl;
          // }

          // std::cout << "Image width: " << diffuseMapImage.width << ", height: " << diffuseMapImage.height << ", channels: " << diffuseMapImage.channels << std::endl;
        }
        
        // diffuseMapImage.data = imageList[imagePath];
        materialMap[lastMaterialName]->diffuseMapImage = imageMap[imagePath];

        // if(diffuseMapImage.data == NULL) {
        //   std::cerr << "Image loading error" << std::endl;
        //   if (stbi_failure_reason()) std::cerr << stbi_failure_reason() << std::endl;
        // }
      } else if (token == "Kd") {
        ss >> materialMap[lastMaterialName]->color.x >> materialMap[lastMaterialName]->color.y >> materialMap[lastMaterialName]->color.z;
      }
    }
  }

  this->pool.finish();

  input.close();
  imageList.clear();

  std::cout << "Imported " << materialMap.size() << " textures" << std::endl;

  return materialMap;
};

void ObjLoader::finishMesh(GroupObject &group, MeshObject &mesh, std::vector<glm::vec3> &position, std::vector<glm::vec3> &normal, std::vector<glm::vec2> &uv) {
  std::map<unsigned int, glm::vec3> positionMap;
  std::map<unsigned int, glm::vec3> normalMap;
  std::map<unsigned int, glm::vec2> uvMap;

  std::map<unsigned int, unsigned int> positionDestMap;
  std::map<unsigned int, unsigned int> normalDestMap;
  std::map<unsigned int, unsigned int> uvDestMap;

  for (Face &face : mesh->faces) // access by reference to avoid copying
  {
    positionMap[face.positionIndices[0]] = position[face.positionIndices[0]];
    positionMap[face.positionIndices[1]] = position[face.positionIndices[1]];
    positionMap[face.positionIndices[2]] = position[face.positionIndices[2]];

    if (mesh->hasNormals) {
      normalMap[face.normalIndices[0]] = normal[face.normalIndices[0]];
      normalMap[face.normalIndices[1]] = normal[face.normalIndices[1]];
      normalMap[face.normalIndices[2]] = normal[face.normalIndices[2]];
    }

    if (mesh->hasUVs) {
      uvMap[face.uvIndices[0]] = uv[face.uvIndices[0]];
      uvMap[face.uvIndices[1]] = uv[face.uvIndices[1]];
      uvMap[face.uvIndices[2]] = uv[face.uvIndices[2]];
    }
  }

  unsigned int lastPositionIndex = 0;
  unsigned int lastNormalIndex = 0;
  unsigned int lastUVIndex = 0;

  for (std::map<unsigned int, glm::vec3>::iterator it = positionMap.begin(); it != positionMap.end(); ++it) {
    mesh->position.push_back(it->second);
    positionDestMap[it->first] = lastPositionIndex;

    lastPositionIndex++;
  }

  if (mesh->hasNormals) {
    for (std::map<unsigned int, glm::vec3>::iterator it = normalMap.begin(); it != normalMap.end(); ++it) {
      mesh->normal.push_back(it->second);
      normalDestMap[it->first] = lastNormalIndex;

      lastNormalIndex++;
    }
  }

  if (mesh->hasUVs) {
    for (std::map<unsigned int, glm::vec2>::iterator it = uvMap.begin(); it != uvMap.end(); ++it) {
      mesh->uv.push_back(it->second);
      uvDestMap[it->first] = lastUVIndex;

      lastUVIndex++;
    }
  }

  for (Face &face : mesh->faces) // access by reference to avoid copying
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

  // std::cout << "Mesh loading finished." << std::endl;

  mesh->finish();
  group->meshes.push_back(mesh);

  positionMap.clear();
  normalMap.clear();
  uvMap.clear();

  positionDestMap.clear();
  normalDestMap.clear();
  uvDestMap.clear();

  position.clear();
  normal.clear();
  uv.clear();
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

  std::vector<glm::vec3> position;
  std::vector<glm::vec3> normal;
  std::vector<glm::vec2> uv;

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
        // currentMesh->finish();
        // std::cout << "Finished by the new material" << std::endl;
        this->finishMesh(currentGroup, currentMesh, position, normal, uv);
      }

      std::string meshMaterialName;
      ss >> meshMaterialName;

      // std::cout << "New mtl: " << meshMaterialName << std::endl;

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
      glm::vec3 vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      position.push_back(vertex);
    } else if (token == "vn") { // Process normals
      glm::vec3 vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      normal.push_back(vertex);
    } else if (token == "vt") { // Process uvs
      glm::vec2 vertex;
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
            // hasUVs = true;
            currentMesh->hasUVs = true;
          } else if (k == 2) {
            normalIndices[i] = (faces[k] == "") ? -1 : std::atoi(faces[2].c_str()) - 1;// - currentNormal;
            // hasNormals = true;
            currentMesh->hasNormals = true;
          }
        }
      }
      
      for (int t = 1; t < points - 1; t += 1) {
        Face face;

        face.positionIndices[0] = positionIndices[0];
        face.positionIndices[1] = positionIndices[t];
        face.positionIndices[2] = positionIndices[t + 1];

        if (currentMesh->hasUVs) {
          face.uvIndices[0] = uvIndices[0];
          face.uvIndices[1] = uvIndices[t];
          face.uvIndices[2] = uvIndices[t + 1];
        }

        if (currentMesh->hasNormals) {
          face.normalIndices[0] = normalIndices[0];
          face.normalIndices[1] = normalIndices[t];
          face.normalIndices[2] = normalIndices[t + 1];
        }

        currentMesh->boundingBox.extend(position[positionIndices[0]]);
        currentMesh->boundingBox.extend(position[positionIndices[t]]);
        currentMesh->boundingBox.extend(position[positionIndices[t + 1]]);

        currentMesh->faces.push_back(face);
      }

      delete[] positionIndices;
      delete[] uvIndices;
      delete[] normalIndices;
    }
  }

  //std::cout << "Finished before by end of func" << std::endl;
  // currentMesh->finish();
  std::cout << "Model has been loaded" << std::endl;

  //currentGroup->meshes.push_back(currentMesh);
  this->finishMesh(currentGroup, currentMesh, position, normal, uv);
  this->object->computeBoundingBox();

  materialMap.clear();
  faces.clear();
  tokens.clear();

  // close the file stream
  input.close();
};
