#include "./B3DMExporter.h"

void B3DMExporter::save(std::string directory, std::string fileName, GroupObject object) {
  GLTFExporter exporter;
  exporter.format = this->format;

  unsigned int headerByteLength = 28;
  
  nlohmann::json featureTableJson;
  featureTableJson["BATCH_LENGTH"] = 0;


  std::string featureTableString = featureTableJson.dump();
  unsigned int boundary = 8;
  unsigned int byteLength = featureTableString.length();
  unsigned int remainder = (headerByteLength + byteLength) % boundary;
  unsigned int padding = (remainder == 0) ? 0 : (boundary - remainder);

  for (unsigned int i = 0; i < padding; ++i) {
    featureTableString += " ";
  }

  exporter.beforeBinWrite = [&](FILE* file, size_t binarySize){
    fwrite("b3dm", sizeof(char), 4, file);  // magic

    uint32_t* writeHeader = new uint32_t[1];

    writeHeader[0] = 1;
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // version

    writeHeader[0] = headerByteLength + featureTableString.length() + binarySize;
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // byteLength - length of entire tile, including header, in bytes

    writeHeader[0] = featureTableString.size();
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // featureTableJSONByteLength - length of feature table JSON section in bytes.

    writeHeader[0] = 0;
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // featureTableBinaryByteLength - length of feature table binary section in bytes.

    writeHeader[0] = 0;
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // batchTableJSONByteLength - length of batch table JSON section in bytes. (0 for basic, no batches)

    writeHeader[0] = 0;
    fwrite(writeHeader, sizeof(uint32_t), 1, file);  // batchTableBinaryByteLength - length of batch table binary section in bytes. (0 for basic, no batches)

    fwrite(featureTableString.c_str(), sizeof(char), featureTableString.length(), file);  // featureTableJSONBuffer
  };

  exporter.save(directory, fileName, object);
};