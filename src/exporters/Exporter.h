#ifndef __EXPORTER_H__
#define __EXPORTER_H__

#include <string>
#include <fstream>
#include <iostream>
#include <memory>

#include <stb/stb_image_write.h>

#include "./../utils.h"
#include "./../loaders/Loader.h"

class Exporter {
  public:
    virtual void save(std::string directory, std::string fileName, GroupObject object, bool indexedGeometry) = 0;

    typedef std::function<std::shared_ptr<Exporter>()> ExporterCreator;
    static std::map<std::string, ExporterCreator> creatorList;

    static std::shared_ptr<Exporter> create(std::string type);
    static void addCreator(const std::string type, ExporterCreator creator);
};

#endif