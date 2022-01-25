#include "./TileBoundingVolume.h"

std::vector<float> TileBoundingBox::toArray() {
  std::vector<float> result;

  result.push_back(this->center.x);
  result.push_back(this->center.y);
  result.push_back(this->center.z);

  result.push_back(this->xHalf.x);
  result.push_back(0.0f);
  result.push_back(0.0f);

  result.push_back(0.0f);
  // result.push_back(this->yHalf.y);
  result.push_back(this->zHalf.z);
  result.push_back(0.0f);

  result.push_back(0.0f);
  // result.push_back(this->zHalf.y);
  // result.push_back(this->zHalf.z);
  result.push_back(0.0f);
  result.push_back(this->yHalf.y);

  return result;
};

std::vector<float> TileBoundingSphere::toArray() {
  std::vector<float> result;



  return result;
};

std::vector<float> TileBoundingRegion::toArray() {
  std::vector<float> result;



  return result;
};
