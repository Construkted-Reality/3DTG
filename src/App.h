#ifndef __APP_H__
#define __APP_H__

#include <iostream>
#include <string>
#include <vector>

#include "Options.h"

#include "split/SplitBase.h"
#include "split/RegularSplitter.h"
#include "split/VoxelsSplitter.h"

#include "./loaders/ObjLoader.h"
#include "./exporters/ObjExporter.h"
#include "./exporters/GLTFExporter.h"
#include "./exporters/B3DMExporter.h"

#include "./utils.h"
#include "./tiles/Tileset.h"

class App {
  public:
    static void run();
};

#endif // __APP_H__