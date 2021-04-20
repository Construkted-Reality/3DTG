#ifndef __OBJEXPORTER_H__
#define __OBJEXPORTER_H__

#include "./Exporter.h"


class ObjExporter : public Exporter {
  public:
    void save(std::string directory, std::string fileName, GroupObject object);
    void saveMaterial(std::string directory, std::string fileName, MaterialMap materialMap);
};

#endif