#include "App.h"

void App::run() {
  Options &opts = Options::GetInstance();

  std::string inputFile = utils::normalize(opts.input);
  std::cout << "Importing " << inputFile.c_str() << std::endl;

  ObjLoader loader;
  loader.parse(inputFile.c_str()); 

  std::cout << "Import finished" << std::endl;

  std::string out = utils::normalize(opts.output);
  std::cout << "Output directory: " << out.c_str() << std::endl;


  GLTFExporter exporter;


  std::cout << "Splitting..." << std::endl;
  

  float totalError = 0.0f;
  Tileset tileset(0);

  std::shared_ptr<SplitInterface> splitInstance = SplitInterface::create(opts.algorithm);

  
  unsigned int chunk = 0;
  unsigned int processed = 0;

  splitInstance->onSave = [&](GroupObject object, IdGenerator::ID targetId, IdGenerator::ID parentId, unsigned int level, bool indexedGeometry){
      object->computeBoundingBox();
      object->computeGeometricError();

      std::string modelDir = std::string("level_") + std::to_string(level);
      std::string modelName = utils::getFileName(object->name) + "_" + std::to_string(chunk);
      std::string modelPath = utils::normalize(
          utils::concatPath("./", utils::concatPath(modelDir, modelName)) + std::string(".") + exporter.format
      );

      std::shared_ptr<Tile> parentTile = tileset.findTileById(parentId);
      if (parentTile != NULL) {
          std::shared_ptr<Tile> targetTile = std::make_shared<Tile>();
          targetTile->id = targetId;
          targetTile->geometricError = object->geometricError;
          //   std::cout << "Geom error: " << targetTile->geometricError << std::endl;
          // targetTile->refine = TileRefine::REPLASE;

          totalError += (float) object->geometricError;

          targetTile->content = std::make_shared<TileContent>();
          targetTile->content->uri = modelPath;

          targetTile->boundingVolume = std::make_shared<TileBoundingVolume>();
          targetTile->boundingVolume->box = std::make_shared<TileBoundingBox>();

          targetTile->boundingVolume->box->center = object->boundingBox.getCenter();
          targetTile->boundingVolume->box->xHalf = object->boundingBox.getSize();
          targetTile->boundingVolume->box->xHalf /= 2.0;

          targetTile->boundingVolume->box->yHalf = targetTile->boundingVolume->box->xHalf;
          targetTile->boundingVolume->box->zHalf = targetTile->boundingVolume->box->xHalf;

          targetTile->boundingVolume->box->xHalf.y = 0.0f;
          targetTile->boundingVolume->box->xHalf.z = 0.0f;

          targetTile->boundingVolume->box->yHalf.x = 0.0f;
          targetTile->boundingVolume->box->yHalf.z = 0.0f;

          targetTile->boundingVolume->box->zHalf.x = 0.0f;
          targetTile->boundingVolume->box->zHalf.y = 0.0f;

          parentTile->children.push_back(targetTile);
      }
      
      exporter.save(utils::concatPath(out, modelDir), modelName, object, indexedGeometry);

      chunk++;

      processed++;
      // std::cout << "Splitting model " << (processed + 1) << std::endl;
  };

  splitInstance->split(loader.object);
  splitInstance->finish();
  
  
  std::cout << "Exported" << std::endl;

  std::cout << "Saving JSON" << std::endl;
  // tileset.computeRootGeometricError();
  tileset.setRootGeometricError(totalError);
  tileset.computeRootBoundingVolume();

  std::fstream fs;
  fs.open(utils::concatPath(out, "tileset.json"), std::fstream::out);
  
  fs << tileset.toJSON().dump(2);

  fs.close();
  std::cout << "Saved" << std::endl;

  loader.free();
};

