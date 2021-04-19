#include "ObjLoader.h"



void ObjLoader::parse(const char* path) {
  std::ifstream input;
  // open the file stream
  input.open(path);
  // check if opening a file failed
  if (input.fail()) {
      std::cerr << "Error opeing a file" << std::endl;
      input.close();
      exit(1);
  }
  
  std::string line;
  std::string token;

  GroupObject currentGroup = this->object;
  MeshObject currentMesh = nullptr;

  unsigned int currentVertex = 0;
  unsigned int currentNormal = 0;
  unsigned int currentUV = 0;

  unsigned int lastVertex = 0;
  unsigned int lastNormal = 0;
  unsigned int lastUV = 0;

  std::stringstream ss;
  std::stringstream faceReader;

  std::string faceData;
  std::vector<std::string> faces;
  std::vector<std::string> tokens;

  while (getline(input, line))
  {
    ss.clear();
    ss.str(line);

    ss >> token;

    if (token == "o") { // Process objects
      if (currentMesh != nullptr) {
        currentMesh->finish();
        currentGroup->meshes.push_back(currentMesh);
      }

      currentMesh = MeshObject(new Mesh());
      ss >> currentMesh->name;

      currentVertex = lastVertex;
      currentNormal = lastNormal;
      currentUV = lastUV;
    } else if (token == "g") { // Process groups
      GroupObject nextGroup = GroupObject(new Group());
      ss >> nextGroup->name;

      currentGroup->children.push_back(nextGroup);
      currentGroup = nextGroup;
    } else if (token == "v") { // Process vertices
      Vector3f vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      currentMesh->position.push_back(vertex);
      lastVertex++;
    } else if (token == "vn") { // Process normals
      Vector3f vertex;
      ss >> vertex.x >> vertex.y >> vertex.z;

      currentMesh->normal.push_back(vertex);
      lastNormal++;
    } else if (token == "vt") { // Process uvs
      Vector2f vertex;
      ss >> vertex.x >> vertex.y;

      currentMesh->uv.push_back(vertex);
      lastUV++;
    } else if (token == "f") { // Process faces
      ss.ignore(1,' '); // Ignore space after 'f'
      tokens.clear();

      while(std::getline(ss, faceData, ' '))
      {
        tokens.push_back(faceData);
      }

      int points = tokens.size();

      Face face;

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
          if (faces[k] == "") continue;
          if (k == 0) {
            face.positionIndices[i] = std::atoi(faces[0].c_str()) - 1 - currentVertex;
          } else if (k == 1) {
            face.uvIndices[i] = std::atoi(faces[1].c_str()) - 1 - currentUV;
          } else if (k == 2) {
            face.normalIndices[i] = std::atoi(faces[2].c_str()) - 1 - currentNormal;
          }
        }

        // std::cout << "V " << faces[0] << ", " << faces[1] << ", " << faces[2] << std::endl;
      }

      
      currentMesh->faces.push_back(face);
    }
  }

  currentMesh->finish();
  currentGroup->meshes.push_back(currentMesh);

  std::cout << "Meshes count: " << this->object->meshes.size() << std::endl;
  std::cout << "1st mesh faces count: " << this->object->meshes[0]->faces.size() << std::endl; 

  /*
  std::cout << "Meshes count: " << this->object->meshes.size() << std::endl;
  std::cout << "1st mesh faces count: " << this->object->meshes[0]->faces.size() << std::endl;

  std::string exportDir = concatPath(GetDirectory(path), "exported");
  std::string exportModelName = concatPath(exportDir, "model.obj");
  std::cout << "Dirname: " << exportDir << std::endl; 

  if (!folder_exists(exportDir)) {
    mkdir(exportDir.c_str());
  }

  std::fstream fs;
  fs.open ("test.txt", std::fstream::in | std::fstream::out | std::fstream::app);

  fs << " more lorem ipsum";

  fs.close();
  */
  // std::filesystem::create_directories("./out/tv/exported");


  /*
  this->object->traverse([&](MeshObject object){
    std::cout << "Mesh name: " << object->name.c_str() << std::endl;
  });
  */

  // close the file stream
  input.close();


};
