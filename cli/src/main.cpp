#include <cxxopts/cxxopts.hpp>

#include <iostream>
#include <string>
#include <vector>

#include "./loaders/ObjLoader.h"
#include "./exporters/ObjExporter.h"
#include "./exporters/GLTFExporter.h"
#include "./exporters/B3DMExporter.h"
#include "./split/splitter.h"
#include "./split/VoxelsSplitter.h"
#include "./utils.h"

#include "./tiles/Tileset.h"

#include <GLTF/GLTFScene.h>
#include <GLTF/GLTFMesh.h>
#include <GLTF/GLTFMaterial.h>
#include <GLTF/GLTFPrimitive.h>
#include <GLTF/GLTFNode.h>
#include <GLTF/GLTFBuffer.h>
#include <GLTF/GLTFBufferView.h>
#include <GLTF/GLTFAccessor.h>
#include <GLTF/GLTFAsset.h>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize.h>


int main(int argc, char** argv) {
    /*
    unsigned char indexData[8] = {
        // 6 bytes of indices and two bytes of padding
        0x00,0x00,0x01,0x00,0x02,0x00,0x00,0x00
    };

    unsigned char posData[36] = {
        // 36 bytes of floating point numbers
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
        0x00,0x00,0x00,0x00,0x00,0x00,0x00,0x00,
        0x00,0x00,0x00,0x00,0x00,0x00,0x80,0x3f,
        0x00,0x00,0x00,0x00
    };

    GLTF::Buffer *indexBuffer = new GLTF::Buffer(indexData, 8);
    GLTF::Buffer *dataBuffer = new GLTF::Buffer(posData, 36);

    GLTF::BufferView *view1 = new GLTF::BufferView(0, 8, indexBuffer);
    view1->target = GLTF::Constants::WebGL::ELEMENT_ARRAY_BUFFER;

    GLTF::BufferView *view2 = new GLTF::BufferView(0, 36, dataBuffer);
    view2->target = GLTF::Constants::WebGL::ARRAY_BUFFER;

    GLTF::Accessor *accessor1 = new GLTF::Accessor(
        GLTF::Accessor::Type::SCALAR,
        GLTF::Constants::WebGL::UNSIGNED_SHORT,
        0, 3, view1
    );

    float accessor1Min[1] = {0.0f};
    float accessor1Max[1] = {2.0f};

    accessor1->min = accessor1Min;
    accessor1->max = accessor1Max;

    GLTF::Accessor *accessor2 = new GLTF::Accessor(
        GLTF::Accessor::Type::VEC3,
        GLTF::Constants::WebGL::FLOAT,
        0, 3, view2
    );

    float accessor2Min[3] = {0.0f, 0.0f, 0.0f};
    float accessor2Max[3] = {1.0f, 1.0f, 0.0f};

    accessor2->min = accessor2Min;
    accessor2->max = accessor2Max;

    // MATERIAL START
    GLTF::MaterialCommon *material = new GLTF::MaterialCommon();
    material->type = GLTF::Material::Type::MATERIAL_COMMON;
    material->technique = GLTF::MaterialCommon::Technique::CONSTANT;
    material->doubleSided = true;

    GLTF::Material::Values *values = new GLTF::Material::Values();

    float diffuse[4] = {0.0f, 0.5f, 1.0f, 1.0f};
    values->diffuse = diffuse;

    material->values = values;
    // MATERIAL END

    GLTF::Primitive *primitive = new GLTF::Primitive();
    primitive->indices = accessor1;
    primitive->attributes["POSITION"] = accessor2;
    primitive->mode = GLTF::Primitive::Mode::TRIANGLES;
    primitive->material = material;


    GLTF::Mesh *mesh = new GLTF::Mesh();
    mesh->primitives.push_back(primitive);

    GLTF::Node *node = new GLTF::Node();
    node->mesh = mesh;

    GLTF::Scene *scene = new GLTF::Scene();
    scene->nodes.push_back(node);


    GLTF::Asset::Metadata meta;
    meta.generator = "3DTG";

    GLTF::Asset *asset = new GLTF::Asset();
    asset->metadata = &meta;
    asset->scenes.push_back(scene);
    asset->scene = 0;


    std::cout << "GLTF successfuly generated!" << std::endl;

    GLTF::Options gltfOptions;
    gltfOptions.binary = true;
    gltfOptions.dracoCompression = true;

    std::cout << "GLTF exporting..." << std::endl;

    // GLTFExporter gltfExporter;
    GLTFExporter::exportGLTF(asset, &gltfOptions, utils::normalize("./out/model.glb").c_str());

    std::cout << "GLTF successfuly exported!" << std::endl;

    delete indexBuffer;
    delete dataBuffer;

    delete view1;
    delete view2;

    delete accessor1;
    delete accessor2;

    delete material;

    delete primitive;
    delete mesh;
    delete node;
    delete scene;

    delete asset;


    return 0;
    */

    cxxopts::Options options("3dTG", "3d models compiler from various formats to tile format.");

    options.add_options()
    ("i,input", "Input model path", cxxopts::value<std::string>())
    ("o,output", "Output directory", cxxopts::value<std::string>()->default_value("./exported"))
    ("l,limit", "Polygons per chunk limit", cxxopts::value<unsigned int>()->default_value("2048"))
    ("iso", "Iso level", cxxopts::value<float>()->default_value("1.0"))
    ("f,format", "Model format to export", cxxopts::value<std::string>()->default_value("b3dm"))
    ("h,help", "Help")
    ;

    options.allow_unrecognised_options();

    cxxopts::ParseResult result;

    try {
        result = options.parse(argc, argv);
    } catch (cxxopts::missing_argument_exception const &exeption) {
        std::cout << exeption.what() << std::endl;
        return 0;
    }

    if (result.count("help")) {
        std::cout << options.help() << std::endl;
        exit(0);
    }

    if (!result.count("input")) {
        std::cout << "Input isn't specified" << std::endl;
        exit(0);
    }

    std::string inputFile = utils::normalize(result["input"].as<std::string>());
    std::cout << "Importing " << inputFile.c_str() << std::endl;

    ObjLoader loader;
    loader.parse(inputFile.c_str()); 

    std::cout << "Import finished" << std::endl;



    std::string out = utils::normalize(result["output"].as<std::string>());

    /*
    if (argc < 3) { 
        out = utils::concatPath(utils::getDirectory(inputFile), "exported");
    } else {
        out = utils::getDirectory(argv[2]);
    }
    */

    std::cout << "Output directory: " << out.c_str() << std::endl;

    GLTFExporter exporter;

 
    std::cout << "Splitting..." << std::endl;

    unsigned int chunk = 0;
    unsigned int processed = 0;
    float totalError = 0.0f;

    Tileset tileset(0);

    VoxelsSplitter currentSplitter;
    currentSplitter.polygonsLimit = result["limit"].as<unsigned int>();
    currentSplitter.grid.isoLevel = result["iso"].as<float>();

    currentSplitter.onSave = [&](GroupObject object, IdGenerator::ID targetId, IdGenerator::ID parentId, unsigned int level){
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
        
        exporter.save(utils::concatPath(out, modelDir), modelName, object);

        chunk++;

        processed++;
        std::cout << "Splitting model " << (processed + 1) << std::endl;
    };

    currentSplitter.split(loader.object);


    /*
    splitter::IDGen.reset(); // Reset id counter;

    Tileset tileset(splitter::IDGen.id);

    unsigned int chunk = 0;
    unsigned int lodChunk = 0;

    unsigned int processed = 0;

    float totalError = 0.0f;

    std::cout << "Splitting model " << (processed + 1) << std::endl;// " of " << 60 << std::endl;  

    splitter::splitObject(
        loader.object,
        result["limit"].as<unsigned int>(),
        [&](GroupObject group, unsigned int targetId, unsigned int parentId, unsigned int level){
            group->computeBoundingBox();
            group->computeGeometricError();

            // std::cout << "Geometric error: " << group->geometricError << std::endl;

            
            // std::string modelDir = utils::getFileName(group->name) + std::to_string(chunk);
            // std::string modelName = utils::getFileName(inputFile) + "_" + std::to_string(chunk);
            // std::string modelPath = utils::normalize(
            //     utils::concatPath("./", utils::concatPath(modelDir, modelName)) + std::string(".") + exporter.format
            // );
            

            std::string modelDir = std::string("level_") + std::to_string(level);
            std::string modelName = utils::getFileName(group->name) + "_" + std::to_string(chunk);
            std::string modelPath = utils::normalize(
                utils::concatPath("./", utils::concatPath(modelDir, modelName)) + std::string(".") + exporter.format
            );

            std::shared_ptr<Tile> parentTile = tileset.findTileById(parentId);
            if (parentTile != NULL) {
                std::shared_ptr<Tile> targetTile = std::make_shared<Tile>();
                targetTile->id = targetId;
                targetTile->geometricError = group->geometricError;
                // targetTile->refine = TileRefine::REPLASE;

                totalError += (float) group->geometricError;

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

            // group->free();
            chunk++;

            processed++;
            // if (processed < 60) {
            //     std::cout << "Splitting model " << (processed + 1) << " of " << 60 << std::endl;
            // }

            std::cout << "Splitting model " << (processed + 1) << std::endl;

            // std::cout << "---------------------------------------------" << std::endl;
        },
        [&](GroupObject group, unsigned int targetId, unsigned int parentId, unsigned int level){
            group->computeBoundingBox();
            group->computeGeometricError();

            // std::cout << "Geometric error: " << group->geometricError << std::endl;

            
            // std::string modelDir = utils::getFileName(group->name) + std::to_string(chunk);
            // std::string modelName = utils::getFileName(inputFile) + "_" + std::to_string(lodChunk);
            // std::string modelPath = utils::normalize(
            //     utils::concatPath("./", utils::concatPath(modelDir, modelName)) + std::string(".") + exporter.format
            // );
            

            std::string modelDir = std::string("level_") + std::to_string(level);
            std::string modelName = utils::getFileName(group->name) + "_" + std::to_string(lodChunk);
            std::string modelPath = utils::normalize(
                utils::concatPath("./", utils::concatPath(modelDir, modelName)) + std::string(".") + exporter.format
            );
            
            std::shared_ptr<Tile> parentTile = tileset.findTileById(parentId);
            if (parentTile != NULL) {
                std::shared_ptr<Tile> targetTile = std::make_shared<Tile>();
                targetTile->id = targetId;
                targetTile->geometricError = group->geometricError;
                // targetTile->refine = TileRefine::REPLASE;

                totalError += (float) group->geometricError;

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

            // group->free();
            lodChunk++;

            // std::cout << "---------------------------------------------" << std::endl;
        }
    );
    */
    
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
    
    return 0;
}