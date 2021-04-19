#include <iostream>
#include <string>

#include "./loaders/ObjLoader.h"
#include "./exporters/ObjExporter.h"
#include "./utils.h"

int main(int argc, char** argv) {
    if (argc < 2) {
        std::cerr << "Input isn't specified" << "\n";

        return 1;
    }

    ObjLoader loader;
    loader.parse(argv[1]); 

    std::string out = "";

    if (argc < 3) {
        out = utils::concatPath(utils::getDirectory(argv[1]), "exported");
    } else {
        out = argv[2];
    }

    ObjExporter exporter;
    exporter.save(out, loader.object);
    
    /*
    std::cout << "Hello World!" << std::endl;
    std::cout << loader.object->name.c_str() << std::endl;

    for (int i = 0; i < argc; ++i)
        std::cout << argv[i] << "\n";
    */
    
    return 0;
}