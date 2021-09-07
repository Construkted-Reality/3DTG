#ifndef __TILEASSET_H__
#define __TILEASSET_H__

#include <string>

struct TileAsset {
  std::string version = "1.0.0";// Required
  std::string tilesetVersion;
};

#endif // __TILEASSET_H__