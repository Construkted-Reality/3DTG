#ifndef __GLTFEXPORTER_H__
#define __GLTFEXPORTER_H__

#include <GLTF/GLTFOptions.h>
#include <GLTF/GLTFScene.h>
#include <GLTF/GLTFMesh.h>

#include <GLTF/GLTFMaterial.h>
#include <GLTF/GLTFTexture.h>
#include <GLTF/GLTFSampler.h>
#include <GLTF/GLTFImage.h>

#include <GLTF/GLTFPrimitive.h>
#include <GLTF/GLTFNode.h>
#include <GLTF/GLTFBuffer.h>
#include <GLTF/GLTFBufferView.h>
#include <GLTF/GLTFAccessor.h>
#include <GLTF/GLTFAsset.h>

#include <fstream>
#include <iostream>
#include <sstream>
#include <string>
#include <map>
#include <algorithm>
#include <functional>

#include <rapidjson/document.h>
#include <rapidjson/prettywriter.h>
#include <rapidjson/stringbuffer.h>
#include <rapidjson/writer.h>

#include "./Exporter.h"

namespace GLTF_BUFFER {
  static const std::string POSITION_BUFFER = "VERTEX";
  static const std::string NORMAL_BUFFER = "NORMAL";
  static const std::string UV_BUFFER = "UV";
  static const std::string INDEX_POSITION_BUFFER = "INDEX_POSITION";
  static const std::string INDEX_NORMAL_BUFFER = "INDEX_NORMAL";
  static const std::string INDEX_UV_BUFFER = "INDEX_UV";
};

class GLTFExporter : public Exporter {
  public:
    static const int HEADER_LENGTH = 12;
    static const int CHUNK_HEADER_LENGTH = 8;

    static void updateAccessorFace(std::map<std::string, GLTF::Accessor*> &accessors, MeshObject &mesh, Face &face);
    static void updateAccessorIndex(GLTF::Accessor* accessor, std::vector<unsigned int> &data);

    static void updateAccessorMin3f(GLTF::Accessor* accessor, glm::vec3 &vec);
    static void updateAccessorMax3f(GLTF::Accessor* accessor, glm::vec3 &vec);

    static void updateAccessorMin2f(GLTF::Accessor* accessor, glm::vec2 &vec);
    static void updateAccessorMax2f(GLTF::Accessor* accessor, glm::vec2 &vec);

    void exportGLTF(GLTF::Asset *asset, GLTF::Options *options, const char* outputPath);
    void save(std::string directory, std::string fileName, GroupObject object, bool indexedGeometry);
    
    std::function<void(FILE* file, size_t binarySize)> beforeBinWrite;
    struct ImageData
    {
      std::stringstream data = std::stringstream(std::stringstream::binary | std::stringstream::in | std::stringstream::out);
      size_t size = 0;
    };

    std::string format = "glb";

    static const std::string Type;
    static std::shared_ptr<Exporter> create();
};


#endif // __GLTFEXPORTER_H__