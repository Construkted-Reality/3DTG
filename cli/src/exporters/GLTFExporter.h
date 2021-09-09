#ifndef __GLTFEXPORTER_H__
#define __GLTFEXPORTER_H__

#include <GLTF/GLTFAsset.h>
#include <GLTF/GLTFOptions.h>

#include <fstream>
#include <iostream>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

class GLTFExporter
{
  public:
    static const int HEADER_LENGTH = 12;
    static const int CHUNK_HEADER_LENGTH = 8;

    static void exportGLTF(GLTF::Asset *asset, GLTF::Options *options, const char* outputPath);
};


#endif // __GLTFEXPORTER_H__