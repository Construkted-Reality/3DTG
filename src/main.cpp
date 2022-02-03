#include "Options.h"

#include "split/SplitBase.h"
#include "split/RegularSplitter.h"
#include "split/VoxelsSplitter.h"

#include "./exporters/Exporter.h"
#include "./exporters/ObjExporter.h"
#include "./exporters/GLTFExporter.h"
#include "./exporters/B3DMExporter.h"

#include "App.h"

#define STB_IMAGE_IMPLEMENTATION
#define STB_IMAGE_WRITE_IMPLEMENTATION
#define STB_IMAGE_RESIZE_IMPLEMENTATION

#include <stb/stb_image.h>
#include <stb/stb_image_write.h>
#include <stb/stb_image_resize.h>


int main(int argc, char** argv) {
  Exporter::addCreator(ObjExporter::Type, ObjExporter::create);
  Exporter::addCreator(GLTFExporter::Type, GLTFExporter::create);
  Exporter::addCreator(B3DMExporter::Type, B3DMExporter::create);

  SplitInterface::addCreator(RegularSplitter::Type, RegularSplitter::create);
  SplitInterface::addCreator(VoxelsSplitter::Type, VoxelsSplitter::create);

  Options &opts = Options::GetInstance();

  opts.addFormat(ObjExporter::Type);
  opts.addFormat(GLTFExporter::Type);
  opts.addFormat(B3DMExporter::Type);

  opts.setDefaultFormat(B3DMExporter::Type);
  

  opts.addAlgorithm(RegularSplitter::Type);
  opts.addAlgorithm(VoxelsSplitter::Type);

  opts.setDefaultAlgorithm(VoxelsSplitter::Type);

  if (!opts.valid(argc, argv)) {
    return 0;
  }

  App::run();
  
  return 0;
}