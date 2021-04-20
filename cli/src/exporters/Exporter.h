#ifndef __EXPORTER_H__
#define __EXPORTER_H__

#include <string>
#include <fstream>
#include <iostream>

#include "./../utils.h"
#include "./../loaders/Loader.h"

class Exporter {
  public:
    virtual void save(std::string directory, std::string fileName, GroupObject object);
};

#endif