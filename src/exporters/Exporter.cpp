#include "Exporter.h"

std::map<std::string, Exporter::ExporterCreator> Exporter::creatorList = {};

std::shared_ptr<Exporter> Exporter::create(std::string type) {
  if (Exporter::creatorList.count(type) == 0) {
    return nullptr;
  }

  return Exporter::creatorList[type]();
};

void Exporter::addCreator(const std::string type, Exporter::ExporterCreator creator) {
  Exporter::creatorList[type] = creator;
};
