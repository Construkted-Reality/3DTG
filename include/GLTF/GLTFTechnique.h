// Copyright 2020 The Khronos® Group Inc.
#pragma once

#include <map>
#include <set>
#include <string>
#include <vector>

#include "GLTFObject.h"
#include "GLTFProgram.h"

namespace GLTF {
class Technique : public GLTF::Object {
 public:
  class Parameter {
   public:
    std::string semantic;
    GLTF::Constants::WebGL type;
    float* value = NULL;
    int node = -1;
    std::string nodeString;
    int count = -1;
    int valueLength;

    explicit Parameter(GLTF::Constants::WebGL type) : type(type) {}
    Parameter(GLTF::Constants::WebGL type, float* value, int valueLength)
        : type(type), value(value), valueLength(valueLength) {}
    Parameter(std::string semantic, GLTF::Constants::WebGL type)
        : semantic(semantic), type(type) {}
    Parameter(std::string semantic, GLTF::Constants::WebGL type, int count)
        : semantic(semantic), type(type), count(count) {}
  };

  ~Technique();

  std::map<std::string, Parameter*> parameters;
  std::map<std::string, std::string> attributes;
  std::map<std::string, std::string> uniforms;
  std::set<GLTF::Constants::WebGL> enableStates;
  std::vector<GLTF::Constants::WebGL> blendEquationSeparate;
  std::vector<GLTF::Constants::WebGL> blendFuncSeparate;
  bool* depthMask = NULL;
  GLTF::Program* program = NULL;

  virtual std::string typeName();
  virtual void writeJSON(void* writer, GLTF::Options* options);
};
}  // namespace GLTF
