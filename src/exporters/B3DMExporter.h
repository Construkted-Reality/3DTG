#ifndef __B3DMEXPORTER_H__
#define __B3DMEXPORTER_H__

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include <json/json.hpp>

#include "./Exporter.h"
#include "./GLTFExporter.h"

class B3DMExporter : public Exporter {
  public:
    void save(std::string directory, std::string fileName, GroupObject object, bool indexedGeometry);
    std::string format = "b3dm";
};


#endif // __B3DMEXPORTER_H__