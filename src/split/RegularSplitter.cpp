#include "./RegularSplitter.h"


const std::string RegularSplitter::Type = "regular";
std::shared_ptr<SplitInterface> RegularSplitter::create() {
  Options &opts = Options::GetInstance();

  std::shared_ptr<RegularSplitter> inst = std::make_shared<RegularSplitter>();
  inst->polygonLimit = opts.limit;

  return inst;
};

bool RegularSplitter::processLod(std::shared_ptr<RegularSplitTask> task) {
  // std::cout << "Split started" << std::endl;

  GroupObject resultGroup = utils::graphics::splitUV(task->target, task->uvModifier);
  resultGroup->name = std::string("Lod");

  GroupObject modified = simplifier::modify(resultGroup, 500.0f);
  // this->onSave(simplifier::modify(resultGroup, 500.0f), this->IDGen.id, parent, splitLevel);
  // std::cout << "Calling callback" << std::endl;
  // modified->traverse([&](MeshObject mesh){
  //   mesh->triangulate();
  // });
  
  task->callback(modified, task->targetId, task->parentID, task->decimationLevel, true);

  // resultGroup->free(false);
  modified->free();

  // std::cout << "Split finished" << std::endl;

  return false;
};


bool RegularSplitter::splitObject(GroupObject baseObject, unsigned int polygonLimit, unsigned int splitLevel, IdGenerator::ID parent, bool isVertical = false) {
  unsigned int polygonCount = 0;

  baseObject->traverse([&](MeshObject mesh){
    polygonCount += mesh->faces.size();
  });

  // std::cout << "Simplifying: " << polygonCount << " polygons" << std::endl;

  // float polyModifier = polygonLimit / polygonCount;// 2048 / 48000

  IdGenerator::ID nextParent = parent;

  if (polygonCount <= polygonLimit) {
    this->IDGen.next();
    nextParent = this->IDGen.id;

    GroupObject resultGroup = utils::graphics::splitUV(baseObject);
    resultGroup->name = "Chunk";

    resultGroup->traverse([&](MeshObject mesh){
      mesh->triangulate();
    });

    this->onSave(resultGroup, this->IDGen.id, parent, splitLevel, false);

    resultGroup->free();

    return false;
  } else {
    this->IDGen.next();
    nextParent = this->IDGen.id;

    float polyModifier = (polygonCount / polygonLimit) * 0.1;
    int uvModifier = std::max(std::min(32, (int) std::ceil(polyModifier)), 1);

    // GroupObject resultGroup = splitUV(baseObject, uvModifier);// baseObject->clone();//

    // // float simplifyLevel = 1.0f - (1.0f / float(i));
    // resultGroup->name = std::string("Lod");// + std::to_string(i - 2) + std::string("_Chunk_");
    // //simplifier::modify(resultGroup, 50.0f)
    // this->onSave(simplifier::modify(resultGroup, 500.0f), this->IDGen.id, parent, splitLevel);// (polygonCount / polygonLimit)


    std::shared_ptr<RegularSplitTask> task = std::make_shared<RegularSplitTask>();

    task->target = baseObject;
    task->targetId = nextParent;
    task->parentID = parent;
    task->decimationLevel = splitLevel;
    task->uvModifier = uvModifier;
    task->callback = this->onSave;

    // std::cout << "Creating a pool task" << std::endl;
    
    this->pool.create(
      bind(&RegularSplitter::processLod, this, std::placeholders::_1),
      task
    );
    
    // std::cout << "Waiting for result" << std::endl;

    this->pool.waitForSlot();

    /*
    for (unsigned int i = 2; i < 7; i++) {// 5 Levels
      if (polygonCount <= polygonLimit * i) {
        splitter::IDGen.next();
        nextParent = splitter::IDGen.id;

        GroupObject resultGroup = splitter::splitUV(baseObject, i - 1);

        // float simplifyLevel = 1.0f - (1.0f / float(i));
        resultGroup->name = std::string("Lod");// + std::to_string(i - 2) + std::string("_Chunk_");
 
        lodFn(simplifier::modify(resultGroup, 0.5f), splitter::IDGen.id, parent, i - 1);

        break;

        //return false;
      }
    }*/

    // baseObject->name = "Lod_0_Chunk_";
    //lodFn(simplifier::modify(splitter::splitUV(baseObject), 0.5f));//splitter::splitUV()
  }

  float xMedian = 0.0f;
  float zMedian = 0.0f;
  unsigned int verticesCount = 0;

  baseObject->traverse([&](MeshObject mesh){
    for (Vector3f &position : mesh->position) // access by reference to avoid copying
    {
      if (isVertical) {
        xMedian += position.x;
      } else {
        zMedian += position.z;
      }
    }

    verticesCount += mesh->position.size();
  });

  if (verticesCount == 0) {
    verticesCount = 1;
  }

  xMedian /= verticesCount;
  zMedian /= verticesCount;

  GroupObject left = GroupObject(new Group());
  GroupObject right = GroupObject(new Group());


  Vector3f pos1, pos2, pos3;
  // unsigned int leftFaces = 0;
  // unsigned int rightFaces = 0;

  bool isLeft = false;
  bool isRight = false;

  baseObject->traverse([&](MeshObject mesh){
    MeshObject leftMesh = MeshObject(new Mesh());
    MeshObject rightMesh = MeshObject(new Mesh());

    for (Face &face : mesh->faces) // access by reference to avoid copying
    {
      pos1 = mesh->position[face.positionIndices[0]];
      pos2 = mesh->position[face.positionIndices[1]];
      pos3 = mesh->position[face.positionIndices[2]];

      if (isVertical) {
        isLeft = pos1.x <= xMedian || pos2.x <= xMedian || pos3.x <= xMedian;
        isRight = pos1.x >= xMedian || pos2.x >= xMedian || pos3.x >= xMedian;

        if (isLeft) {
          leftMesh->faces.push_back(face);
        }
        if (isRight) {
          rightMesh->faces.push_back(face);
        }
      } else {
        isLeft = pos1.z <= zMedian || pos2.z <= zMedian || pos3.z <= zMedian;
        isRight = pos1.z >= zMedian || pos2.z >= zMedian || pos3.z >= zMedian;

        if (isLeft) {
          leftMesh->faces.push_back(face);
        }
        if (isRight) {
          rightMesh->faces.push_back(face);
        }
      }
    }

    if (leftMesh->faces.size() != 0) {
      leftMesh->name = mesh->name;
      leftMesh->material = mesh->material;
      leftMesh->hasNormals = mesh->hasNormals;
      leftMesh->hasUVs = mesh->hasUVs;

      leftMesh->remesh(mesh->position, mesh->normal, mesh->uv);

      left->meshes.push_back(leftMesh);
    }

    if (rightMesh->faces.size() != 0) {
      rightMesh->name = mesh->name;
      rightMesh->material = mesh->material;
      rightMesh->hasNormals = mesh->hasNormals;
      rightMesh->hasUVs = mesh->hasUVs;

      rightMesh->remesh(mesh->position, mesh->normal, mesh->uv);

      right->meshes.push_back(rightMesh);
    }
  });

  if (left->meshes.size() != 0) { 
    this->straightLine(left, isVertical, true, xMedian, zMedian);
    this->splitObject(left, polygonLimit, splitLevel + 1, nextParent, !isVertical);
  }

  if (right->meshes.size() != 0) {
    this->straightLine(right, isVertical, false, xMedian, zMedian);
    this->splitObject(right, polygonLimit, splitLevel + 1, nextParent, !isVertical);
  }

  return true;
};


// TODO optimize a lot
void RegularSplitter::straightLineX(MeshObject &mesh, Face &face, bool isLeft, float xValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces) {
  Vector3f pos1, pos2, pos3;

  pos1 = mesh->position[face.positionIndices[0]];
  pos2 = mesh->position[face.positionIndices[1]];
  pos3 = mesh->position[face.positionIndices[2]];

  int intersectionCount = 0;
  
  if (isLeft) {
    intersectionCount = int(pos1.x <= xValue) + int(pos2.x <= xValue) + int(pos3.x <= xValue);
  } else {
    intersectionCount = int(pos1.x >= xValue) + int(pos2.x >= xValue) + int(pos3.x >= xValue);
  }

  if (intersectionCount != 3) {
    bool aInside = true;
    bool bInside = true;
    bool cInside = true;

    if (isLeft) {
      if (pos1.x > xValue) aInside = false;
      if (pos2.x > xValue) bInside = false;
      if (pos3.x > xValue) cInside = false;
    } else {
      if (pos1.x < xValue) aInside = false;
      if (pos2.x < xValue) bInside = false;
      if (pos3.x < xValue) cInside = false;
    }

    if (intersectionCount == 1) {
      if (aInside) {
        mesh->position[face.positionIndices[1]].lerpToX(mesh->position[face.positionIndices[1]], mesh->position[face.positionIndices[0]], xValue);
        mesh->position[face.positionIndices[2]].lerpToX(mesh->position[face.positionIndices[2]], mesh->position[face.positionIndices[0]], xValue);
      } else if (bInside) {
        mesh->position[face.positionIndices[0]].lerpToX(mesh->position[face.positionIndices[0]], mesh->position[face.positionIndices[1]], xValue);
        mesh->position[face.positionIndices[2]].lerpToX(mesh->position[face.positionIndices[2]], mesh->position[face.positionIndices[1]], xValue);
      } else if (cInside) {
        mesh->position[face.positionIndices[0]].lerpToX(mesh->position[face.positionIndices[0]], mesh->position[face.positionIndices[2]], xValue);
        mesh->position[face.positionIndices[1]].lerpToX(mesh->position[face.positionIndices[1]], mesh->position[face.positionIndices[2]], xValue);
      }
    } else if (intersectionCount == 2) {
      float deltaA = 0.0f;
      float deltaB = 0.0f;

      Vector3f pos4;
      Face nextFace;

      if (aInside && bInside) {
        //std::cout << "Construct new vertex from C" << std::endl;

        deltaA = Vector3f::deltaX(pos3, pos1, xValue);
        deltaB = Vector3f::deltaX(pos3, pos2, xValue);

        pos4.lerp(pos3, pos2, deltaB);
        mesh->position[face.positionIndices[2]].lerp(pos3, pos1, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[1];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[2];

        nextFace.normalIndices[0] = face.normalIndices[1];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[2];

        nextFace.uvIndices[0] = face.uvIndices[1];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[2];

        faces.push_back(nextFace);
      } else if (aInside && cInside) {
        //std::cout << "Construct new vertex from B" << std::endl;

        deltaA = Vector3f::deltaX(pos2, pos3, xValue);
        deltaB = Vector3f::deltaX(pos2, pos1, xValue);

        pos4.lerp(pos2, pos1, deltaB);
        mesh->position[face.positionIndices[1]].lerp(pos2, pos3, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[0];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[1];

        nextFace.normalIndices[0] = face.normalIndices[0];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[1];

        nextFace.uvIndices[0] = face.uvIndices[0];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[1];

        faces.push_back(nextFace);
      } else if (bInside && cInside) {
        //std::cout << "Construct new vertex from B" << std::endl;

        deltaA = Vector3f::deltaX(pos1, pos3, xValue);
        deltaB = Vector3f::deltaX(pos1, pos2, xValue);

        pos4.lerp(pos1, pos2, deltaB);
        mesh->position[face.positionIndices[0]].lerp(pos1, pos3, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[1];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[0];

        nextFace.normalIndices[0] = face.normalIndices[1];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[0];

        nextFace.uvIndices[0] = face.uvIndices[1];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[0];

        faces.push_back(nextFace);
      }
    }
  }
};

// TODO optimize a lot
void RegularSplitter::straightLineZ(MeshObject &mesh, Face &face, bool isLeft, float zValue, std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv, std::vector<Face> &faces) {
  Vector3f pos1, pos2, pos3;

  pos1 = mesh->position[face.positionIndices[0]];
  pos2 = mesh->position[face.positionIndices[1]];
  pos3 = mesh->position[face.positionIndices[2]];

  int intersectionCount = 0;
  
  if (isLeft) {
    intersectionCount = int(pos1.z <= zValue) + int(pos2.z <= zValue) + int(pos3.z <= zValue);
  } else {
    intersectionCount = int(pos1.z >= zValue) + int(pos2.z >= zValue) + int(pos3.z >= zValue);
  }

  if (intersectionCount != 3) {
    bool aInside = true;
    bool bInside = true;
    bool cInside = true;

    if (isLeft) {
      if (pos1.z > zValue) aInside = false;
      if (pos2.z > zValue) bInside = false;
      if (pos3.z > zValue) cInside = false;
    } else {  
      if (pos1.z < zValue) aInside = false;
      if (pos2.z < zValue) bInside = false;
      if (pos3.z < zValue) cInside = false;
    }

    if (intersectionCount == 1) {
      if (aInside) {
        mesh->position[face.positionIndices[1]].lerpToZ(mesh->position[face.positionIndices[1]], mesh->position[face.positionIndices[0]], zValue);
        mesh->position[face.positionIndices[2]].lerpToZ(mesh->position[face.positionIndices[2]], mesh->position[face.positionIndices[0]], zValue);
      } else if (bInside) {
        mesh->position[face.positionIndices[0]].lerpToZ(mesh->position[face.positionIndices[0]], mesh->position[face.positionIndices[1]], zValue);
        mesh->position[face.positionIndices[2]].lerpToZ(mesh->position[face.positionIndices[2]], mesh->position[face.positionIndices[1]], zValue);
      } else if (cInside) {
        mesh->position[face.positionIndices[0]].lerpToZ(mesh->position[face.positionIndices[0]], mesh->position[face.positionIndices[2]], zValue);
        mesh->position[face.positionIndices[1]].lerpToZ(mesh->position[face.positionIndices[1]], mesh->position[face.positionIndices[2]], zValue);
      }
    } else if (intersectionCount == 2) {
      float deltaA = 0.0f;
      float deltaB = 0.0f;

      Vector3f pos4;
      Face nextFace;

      if (aInside && bInside) {
        //std::cout << "Construct new vertex from C" << std::endl;

        deltaA = Vector3f::deltaZ(pos3, pos1, zValue);
        deltaB = Vector3f::deltaZ(pos3, pos2, zValue);

        pos4.lerp(pos3, pos2, deltaB);
        mesh->position[face.positionIndices[2]].lerp(pos3, pos1, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[1];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[2];

        nextFace.normalIndices[0] = face.normalIndices[1];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[2];

        nextFace.uvIndices[0] = face.uvIndices[1];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[2];

        faces.push_back(nextFace);
      } else if (aInside && cInside) {
        //std::cout << "Construct new vertex from B" << std::endl;

        deltaA = Vector3f::deltaZ(pos2, pos3, zValue);
        deltaB = Vector3f::deltaZ(pos2, pos1, zValue);

        pos4.lerp(pos2, pos1, deltaB);
        mesh->position[face.positionIndices[1]].lerp(pos2, pos3, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[0];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[1];

        nextFace.normalIndices[0] = face.normalIndices[0];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[1];

        nextFace.uvIndices[0] = face.uvIndices[0];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[1];

        faces.push_back(nextFace);
      } else if (bInside && cInside) {
        //std::cout << "Construct new vertex from B" << std::endl;

        deltaA = Vector3f::deltaZ(pos1, pos3, zValue);
        deltaB = Vector3f::deltaZ(pos1, pos2, zValue);

        pos4.lerp(pos1, pos2, deltaB);
        mesh->position[face.positionIndices[0]].lerp(pos1, pos3, deltaA);

        position.push_back(pos4);
        if (mesh->hasNormals) {
          normal.push_back(mesh->normal[face.normalIndices[2]]);
        }
        if (mesh->hasUVs) {
          uv.push_back(mesh->uv[face.uvIndices[2]]);
        }

        nextFace.positionIndices[0] = face.positionIndices[1];
        nextFace.positionIndices[1] = mesh->position.size() + position.size() - 1;
        nextFace.positionIndices[2] = face.positionIndices[0];

        nextFace.normalIndices[0] = face.normalIndices[1];
        nextFace.normalIndices[1] = mesh->normal.size() + normal.size() - 1;
        nextFace.normalIndices[2] = face.normalIndices[0];

        nextFace.uvIndices[0] = face.uvIndices[1];
        nextFace.uvIndices[1] = mesh->uv.size() + uv.size() - 1;
        nextFace.uvIndices[2] = face.uvIndices[0];

        faces.push_back(nextFace);
      }
    }
  }
};

void RegularSplitter::straightLine(GroupObject &baseObject, bool isVertical, bool isLeft, float xValue, float zValue) {
  std::vector<Vector3f> position;
  std::vector<Vector3f> normal;
  std::vector<Vector2f> uv;

  std::vector<Face> faces;

  baseObject->traverse([&](MeshObject mesh){
    position.clear();
    normal.clear();
    uv.clear();
    faces.clear();

    for (Face &face : mesh->faces) // access by reference to avoid copying
    {
      if (isVertical) {
        this->straightLineX(mesh, face, isLeft, xValue, position, normal, uv, faces);
      } else {
        this->straightLineZ(mesh, face, isLeft, zValue, position, normal, uv, faces);
      }
    }

    if (position.size() != 0) {
      std::copy(position.begin(), position.end(), std::back_inserter(mesh->position));
      std::copy(normal.begin(), normal.end(), std::back_inserter(mesh->normal));
      std::copy(uv.begin(), uv.end(), std::back_inserter(mesh->uv));
      std::copy(faces.begin(), faces.end(), std::back_inserter(mesh->faces));

      mesh->finish();
    }
  });
};

bool RegularSplitter::split(GroupObject baseObject) {
  // splitter::IDGen.reset();
  this->IDGen.reset();

  this->splitObject(baseObject, this->polygonLimit, 0, this->IDGen.id, true);

  return true;
};