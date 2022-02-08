#include "VoxelsSplitter.h"


const std::string VoxelsSplitter::Type = "voxel";
std::shared_ptr<SplitInterface> VoxelsSplitter::create() {
  Options &opts = Options::GetInstance();

  std::shared_ptr<VoxelsSplitter> inst = std::make_shared<VoxelsSplitter>();
  inst->polygonsLimit = opts.limit;
  inst->gridSettings.gridResolution = glm::ivec3(opts.grid, opts.grid, opts.grid);
  inst->gridSettings.isoLevel = opts.iso;

  return inst;
};

VoxelsSplitter::VoxelsSplitter() {
  //this->grid.init();
};

bool VoxelsSplitter::split(GroupObject target) {
  this->IDGen.reset();

  return this->split(target, this->IDGen.id, 0, true);
};

GroupObject VoxelsSplitter::halfMesh(GroupObject target, bool divideVertical) {
  float xMedian = 0.0f;
  float zMedian = 0.0f;
  unsigned int verticesCount = 0;

  target->traverse([&](MeshObject mesh){
    for (glm::vec3 &position : mesh->position) // access by reference to avoid copying
    {
      if (divideVertical) {
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

  GroupObject base = GroupObject(new Group());
  GroupObject left = GroupObject(new Group());
  GroupObject right = GroupObject(new Group());


  glm::vec3 pos1, pos2, pos3;

  bool isLeft = false;
  bool isRight = false;

  target->traverse([&](MeshObject mesh){
    MeshObject leftMesh = MeshObject(new Mesh());
    MeshObject rightMesh = MeshObject(new Mesh());

    for (Face &face : mesh->faces) // access by reference to avoid copying
    {
      pos1 = mesh->position[face.positionIndices[0]];
      pos2 = mesh->position[face.positionIndices[1]];
      pos3 = mesh->position[face.positionIndices[2]];

      if (divideVertical) {
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
      leftMesh->finish();
      leftMesh->triangulate();

      left->meshes.push_back(leftMesh);
    }

    if (rightMesh->faces.size() != 0) {
      rightMesh->name = mesh->name;
      rightMesh->material = mesh->material;
      rightMesh->hasNormals = mesh->hasNormals;
      rightMesh->hasUVs = mesh->hasUVs;

      rightMesh->remesh(mesh->position, mesh->normal, mesh->uv);
      rightMesh->finish();
      rightMesh->triangulate();

      right->meshes.push_back(rightMesh);
    }
  });

  if (left->meshes.size() != 0) { 
    base->children.push_back(left);
  }

  if (right->meshes.size() != 0) {
    base->children.push_back(right);
  }
  
  return base;
};

GroupObject VoxelsSplitter::decimate(GroupObject target, GridRef grid) {
  GroupObject result = GroupObject(new Group());

  grid->rasterize(target, result);

  // result->computeUVBox();
  result->computeBoundingBox();

  return result;
};


bool VoxelsSplitter::processLod(std::shared_ptr<VoxelSplitTask> task, GridRef grid) {
  // std::cout << "Split started" << std::endl;

  grid->init();
  GroupObject voxelized = this->decimate(task->target, grid);
  utils::graphics::textureLOD(voxelized, 8);
  //targetMesh->material->diffuseMapImage
  // voxelized->traverse([&](MeshObject mesh){
  //   mesh->material = mesh->material->clone(false);// Without texture
  // });


  voxelized->name = "Lod";

  // std::cout << "Calling callback" << std::endl;

  task->callback(voxelized, task->targetId, task->parentID, task->decimationLevel, false);
  voxelized->free(false);

  // std::cout << "Split finished" << std::endl;

  grid->free();

  return false;
};


bool VoxelsSplitter::split(GroupObject target, IdGenerator::ID parentId, unsigned int decimationLevel = 0, bool divideVertical = true) {
  // std::cout << "Split id: " << parentId << std::endl;

  unsigned int polygonCount = 0;

  target->traverse([&](MeshObject mesh){
    polygonCount += mesh->faces.size();
  });

  IdGenerator::ID nextParent = parentId;

  if (polygonCount <= this->polygonsLimit) {
    if (this->onSave) {
      this->IDGen.next();
      nextParent = this->IDGen.id;

      GroupObject resultGroup = utils::graphics::splitUV(target, 0);
      resultGroup->name = "Chunk";
      
      //target->name = "Chunk";
      this->onSave(resultGroup, nextParent, parentId, decimationLevel, false);

      // std::cout << "Clearing the chunk" << std::endl;
      // resultGroup->free();
    }

    return true;
  }

  
  if (this->onSave) {
    this->IDGen.next();
    nextParent = this->IDGen.id;

    
    std::shared_ptr<VoxelSplitTask> task = std::make_shared<VoxelSplitTask>();

    task->target = target;
    task->targetId = nextParent;
    task->parentID = parentId;
    task->decimationLevel = decimationLevel;
    task->callback = this->onSave;

    // std::cout << "Creating a grid" << std::endl;

    GridRef grid = std::make_shared<VoxelGrid>();
    grid->isoLevel = this->gridSettings.isoLevel;
    grid->gridResolution = this->gridSettings.gridResolution;

    // std::cout << "Creating a pool task" << std::endl;
    
    this->pool.create(
      bind(&VoxelsSplitter::processLod, this, std::placeholders::_1, std::placeholders::_2),
      task,
      grid
    );
    
    // std::cout << "Waiting for result" << std::endl;

    this->pool.waitForSlot();
    

    // std::cout << "Processing next task" << std::endl;

    /*
    GroupObject voxelized = this->decimate(target);
    // GroupObject voxelized = splitter::splitUV(this->decimate(target), 8);
    // GroupObject simplified = simplifier::modify(voxelized, 0.5f);
    // simplified->name = "Lod";
    splitter::textureLOD(voxelized, 8);
    voxelized->name = "Lod";

    this->onSave(voxelized, nextParent, parentId, decimationLevel);

    voxelized->free(false);
    */
  }

  // std::cout << "Median split" << std::endl;
  

  GroupObject halfs = this->halfMesh(target, divideVertical);
  // target->free();

  for (GroupObject &half : halfs->children) {
    this->split(half, nextParent, decimationLevel + 1, !divideVertical);
  }

  return true;
};
