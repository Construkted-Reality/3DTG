#include "./IdGenerator.h"

void IdGenerator::reset() {
  this->id = 0;
};

IdGenerator::ID IdGenerator::next() {
  return ++this->id;
};
