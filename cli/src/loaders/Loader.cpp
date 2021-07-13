#include "./Loader.h"



void Group::traverse(TraverseMeshCallback fn) {
  for (GroupObject &group : this->children) // access by reference to avoid copying
  {  
    group->traverse(fn);
  }

  for (MeshObject &mesh : this->meshes) // access by reference to avoid copying
  {  
    fn(mesh);
  }
};

void Group::traverseGroup(TraverseGroupCallback fn) {
  for (GroupObject &group : this->children) // access by reference to avoid copying
  {
    fn(group);
    group->traverseGroup(fn);
  }
};

void Group::computeBoundingBox() {
  this->traverse([&](MeshObject mesh){
    this->boundingBox.extend(mesh->boundingBox);
  });
};

void Group::computeUVBox() {
  this->traverse([&](MeshObject mesh){
    this->uvBox.extend(mesh->uvBox);
  });
};


void Mesh::finish() {
  this->hasNormals = this->normal.size() > 0;
  this->hasUVs = this->uv.size() > 0;

  // std::cout << "Normals length: " << this->normal.size() << std::endl;

  std::cout << "Loading finished: " << this->name << std::endl;
};

Loader::Loader() {
  this->object->name = "root";
};


void BBoxf::extend(Vector3f position) {
  this->min.x = utils::min(this->min.x, position.x);
  this->min.y = utils::min(this->min.y, position.y);
  this->min.z = utils::min(this->min.z, position.z);

  this->max.x = utils::max(this->max.x, position.x);
  this->max.y = utils::max(this->max.y, position.y);
  this->max.z = utils::max(this->max.z, position.z);
};

void BBoxf::extend(BBoxf box) {
  this->min.x = utils::min(this->min.x, box.min.x);
  this->min.y = utils::min(this->min.y, box.min.y);
  this->min.z = utils::min(this->min.z, box.min.z);

  this->max.x = utils::max(this->max.x, box.max.x);
  this->max.y = utils::max(this->max.y, box.max.y);
  this->max.z = utils::max(this->max.z, box.max.z);
};

void BBoxf::extend(float x, float y, float z) {
  this->min.x = utils::min(this->min.x, x);
  this->min.y = utils::min(this->min.y, y);
  this->min.z = utils::min(this->min.z, z);

  this->max.x = utils::max(this->max.x, x);
  this->max.y = utils::max(this->max.y, y);
  this->max.z = utils::max(this->max.z, z);
};

Vector3f BBoxf::size() {
  Vector3f size;

  size.x = this->max.x - this->min.x;
  size.y = this->max.y - this->min.y;
  size.z = this->max.z - this->min.z;

  return size;
};

bool BBoxf::intersect(Vector3f point) {
  bool intersectX = (point.x >= this->min.x) && (point.x <= this->max.x);
  bool intersectY = (point.y >= this->min.y) && (point.y <= this->max.y);
  bool intersectZ = (point.z >= this->min.z) && (point.z <= this->max.z);

  return intersectX && intersectY && intersectZ;
};

bool BBoxf::intersect(BBoxf box) {
  BBoxf result;

  result.extend(this->min);
  result.extend(this->max);

  result.extend(box);

  Vector3f selfSize = this->size();
  Vector3f boxSize = box.size();
  Vector3f resultSize = result.size();

  bool intersectX = resultSize.x <= (selfSize.x + boxSize.x);
  bool intersectY = resultSize.y <= (selfSize.y + boxSize.y);
  bool intersectZ = resultSize.z <= (selfSize.z + boxSize.z);

  return intersectX && intersectY && intersectZ;
};

void BBoxf::translate(float x, float y, float z) {
  Vector3f size = this->size();

  this->min.x = x;
  this->min.y = y;
  this->min.z = z;

  this->max.x = x + size.x;
  this->max.y = y + size.y;
  this->max.z = z + size.z;
};

void BBoxf::fromPoint(float x, float y, float z) {
  this->min.x = x;
  this->min.y = y;
  this->min.z = z;

  this->max.x = x;
  this->max.y = y;
  this->max.z = z;
};


float Vector3f::dot(Vector3f vector) {
  return this->x * vector.x + this->y * vector.y + this->z * vector.z;
};

float Vector3f::length() {
  return sqrt(this->x * this->x + this->y * this->y + this->z *this->z);
};

void Vector3f::divideScalar(float value) {
  this->x /= value;
  this->y /= value;
  this->z /= value;
};

void Vector3f::normalize() {
  float length = this->length();
  this->divideScalar(length);
};

void Vector3f::multiplyScalar(float value) {
  this->x *= value;
  this->y *= value;
  this->z *= value;
};

void Vector3f::add(Vector3f vector) {
  this->x += vector.x;
  this->y += vector.y;
  this->z += vector.z;
};

void Vector3f::sub(Vector3f vector) {
  this->x -= vector.x;
  this->y -= vector.y;
  this->z -= vector.z;
};

Vector3f Vector3f::clone() {
  Vector3f cloned;

  cloned.x = this->x;
  cloned.y = this->y;
  cloned.z = this->z;

  return cloned;
};

Vector3f Vector3f::intersectPlane(Vector3f planePoint, Vector3f planeNormal, Vector3f lineBegin, Vector3f lineDirection) {
  Vector3f direction = lineDirection.clone();
  direction.normalize();

  float t = (planeNormal.dot(planePoint) - planeNormal.dot(lineBegin)) / planeNormal.dot(lineDirection);
  direction.multiplyScalar(t);

  Vector3f result = lineBegin.clone();
  result.add(direction);

  return result;
};

void Image::free() {
  if (this->data != NULL) {
    stbi_image_free(this->data);
  }
};

void Loader::free() {
  std::cout << "Cleaning up the memory..." << std::endl; 
  this->object->traverse([&](MeshObject mesh){
    mesh->material.diffuseMapImage.free();
  });

  std::cout << "Memory has been cleaned" << std::endl; 
};

