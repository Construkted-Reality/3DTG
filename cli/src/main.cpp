#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "./loaders/ObjLoader.h"
#include "./exporters/ObjExporter.h"
#include "./split/splitter.h"
#include "./utils.h"

#include "./tiles/Tileset.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Input isn't specified" << "\n";
        return 1;
    }

    // std::cout << utils::normalize(".//Chunk_10/bunker36-3txt_10").c_str() << std::endl;
    // std::cout << utils::normalize(".//Chunk_10//bunker36-3txt_10").c_str() << std::endl;
    // std::cout << utils::normalize("//Chunk_10/bunker36-3txt_10").c_str() << std::endl;
    // std::cout << utils::normalize("//Chunk_10//bunker36-3txt_10").c_str() << std::endl;
    // std::cout << utils::normalize("C://Chunk_10/bunker36-3txt_10").c_str() << std::endl;
    // std::cout << utils::normalize("C://Chunk_10//bunker36-3txt_10").c_str() << std::endl;

    std::cout << "Importing..." << std::endl;

    std::string inputFile = utils::normalize(argv[1]);

    ObjLoader loader;
    loader.parse(inputFile.c_str()); 

    std::cout << "Import finished" << std::endl;



    std::string out = "";

    if (argc < 3) { 
        out = utils::concatPath(utils::getDirectory(inputFile), "exported");
    } else {
        out = utils::getDirectory(argv[2]);
    }

    std::cout << "Output directory: " << out.c_str() << std::endl;

    ObjExporter exporter;


    std::cout << "Splitting..." << std::endl;

    splitter::IDGen.reset(); // Reset id counter;
    Tileset tileset(splitter::IDGen.id);

    unsigned int chunk = 0;
    unsigned int lodChunk = 0;
    splitter::splitObject(
        loader.object,
        [&](GroupObject group, unsigned int targetId, unsigned int parentId){
            std::cout << "Base export" << std::endl;

            std::string modelDir = utils::getFileName(group->name) + std::to_string(chunk);
            std::string modelName = utils::getFileName(inputFile) + "_" + std::to_string(chunk);
            std::string modelPath = utils::normalize(
                utils::concatPath(
                    "./",
                    utils::concatPath(modelDir, modelName)
                )
            );

            std::shared_ptr<Tile> parentTile = tileset.findTileById(parentId);
            if (parentTile != NULL) {
                std::shared_ptr<Tile> targetTile = std::make_shared<Tile>();
                targetTile->id = targetId;
                targetTile->geometricError = group->geometricError;

                targetTile->content = std::make_shared<TileContent>();
                targetTile->content->uri = modelPath;

                targetTile->boundingVolume = std::make_shared<TileBoundingVolume>();
                targetTile->boundingVolume->box = std::make_shared<TileBoundingBox>();

                targetTile->boundingVolume->box->center = group->boundingBox.getCenter();
                targetTile->boundingVolume->box->xHalf = group->boundingBox.getSize();
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

            exporter.save(utils::concatPath(out, modelDir), modelName, group);

            group->free();
            chunk++;

            std::cout << "---------------------------------------------" << std::endl;
        },
        [&](GroupObject group, unsigned int targetId, unsigned int parentId){
            std::cout << "Lod export" << std::endl;

            std::string modelDir = utils::getFileName(group->name) + std::to_string(lodChunk);
            std::string modelName = utils::getFileName(inputFile) + "_" + std::to_string(lodChunk);
            std::string modelPath = utils::normalize(
                utils::concatPath(
                    "./",
                    utils::concatPath(modelDir, modelName)
                )
            );
            
            std::shared_ptr<Tile> parentTile = tileset.findTileById(parentId);
            if (parentTile != NULL) {
                std::shared_ptr<Tile> targetTile = std::make_shared<Tile>();
                targetTile->id = targetId;
                targetTile->geometricError = group->geometricError;

                targetTile->content = std::make_shared<TileContent>();
                targetTile->content->uri = modelPath;

                targetTile->boundingVolume = std::make_shared<TileBoundingVolume>();
                targetTile->boundingVolume->box = std::make_shared<TileBoundingBox>();

                targetTile->boundingVolume->box->center = group->boundingBox.getCenter();
                targetTile->boundingVolume->box->xHalf = group->boundingBox.getSize();
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

            exporter.save(utils::concatPath(out, modelDir), modelName, group);

            group->free();
            lodChunk++;

            std::cout << "---------------------------------------------" << std::endl;
        }
    );
    
    std::cout << "Exported" << std::endl;

    std::cout << "Saving JSON" << std::endl;

    std::fstream fs;
    fs.open(utils::concatPath(out, "tileset.json"), std::fstream::out);
    
    fs << tileset.toJSON().dump(2);

    fs.close();
    std::cout << "Saved" << std::endl;

    loader.free();
    
    return 0;
}