#ifndef __OBJLOADER_H__
#define __OBJLOADER_H__

#include <iostream>
#include <fstream>
#include <sstream>
#include <cstdio>


#include "Loader.h"

class ObjLoader : public Loader {
  public:
    void parse(const char* path);
    MaterialMap loadMaterials(const char* path);
};

#endif 