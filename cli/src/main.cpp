#include <iostream>
#include <string>
#include <vector>

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

    ObjLoader loader;
    loader.parse(argv[1]); 

    std::cout << "Import finished" << std::endl << "Splitting..." << std::endl;

    std::vector<GroupObject> splitted = splitter::splitObject(loader.object, 1, 1, 1);

    if (splitted.size() == 0) {
        std::cerr << "Can't split an object" << "\n";
        return 1;
    }

    std::cout << "Splitting finished" << std::endl << "Meshes count after splitting: " << splitted[0]->meshes.size() << std::endl << "Exporting..." << std::endl;

    std::string out = "";

    if (argc < 3) {
        out = utils::concatPath(utils::getDirectory(argv[1]), "exported");
    } else {
        out = argv[2];
    }

    std::cout << "Filename: " << utils::getFileName(argv[1]) << std::endl;

    ObjExporter exporter;

    for (GroupObject &group : splitted) // access by reference to avoid copying
    {  
        exporter.save(out, utils::getFileName(argv[1]) + "_" + group->name, group);
    }
    
    std::cout << "Exported" << std::endl;
    
    return 0;
}