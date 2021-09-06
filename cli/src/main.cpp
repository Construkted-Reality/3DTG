#include <iostream>
#include <string>
#include <vector>

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION

#include "./loaders/ObjLoader.h"
#include "./exporters/ObjExporter.h"
#include "./split/splitter.h"
#include "./utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Input isn't specified" << "\n";
        return 1;
    }

    std::cout << "Importing..." << std::endl;

    std::string inputFile = utils::normalize(argv[1]);

    ObjLoader loader;
    loader.parse(inputFile.c_str()); 

    std::cout << "Import finished" << std::endl;

    // std::vector<GroupObject> splitted = splitter::splitObject(loader.object, 2, 2, 1);

    // if (splitted.size() == 0) {
    //     std::cerr << "Can't split an object" << "\n";
    //     return 1;
    // }

    // std::cout << "Splitting finished" << std::endl << "Meshes count after splitting: " << splitted[0]->meshes.size() << std::endl << "Exporting..." << std::endl;

    std::string out = "";

    if (argc < 3) { 
        out = utils::concatPath(utils::getDirectory(inputFile), "exported");
    } else {
        out = utils::getDirectory(argv[2]);
    }

    std::cout << "Output directory: " << out.c_str() << std::endl;

    ObjExporter exporter;


    std::cout << "Splitting..." << std::endl;

    unsigned int chunk = 0;
    unsigned int lodChunk = 0;
    splitter::splitObject(
        loader.object,
        [&](GroupObject group){
            // std::cout << "Group box (" << group->uvBox.min.x << ", " << group->uvBox.min.y << ", " << group->uvBox.min.z << ")";
            // std::cout << " (" << group->uvBox.max.x << ", " << group->uvBox.max.y << ", " << group->uvBox.max.z << ")"<< std::endl;
            std::cout << "Base export" << std::endl;
            exporter.save(utils::concatPath(out, utils::getFileName(group->name)) + std::to_string(chunk), utils::getFileName(inputFile) + "_" + std::to_string(chunk), group);

            // group->free();
            chunk++;

            std::cout << "---------------------------------------------" << std::endl;
        },
        [&](GroupObject group){
            // std::cout << "Group box (" << group->uvBox.min.x << ", " << group->uvBox.min.y << ", " << group->uvBox.min.z << ")";
            // std::cout << " (" << group->uvBox.max.x << ", " << group->uvBox.max.y << ", " << group->uvBox.max.z << ")"<< std::endl;
            std::cout << "Lod export" << std::endl;
            exporter.save(utils::concatPath(out, utils::getFileName(group->name)) + std::to_string(lodChunk), utils::getFileName(inputFile) + "_" + std::to_string(lodChunk), group);

            group->free();
            lodChunk++;

            std::cout << "---------------------------------------------" << std::endl;
        }
    );
    
    std::cout << "Exported" << std::endl;

    loader.free();
    
    return 0;
}