#include "./GLTFExporter.h"



const std::string GLTFExporter::Type = "glb";
std::shared_ptr<Exporter> GLTFExporter::create() {
  return std::make_shared<GLTFExporter>();
};

void GLTFExporter::updateAccessorFace(std::map<std::string, GLTF::Accessor*> &accessors, MeshObject &mesh, Face &face) {
  for (unsigned int i = 0; i < 3; i++) {
    GLTFExporter::updateAccessorMin3f(accessors[GLTF_BUFFER::POSITION_BUFFER], mesh->position[face.positionIndices[i]]);
    GLTFExporter::updateAccessorMax3f(accessors[GLTF_BUFFER::POSITION_BUFFER], mesh->position[face.positionIndices[i]]);

    // if (mesh->hasNormals && accessors.count(GLTF_BUFFER::NORMAL_BUFFER)) {
    //   GLTFExporter::updateAccessorMin3f(accessors[GLTF_BUFFER::NORMAL_BUFFER], mesh->normal[face.normalIndices[i]]);
    //   GLTFExporter::updateAccessorMax3f(accessors[GLTF_BUFFER::NORMAL_BUFFER], mesh->normal[face.normalIndices[i]]);
    // }

    // if (mesh->hasUVs && accessors.count(GLTF_BUFFER::UV_BUFFER)) {
    //   GLTFExporter::updateAccessorMin2f(accessors[GLTF_BUFFER::UV_BUFFER], mesh->uv[face.uvIndices[i]]);
    //   GLTFExporter::updateAccessorMax2f(accessors[GLTF_BUFFER::UV_BUFFER], mesh->uv[face.uvIndices[i]]);
    // }
  }
};

void GLTFExporter::updateAccessorIndex(GLTF::Accessor* accessor, std::vector<unsigned int> &data) {
  accessor->min[0] = static_cast<float>(data[0]);
  accessor->max[0] = static_cast<float>(data[0]);

  for (unsigned int &index : data) {
    accessor->min[0] = std::min(accessor->min[0], static_cast<float>(index));
    accessor->max[0] = std::max(accessor->max[0], static_cast<float>(index));
  }
};

void GLTFExporter::updateAccessorMin3f(GLTF::Accessor* accessor, glm::vec3 &vec) {
  accessor->min[0] = std::min(accessor->min[0], vec.x);
  accessor->min[1] = std::min(accessor->min[1], vec.y);
  accessor->min[2] = std::min(accessor->min[2], vec.z);
};
void GLTFExporter::updateAccessorMax3f(GLTF::Accessor* accessor, glm::vec3 &vec) {
  accessor->max[0] = std::max(accessor->max[0], vec.x);
  accessor->max[1] = std::max(accessor->max[1], vec.y);
  accessor->max[2] = std::max(accessor->max[2], vec.z);
};

void GLTFExporter::updateAccessorMin2f(GLTF::Accessor* accessor, glm::vec2 &vec) {
  accessor->min[0] = std::min(accessor->min[0], vec.x);
  accessor->min[1] = std::min(accessor->min[1], vec.y);
};
void GLTFExporter::updateAccessorMax2f(GLTF::Accessor* accessor, glm::vec2 &vec) {
  accessor->max[0] = std::max(accessor->max[0], vec.x);
  accessor->max[1] = std::max(accessor->max[1], vec.y);
};

void GLTFExporter::save(std::string directory, std::string fileName, GroupObject object, bool indexedGeometry) {
  std::string exportModelPath = utils::concatPath(directory, fileName + "." + this->format);

  if (!utils::folder_exists(directory)) {
    utils::mkdir(directory.c_str());
  }
  // std::cout << "Export begin" << std::endl;
  
  std::vector<GLTF::Node*> memNodes;
  std::vector<GLTF::Mesh*> memMeshes;
  std::vector<std::map<std::string, GLTF::Accessor*>> memAccessors;
  std::vector<std::map<std::string, GLTF::Primitive*>> memPrimitives;
  std::vector<GLTF::MaterialCommon*> memMaterials;
  std::vector<GLTF::Material::Values*> memMaterialValues;
  std::vector<GLTF::Texture*> memTextures;
  std::vector<GLTF::Sampler*> memSamplers;
  std::vector<GLTF::Image*> memImages;


  GLTF::Scene *scene = new GLTF::Scene();
  // scene->nodes.push_back(node);

  GLTF::Asset::Metadata meta;
  meta.generator = "3DTG";

  GLTF::Asset *asset = new GLTF::Asset();
  asset->metadata = &meta;


  object->traverse([&](MeshObject targetMesh){
    // std::cout << "Mesh processing" << std::endl;
    
    GLTF::Node *node = new GLTF::Node();
    // node->mesh = mesh;

    GLTF::Mesh *mesh = new GLTF::Mesh();
    // mesh->primitives.push_back(primitive);

    std::map<std::string, GLTF::Accessor*> accessors;
    std::map<std::string, GLTF::Primitive*> primitives;

    std::vector<float> vertices;
    std::vector<float> normals;
    std::vector<float> uvs;

    std::vector<unsigned int> positionIndexBuffer;
    std::vector<unsigned int> normalIndexBuffer;
    std::vector<unsigned int> uvIndexBuffer;

    /*
    for (Face &face : targetMesh->faces) // access by reference to avoid copying
    {
      // positionIndexBuffer.insert(positionIndexBuffer.end(), face.positionIndices, face.positionIndices + 3);

      vertices.push_back(targetMesh->position[face.positionIndices[0]].x);
      vertices.push_back(targetMesh->position[face.positionIndices[0]].y);
      vertices.push_back(targetMesh->position[face.positionIndices[0]].z);

      vertices.push_back(targetMesh->position[face.positionIndices[1]].x);
      vertices.push_back(targetMesh->position[face.positionIndices[1]].y);
      vertices.push_back(targetMesh->position[face.positionIndices[1]].z);

      vertices.push_back(targetMesh->position[face.positionIndices[2]].x);
      vertices.push_back(targetMesh->position[face.positionIndices[2]].y);
      vertices.push_back(targetMesh->position[face.positionIndices[2]].z);

      if (targetMesh->hasNormals) {
        // normalIndexBuffer.insert(normalIndexBuffer.end(), face.normalIndices, face.normalIndices + 3);

        Vector3f normal1 = targetMesh->normal[face.normalIndices[0]].clone();
        Vector3f normal2 = targetMesh->normal[face.normalIndices[1]].clone();
        Vector3f normal3 = targetMesh->normal[face.normalIndices[2]].clone();

        normal1.normalize();
        normal2.normalize();
        normal3.normalize();

        normals.push_back(normal1.x);
        normals.push_back(normal1.y);
        normals.push_back(normal1.z);

        normals.push_back(normal2.x);
        normals.push_back(normal2.y);
        normals.push_back(normal2.z);

        normals.push_back(normal3.x);
        normals.push_back(normal3.y);
        normals.push_back(normal3.z);
      }

      if (targetMesh->hasUVs) {
        // uvIndexBuffer.insert(uvIndexBuffer.end(), face.uvIndices, face.uvIndices + 3);

        uvs.push_back(targetMesh->uv[face.uvIndices[0]].x);
        uvs.push_back(targetMesh->uv[face.uvIndices[0]].y);

        uvs.push_back(targetMesh->uv[face.uvIndices[1]].x);
        uvs.push_back(targetMesh->uv[face.uvIndices[1]].y);

        uvs.push_back(targetMesh->uv[face.uvIndices[2]].x);
        uvs.push_back(targetMesh->uv[face.uvIndices[2]].y);
      }
    }
    */

    // DATA BUFFERS
    // std::cout << "Generating data buffers" << std::endl;
    if (targetMesh->position.size()) {
      GLTF::Accessor* accessor = new GLTF::Accessor(
        GLTF::Accessor::Type::VEC3, GLTF::Constants::WebGL::FLOAT,
        (unsigned char*)&targetMesh->position[0],
        targetMesh->position.size(),// targetMesh->position.size(),//  / 3
        GLTF::Constants::WebGL::ARRAY_BUFFER
      );

      accessors[GLTF_BUFFER::POSITION_BUFFER] = accessor;
    }
    
    if (targetMesh->hasNormals) {
      GLTF::Accessor* accessor = new GLTF::Accessor(
        GLTF::Accessor::Type::VEC3, GLTF::Constants::WebGL::FLOAT,
        (unsigned char*)&targetMesh->normal[0],
        targetMesh->normal.size(),// targetMesh->normal.size(),
        GLTF::Constants::WebGL::ARRAY_BUFFER
      );

      accessors[GLTF_BUFFER::NORMAL_BUFFER] = accessor;
    }

    if (targetMesh->hasUVs) {
      GLTF::Accessor* accessor = new GLTF::Accessor(
        GLTF::Accessor::Type::VEC2, GLTF::Constants::WebGL::FLOAT,
        (unsigned char*)&targetMesh->uv[0],
        targetMesh->uv.size(),// targetMesh->uv.size(),
        GLTF::Constants::WebGL::ARRAY_BUFFER
      );

      accessors[GLTF_BUFFER::UV_BUFFER] = accessor;
    }

    std::vector<unsigned int> indices;
    for (Face &face : targetMesh->faces) {
      indices.push_back(face.positionIndices[0]);
      indices.push_back(face.positionIndices[1]);
      indices.push_back(face.positionIndices[2]);
    }

    GLTF::Accessor* accessor = new GLTF::Accessor(
      GLTF::Accessor::Type::SCALAR, GLTF::Constants::WebGL::UNSIGNED_INT,
      (unsigned char*)&indices[0],
      indices.size(),// targetMesh->uv.size(),
      GLTF::Constants::WebGL::ELEMENT_ARRAY_BUFFER
    );

    accessors[GLTF_BUFFER::INDEX_POSITION_BUFFER] = accessor;

    // DATA BUFFERS END


    // MATERIAL START
    GLTF::MaterialCommon *material = new GLTF::MaterialCommon();
    material->type = GLTF::Material::Type::MATERIAL_COMMON;
    material->technique = GLTF::MaterialCommon::Technique::CONSTANT;
    material->doubleSided = true;

    GLTF::Material::Values *values = new GLTF::Material::Values();

    if (targetMesh->material->diffuseMapImage.data == NULL) {
      float* diffuse = new float[4]{1.0f, 1.0f, 1.0f, 1.0f};
      values->diffuse = diffuse;
    } else {
      GLTFExporter::ImageData ii;

      stbi_flip_vertically_on_write(1);
      int writeSuccess = stbi_write_jpg_to_func(
        [](void *context, void *data, int size)
        {
          ((ImageData*)context)->data.write((const char*)data, size);
          ((ImageData*)context)->size += size;
        }
        ,&ii,
        targetMesh->material->diffuseMapImage.width,
        targetMesh->material->diffuseMapImage.height,
        targetMesh->material->diffuseMapImage.channels,
        targetMesh->material->diffuseMapImage.data,
        80
      );
      

      // std::cout << "Image generating finished, size: " << ii.size << std::endl;

      if (!writeSuccess) {
        float* diffuse = new float[4]{1.0f, 1.0f, 1.0f, 1.0f};
        values->diffuse = diffuse;
      } else {

        char * imageData = new char [ii.size];
        ii.data.read(imageData, ii.size);

        // unsigned char* imageData = (unsigned char*) ii.data.str().c_str();

        // if ((unsigned char) imageData[0] == 255 && (unsigned char) imageData[1] == 216) {
          // std::cout << "Is JPEG!" << std::endl;
        // }

        // std::cout << "JPEG size: " << ii.data.str().size() << std::endl;

        // unsigned char * buffer = new unsigned char [ii.data.str().size()];
        // ii.data.read(buffer, ii.data.str().size());

        values->diffuseTexture = new GLTF::Texture();

        values->diffuseTexture->sampler = new GLTF::Sampler();
        // values->diffuseTexture->sampler->minFilter = GLTF::Constants::WebGL::LINEAR;

        values->diffuseTexture->source = new GLTF::Image(
          targetMesh->material->name,
          (unsigned char *) imageData,
          ii.size,
          "jpeg"
        );

        memTextures.push_back(values->diffuseTexture);
        memSamplers.push_back(values->diffuseTexture->sampler);
        memImages.push_back(values->diffuseTexture->source);
      }

      
      // values->diffuseTexture->source->bufferView = imageView;
      // values->diffuseTexture->source->mimeType = "image/jpeg";

      // int size = (sizeof(imageView->buffer->data)/sizeof(imageView->buffer->data[0]));
      // std::cout << "Buffer size: " << size << ", " << targetMesh->material.diffuseMapImage.width << ", " << targetMesh->material.name.c_str() << std::endl;
    }

    // values->diffuse = diffuse;
    

    material->values = values;
    // MATERIAL END


    // PRIMITIVE START
    // std::cout << "Generating primitive" << std::endl;

    GLTF::Primitive *primitive = new GLTF::Primitive();
    if (indexedGeometry) {
      primitive->indices = accessors[GLTF_BUFFER::INDEX_POSITION_BUFFER];
    }
    // primitive->indices = accessors[GLTF_BUFFER::INDEX_POSITION_BUFFER];
    primitive->attributes["POSITION"] = accessors[GLTF_BUFFER::POSITION_BUFFER];
    if (accessors.count(GLTF_BUFFER::NORMAL_BUFFER)) {
      primitive->attributes["NORMAL"] = accessors[GLTF_BUFFER::NORMAL_BUFFER];
    }
    if (accessors.count(GLTF_BUFFER::UV_BUFFER)) {
      primitive->attributes["TEXCOORD_0"] = accessors[GLTF_BUFFER::UV_BUFFER];
    }
    primitive->mode = GLTF::Primitive::Mode::TRIANGLES;
    primitive->material = material;

    mesh->primitives.push_back(primitive);
    primitives[GLTF_BUFFER::POSITION_BUFFER] = primitive;

    if (targetMesh->position.size()) {
      node->mesh = mesh;
      scene->nodes.push_back(node);

      memMeshes.push_back(mesh);
      memNodes.push_back(node);
    } else {
      delete mesh;
      delete node;
    }
    // PRIMITIVE END

    // SAVE REFS TO DELETE
    // std::cout << "Generating cleanup info" << std::endl;

    memAccessors.push_back(accessors);
    memPrimitives.push_back(primitives);
    memMaterials.push_back(material);
    memMaterialValues.push_back(values);
  });

  // std::cout << "Model data has been collected, generating gltf..." << std::endl;

  asset->scenes.push_back(scene);
  asset->scene = 0;

  GLTF::Options gltfOptions;
  gltfOptions.binary = true;
  gltfOptions.dracoCompression = true;
  gltfOptions.embeddedTextures = true;
  // gltfOptions.materialsCommon = true;

  // std::cout << "GLTF has been generated, exporting..." << std::endl;

  this->exportGLTF(asset, &gltfOptions, utils::normalize(exportModelPath).c_str());
  

  // std::cout << "Exported, cleaning up..." << std::endl;

  
  // delete asset;

  /*
  delete scene;

  
  for ( GLTF::MaterialCommon* material : memMaterials ) {
    delete material;
  }

  for ( GLTF::Material::Values* values : memMaterialValues ) {
    delete values;
  }


  for ( GLTF::Texture* texture : memTextures ) {
    delete texture;
  }

  for ( GLTF::Sampler* sampler : memSamplers ) {
    delete sampler;
  }

  for ( GLTF::Image* image : memImages ) {
    delete image;
  }

  for ( GLTF::Mesh* mesh : memMeshes ) {
    delete mesh;
  }

  for ( GLTF::Node* node : memNodes ) {
    delete node;
  }

  for ( std::map<std::string, GLTF::Accessor*> &accessorMap : memAccessors ) {
    for (std::map<std::string, GLTF::Accessor*>::iterator it = accessorMap.begin(); it != accessorMap.end(); it++)
    {
      delete it->second;
    }
    accessorMap.clear();
  }

  for ( std::map<std::string, GLTF::Primitive*> &primitiveMap : memPrimitives ) {
    for (std::map<std::string, GLTF::Primitive*>::iterator it = primitiveMap.begin(); it != primitiveMap.end(); it++)
    {
      delete it->second;
    }
    primitiveMap.clear();
  }
  */

  memMaterials.clear();
  memMaterialValues.clear();
  memTextures.clear();
  memSamplers.clear();
  memImages.clear();
  memMeshes.clear();
  memNodes.clear();
  memAccessors.clear();
  memPrimitives.clear();

};


void GLTFExporter::exportGLTF(GLTF::Asset *asset, GLTF::Options *options, const char* outputPath) {
  
  // std::cout << "Removing unused nodes" << std::endl;
  asset->removeUnusedNodes(options);

  if (!options->preserveUnusedSemantics) {
    // std::cout << "Removing unused semantics" << std::endl;
    asset->removeUnusedSemantics();
  }

  if (options->dracoCompression) {
    // std::cout << "Draco compression" << std::endl;
    asset->removeUncompressedBufferViews();
    asset->compressPrimitives(options);
  }

  // std::cout << "Packing" << std::endl;
  GLTF::Buffer* buffer = asset->packAccessors();
  if (options->binary && options->version == "1.0") {
    buffer->stringId = "binary_glTF";
  }

  // std::cout << "Packing finished" << std::endl;

  // Create image bufferViews for binary glTF
  // std::cout << "Texture saving started" << std::endl;
  
  if (options->binary && options->embeddedTextures) {
    size_t imageBufferLength = 0;
    std::vector<GLTF::Image*> images = asset->getAllImages();
    for (GLTF::Image* image : images) {
      imageBufferLength += image->byteLength;
    }

    // std::cout << "Image buffer length: " << imageBufferLength << std::endl;

    unsigned char* bufferData = buffer->data;
    bufferData = (unsigned char*)realloc(
        bufferData, buffer->byteLength + imageBufferLength);
    size_t byteOffset = buffer->byteLength;
    for (GLTF::Image* image : images) {
      GLTF::BufferView* bufferView =
          new GLTF::BufferView(byteOffset, image->byteLength, buffer);
      image->bufferView = bufferView;
      std::memcpy(bufferData + byteOffset, image->data, image->byteLength);
      byteOffset += image->byteLength;
    }
    buffer->data = bufferData;
    buffer->byteLength += imageBufferLength;
  }

  // std::cout << "Writing options to json" << std::endl; 

  rapidjson::StringBuffer s;
  rapidjson::Writer<rapidjson::StringBuffer> jsonWriter =
      rapidjson::Writer<rapidjson::StringBuffer>(s);
  jsonWriter.StartObject();
  asset->writeJSON(&jsonWriter, options);
  jsonWriter.EndObject();

  // std::cout << "Options written" << std::endl;

  /*
  if (!options->embeddedTextures) {
    for (GLTF::Image* image : asset->getAllImages()) {
      COLLADABU::URI imageURI =
          COLLADABU::URI::nativePathToUri(outputPathDir + image->uri);
      std::string imageString =
          imageURI.toNativePath(COLLADABU::Utils::getSystemType());
      FILE* file = fopen(imageString.c_str(), "wb");
      if (file != NULL) {
        fwrite(image->data, sizeof(unsigned char), image->byteLength, file);
        fclose(file);
      } else {
        std::cout << "ERROR: Couldn't write image to path '" << imageString
                  << "'" << std::endl;
      }
    }
  }
  */

  /*
  if (!options->embeddedBuffers) {
    COLLADABU::URI bufferURI =
        COLLADABU::URI::nativePathToUri(outputPathDir + buffer->uri);
    std::string bufferString =
        bufferURI.toNativePath(COLLADABU::Utils::getSystemType());
    FILE* file = fopen(bufferString.c_str(), "wb");
    if (file != NULL) {
      fwrite(buffer->data, sizeof(unsigned char), buffer->byteLength, file);
      fclose(file);
    } else {
      std::cout << "ERROR: Couldn't write buffer to path '" << bufferString
                << "'" << std::endl;
    }
  }
  */

  /*
  if (!options->embeddedShaders) {
    for (GLTF::Shader* shader : asset->getAllShaders()) {
      COLLADABU::URI shaderURI =
          COLLADABU::URI::nativePathToUri(outputPathDir + shader->uri);
      std::string shaderString =
          shaderURI.toNativePath(COLLADABU::Utils::getSystemType());
      FILE* file = fopen(shaderString.c_str(), "wb");
      if (file != NULL) {
        fwrite(shader->source.c_str(), sizeof(unsigned char),
                shader->source.length(), file);
        fclose(file);
      } else {
        std::cout << "ERROR: Couldn't write shader to path '" << shaderString
                  << "'" << std::endl;
      }
    }
  }
  */
  // std::cout << "Writing gltf" << std::endl;
  std::string jsonString = s.GetString();
  if (!options->binary) {
    rapidjson::Document jsonDocument;
    jsonDocument.Parse(jsonString.c_str());

    rapidjson::StringBuffer buffer;
    rapidjson::PrettyWriter<rapidjson::StringBuffer> writer(buffer);
    jsonDocument.Accept(writer);

    std::ofstream file(outputPath);
    if (file.is_open()) {
      file << buffer.GetString() << std::endl;
      file.close();
    } else {
      std::cout << "ERROR: couldn't write glTF to path '"
                << outputPath << "'" << std::endl;
    }
  } else {
    // std::cout << "Is binary" << std::endl;
    FILE* file = fopen(outputPath, "wb");
    if (file != NULL) {
      int jsonPadding = (4 - (jsonString.length() & 3)) & 3;
      int binPadding = (4 - (buffer->byteLength & 3)) & 3;

      size_t totalBinSize = 0;

      totalBinSize += sizeof(char) * 4;// glTF magic
      totalBinSize += sizeof(uint32_t) * 2;// GLB header
      totalBinSize += sizeof(uint32_t) * 2;// JSON header
      totalBinSize += sizeof(char) * jsonString.length();// JSON
      totalBinSize += sizeof(char) * jsonPadding;// JSON padding
      if (options->version != "1.0") {
        totalBinSize += sizeof(uint32_t) * 2;// BIN chunk header
      }
      totalBinSize += sizeof(unsigned char) * buffer->byteLength;// BIN buffer
      totalBinSize += sizeof(char) * binPadding;// BIN padding

      if (this->beforeBinWrite) {
        this->beforeBinWrite(file, totalBinSize);
      }

      fwrite("glTF", sizeof(char), 4, file);  // magic

      uint32_t* writeHeader = new uint32_t[2];
      // version
      if (options->version == "1.0") {
        writeHeader[0] = 1;
      } else {
        writeHeader[0] = 2;
      }

      writeHeader[1] =
          GLTFExporter::HEADER_LENGTH +
          (GLTFExporter::CHUNK_HEADER_LENGTH + jsonString.length() + jsonPadding +
            buffer->byteLength + binPadding);  // length
      if (options->version != "1.0") {
        writeHeader[1] += GLTFExporter::CHUNK_HEADER_LENGTH;
      }
      fwrite(writeHeader, sizeof(uint32_t), 2, file);  // GLB header

      writeHeader[0] =
          jsonString.length() +
          jsonPadding;  // 2.0 - chunkLength / 1.0 - contentLength
      if (options->version == "1.0") {
        writeHeader[1] = 0;  // 1.0 - contentFormat
      } else {
        writeHeader[1] = 0x4E4F534A;  // 2.0 - chunkType JSON
      }
      fwrite(writeHeader, sizeof(uint32_t), 2, file);
      fwrite(jsonString.c_str(), sizeof(char), jsonString.length(), file);
      for (int i = 0; i < jsonPadding; i++) {
        fwrite(" ", sizeof(char), 1, file);
      }
      if (options->version != "1.0") {
        writeHeader[0] = buffer->byteLength + binPadding;  // chunkLength
        writeHeader[1] = 0x004E4942;                       // chunkType BIN
        fwrite(writeHeader, sizeof(uint32_t), 2, file);
      }
      fwrite(buffer->data, sizeof(unsigned char), buffer->byteLength, file);
      for (int i = 0; i < binPadding; i++) {
        fwrite("\0", sizeof(char), 1, file);
      }
      delete[] writeHeader;

      fclose(file);
    } else {
      std::cout << "ERROR couldn't write binary glTF to path '"
                << outputPath << "'" << std::endl;
    }
  }
};
