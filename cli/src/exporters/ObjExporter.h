#ifndef __OBJEXPORTER_H__
#define __OBJEXPORTER_H__

#include "./Exporter.h"


class ObjExporter : public Exporter {
  public:
    void save(std::string dir, GroupObject object);
};

#endif