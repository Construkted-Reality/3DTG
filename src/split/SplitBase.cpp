#include "SplitBase.h"


std::map<std::string, SplitInterface::SplitCreator> SplitInterface::creatorList = {};

std::shared_ptr<SplitInterface> SplitInterface::create(std::string type) {
  if (SplitInterface::creatorList.count(type) == 0) {
    return nullptr;
  }

  return SplitInterface::creatorList[type]();
};

void SplitInterface::addCreator(const std::string type, SplitInterface::SplitCreator creator) {
  SplitInterface::creatorList[type] = creator;
};
