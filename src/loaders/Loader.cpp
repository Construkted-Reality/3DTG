#include "./Loader.h"


MaterialObject Material::clone(bool deep) {
  MaterialObject next = std::make_shared<Material>();

  next->name = this->name;
  // next->baseName = this->baseName;
  next->color = this->color;

  if (deep) {
    next->diffuseMap = this->diffuseMap;
  }

  return next;
};

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
  unsigned int meshNumber = 0;
  for (MeshObject &mesh : this->meshes) // access by reference to avoid copying
  {
    if (meshNumber == 0) { // Set first captured bbox as a base
      this->boundingBox = mesh->boundingBox.clone();
    } else {
      this->boundingBox.extend(mesh->boundingBox);
    }
    
    meshNumber++;
  }

  unsigned int groupNumber = 0;
  for (GroupObject &group : this->children) // access by reference to avoid copying
  {
    group->computeBoundingBox();

    if (groupNumber == 0 && meshNumber == 0) { // Set first captured bbox as a base
      this->boundingBox = group->boundingBox.clone();
    } else {
      this->boundingBox.extend(group->boundingBox);
    }
  }
};

void Group::computeGeometricError() {
  unsigned int meshNumber = 0;
  for (MeshObject &mesh : this->meshes) // access by reference to avoid copying
  {
    if (meshNumber == 0) { // Set first captured error as a base
      this->geometricError = mesh->geometricError;
    } else {
      this->geometricError += mesh->geometricError;
      // this->geometricError = std::max(mesh->geometricError, this->geometricError);
    }
    
    meshNumber++;
  }

  if (meshNumber != 0) {
    this->geometricError /= (float) meshNumber;
  } else {
    unsigned int groupNumber = 0;
    for (GroupObject &group : this->children) // access by reference to avoid copying
    {
      group->computeGeometricError();

      if (groupNumber == 0) { // Set first captured error as a base
        this->geometricError = group->geometricError;
      } else {
        this->geometricError += group->geometricError;
        // this->geometricError = std::max(group->geometricError, this->geometricError);
      }
    }

    if (groupNumber != 0) {
      this->geometricError /= (float) groupNumber;
    }
  }
};

void Group::computeUVBox() {
  // this->traverse([&](MeshObject mesh){
  //   this->uvBox.extend(mesh->uvBox);
  // });

  unsigned int meshNumber = 0;
  for (MeshObject &mesh : this->meshes) // access by reference to avoid copying
  {
    if (meshNumber == 0) { // Set first captured bbox as a base
      this->uvBox = mesh->uvBox.clone();
    } else {
      this->uvBox.extend(mesh->uvBox);
    }
    
    meshNumber++;
  }

  unsigned int groupNumber = 0;
  for (GroupObject &group : this->children) // access by reference to avoid copying
  {
    group->computeUVBox();

    if (groupNumber == 0 && meshNumber == 0) { // Set first captured bbox as a base
      this->uvBox = group->uvBox.clone();
    } else {
      this->uvBox.extend(group->uvBox);
    }
  }
};

void Group::free(bool deep) {
  this->traverse([&](MeshObject mesh){
    mesh->free(deep);
  });

  for (GroupObject &child : this->children) {
    child->free(deep);
  }

  this->meshes.clear();
  this->children.clear();
};

GroupObject Group::clone() {
  GroupObject cloned = GroupObject(new Group());

  cloned->name = this->name;
  cloned->geometricError = this->geometricError;

  cloned->boundingBox = this->boundingBox.clone();
  cloned->uvBox = this->uvBox.clone();

  for (GroupObject &group : this->children) {
    cloned->children.push_back(group->clone());
  }

  for (MeshObject &mesh : this->meshes) {
    cloned->meshes.push_back(mesh->clone());
  }

  return cloned;
};

Mesh::Mesh() {
  this->material = std::make_shared<Material>();
};

MeshObject Mesh::clone() {
  MeshObject cloned = MeshObject(new Mesh());

  cloned->name = this->name;
  cloned->geometricError = this->geometricError;

  cloned->boundingBox = this->boundingBox.clone();
  cloned->uvBox = this->uvBox.clone();

  cloned->hasNormals = this->hasNormals;
  cloned->hasUVs = this->hasUVs;

  cloned->material = this->material;
  
  cloned->position.reserve(this->position.size());
  std::copy(this->position.begin(), this->position.end(), cloned->position.begin());

  cloned->normal.reserve(this->normal.size());
  std::copy(this->normal.begin(), this->normal.end(), cloned->normal.begin());

  cloned->uv.reserve(this->uv.size());
  std::copy(this->uv.begin(), this->uv.end(), cloned->uv.begin());

  cloned->faces.reserve(this->faces.size());
  std::copy(this->faces.begin(), this->faces.end(), cloned->faces.begin());

  for (GroupObject &group : this->children) {
    cloned->children.push_back(group->clone());
  }

  for (MeshObject &mesh : this->meshes) {
    cloned->meshes.push_back(mesh->clone());
  }

  return cloned;
};

void Mesh::finish() {
  this->hasNormals = this->normal.size() > 0;
  this->hasUVs = this->uv.size() > 0;

  // std::cout << "Vertices count: " << this->position.size() << std::endl;
  // std::cout << "Normals count: " << this->normal.size() << std::endl;

  // std::cout << "Loading finished: " << this->name << std::endl;
};

void Mesh::free(bool deep) {
  this->uv.clear();
  this->position.clear();
  this->normal.clear();
  this->faces.clear();
  
  /*
  this->material->name = "";
  */
  if (deep) {
    if (this->material->diffuseMapImage.data != NULL) {
      this->material->diffuseMapImage.free();
    }
    
    for (std::map<int, Image>::iterator it = this->material->mipMaps.begin(); it != this->material->mipMaps.end(); ++it) {
      it->second.free();
    }
  }

  this->material.reset();
};

void Mesh::triangulate() {
  std::vector<Vector3f> positions;
  std::vector<Vector3f> normals;
  std::vector<Vector2f> uvs;

  std::vector<Face> faces;

  unsigned int index = 0;
  for (Face &face : this->faces) {
    Face nextFace;

    for (unsigned int i = 0; i < 3; i++) {
      positions.push_back(this->position[face.positionIndices[i]]);

      if (this->hasNormals) {
        normals.push_back(this->normal[face.normalIndices[i]]);
      }

      if (this->hasUVs) {
        uvs.push_back(this->uv[face.uvIndices[i]]);
      }

      nextFace.positionIndices[i] = index;
      nextFace.normalIndices[i] = index;
      nextFace.uvIndices[i] = index;

      index++;
    }

    faces.push_back(nextFace);
  }

  this->position.swap(positions);
  this->normal.swap(normals);
  this->uv.swap(uvs);

  this->faces.swap(faces);
};

void Mesh::remesh(std::vector<Vector3f> &position, std::vector<Vector3f> &normal, std::vector<Vector2f> &uv) {
  std::map<unsigned int, Vector3f> positionMap;
  std::map<unsigned int, Vector3f> normalMap;
  std::map<unsigned int, Vector2f> uvMap;

  std::map<unsigned int, unsigned int> positionDestMap;
  std::map<unsigned int, unsigned int> normalDestMap;
  std::map<unsigned int, unsigned int> uvDestMap;

  for (Face &face : this->faces) // access by reference to avoid copying
  {
    for (unsigned int i = 0; i < 3; i++) {
      if (positionMap.count(face.positionIndices[i]) == 0) {
        positionMap[face.positionIndices[i]] = position[face.positionIndices[i]];
      }
      // positionMap[face.positionIndices[1]] = position[face.positionIndices[1]];
      // positionMap[face.positionIndices[2]] = position[face.positionIndices[2]];

      if (this->hasNormals) {
        if (normalMap.count(face.normalIndices[i]) == 0) {
          normalMap[face.normalIndices[i]] = normal[face.normalIndices[i]];
        }
        // normalMap[face.normalIndices[1]] = normal[face.normalIndices[1]];
        // normalMap[face.normalIndices[2]] = normal[face.normalIndices[2]];
      }

      if (this->hasUVs) {
        if (uvMap.count(face.uvIndices[i]) == 0) {
          uvMap[face.uvIndices[i]] = uv[face.uvIndices[i]];
        }
        // uvMap[face.uvIndices[1]] = uv[face.uvIndices[1]];
        // uvMap[face.uvIndices[2]] = uv[face.uvIndices[2]];
      }
    }
  }

  unsigned int lastPositionIndex = 0;
  unsigned int lastNormalIndex = 0;
  unsigned int lastUVIndex = 0;

  for (std::map<unsigned int, Vector3f>::iterator it = positionMap.begin(); it != positionMap.end(); ++it) {
    this->position.push_back(it->second);
    positionDestMap[it->first] = lastPositionIndex;

    lastPositionIndex++;

    if (this->position.size() == 1) {
      this->boundingBox.fromPoint(it->second.x, it->second.y, it->second.z);
    } else {
      this->boundingBox.extend(it->second.x, it->second.y, it->second.z);
    }
  }

  if (this->hasNormals) {
    for (std::map<unsigned int, Vector3f>::iterator it = normalMap.begin(); it != normalMap.end(); ++it) {
      this->normal.push_back(it->second);
      normalDestMap[it->first] = lastNormalIndex;

      lastNormalIndex++;
    }
  }

  if (this->hasUVs) {
    for (std::map<unsigned int, Vector2f>::iterator it = uvMap.begin(); it != uvMap.end(); ++it) {
      this->uv.push_back(it->second);
      uvDestMap[it->first] = lastUVIndex;

      lastUVIndex++;

      if (this->uv.size() == 1) {
        this->uvBox.fromPoint(it->second.x, it->second.y, 0.0f);
      } else {
        this->uvBox.extend(it->second.x, it->second.y, 0.0f);
      }
    }
  }

  for (Face &face : this->faces) // access by reference to avoid copying
  {
    face.positionIndices[0] = positionDestMap[face.positionIndices[0]];
    face.positionIndices[1] = positionDestMap[face.positionIndices[1]];
    face.positionIndices[2] = positionDestMap[face.positionIndices[2]];

    if (this->hasNormals) {
      face.normalIndices[0] = normalDestMap[face.normalIndices[0]];
      face.normalIndices[1] = normalDestMap[face.normalIndices[1]];
      face.normalIndices[2] = normalDestMap[face.normalIndices[2]];
    }

    if (this->hasUVs) {
      face.uvIndices[0] = uvDestMap[face.uvIndices[0]];
      face.uvIndices[1] = uvDestMap[face.uvIndices[1]];
      face.uvIndices[2] = uvDestMap[face.uvIndices[2]];
    }
  }

  // std::cout << "Mesh loading finished." << std::endl;

  this->finish();
};

void Mesh::computeBoundingBox() {
  for (unsigned int i = 0; i < this->position.size(); i++) {
    if (i == 0) {
      this->boundingBox.fromPoint(this->position[i].x, this->position[i].y, this->position[i].z);
    } else {
      this->boundingBox.extend(this->position[i].x, this->position[i].y, this->position[i].z);
    }
  }
};

void Mesh::computeUVBox() {
  for (unsigned int i = 0; i < this->uv.size(); i++) {
    if (i == 0) {
      this->uvBox.fromPoint(this->uv[i].x, this->uv[i].y, 0.0f);
    } else {
      this->uvBox.extend(this->uv[i].x, this->uv[i].y, 0.0f);
    }
  }
};

void Mesh::pushTriangle(Vector3f a, Vector3f b, Vector3f c, Vector3f n, Vector2f t1, Vector2f t2, Vector2f t3) {
  unsigned int lastVertex = this->position.size();
  unsigned int lastNormal = this->normal.size();
  unsigned int lastUV = this->uv.size();

  this->position.push_back(a);
  this->position.push_back(b);
  this->position.push_back(c);

  if (this->hasNormals) {
    this->normal.push_back(n);
  }

  if (this->hasUVs) {
    this->uv.push_back(t1);
    this->uv.push_back(t2);
    this->uv.push_back(t3);
  }
  

  Face f = {
    lastVertex, lastVertex + 1, lastVertex + 2,
    lastNormal, lastNormal, lastNormal,
    lastUV, lastUV + 1, lastUV + 2
  };

  this->faces.push_back(f);
};

void Mesh::pushQuad(Vector3f a, Vector3f b, Vector3f c, Vector3f d, Vector3f n1, Vector3f n2, Vector2f t1, Vector2f t2, Vector2f t3, Vector2f t4) {
  unsigned int lastVertex = this->position.size();
  unsigned int lastNormal = this->normal.size();
  unsigned int lastUV = this->uv.size();

  this->position.push_back(a);
  this->position.push_back(b);
  this->position.push_back(c);
  this->position.push_back(d);

  if (this->hasNormals) {
    this->normal.push_back(n1);
    this->normal.push_back(n2);
  }

  if (this->hasUVs) {
    this->uv.push_back(t1);
    this->uv.push_back(t2);
    this->uv.push_back(t3);
    this->uv.push_back(t4);
  }

  Face f1 = {
    lastVertex, lastVertex + 1, lastVertex + 3,
    lastNormal, lastNormal, lastNormal,
    lastUV, lastUV + 1, lastUV + 3
  };

  Face f2 = {
    lastVertex + 1, lastVertex + 2, lastVertex + 3,
    lastNormal + 1, lastNormal + 1, lastNormal + 1,
    lastUV + 1, lastUV + 2, lastUV + 3
  };

  this->faces.push_back(f1);
  this->faces.push_back(f2);
};


Loader::Loader() {
  this->object->name = "root";
};

BBoxf BBoxf::clone() {
  BBoxf cloned;

  cloned.min = this->min.clone();
  cloned.max = this->max.clone();

  return cloned;
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

bool BBoxf::intersect(Vector2f point) {
  bool intersectX = (point.x >= this->min.x) && (point.x <= this->max.x);
  bool intersectY = (point.y >= this->min.y) && (point.y <= this->max.y);

  return intersectX && intersectY;
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

bool BBoxf::intersectTriangle(glm::vec3 p1, glm::vec3 p2, glm::vec3 p3) {
  if (this->intersect(Vector3f::fromGLM(p1))) {// P1 is in box
    return true;
  }

  if (this->intersect(Vector3f::fromGLM(p2))) {// P2 is in box
    return true;
  }

  if (this->intersect(Vector3f::fromGLM(p3))) {// P3 is in box
    return true;
  }

  

  return false;
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

glm::vec3 BBoxf::getCenter() {
  glm::vec3 res;

  res.x = (this->min.x + this->max.x) / 2.0f;
  res.y = (this->min.y + this->max.y) / 2.0f;
  res.z = (this->min.z + this->max.z) / 2.0f;

  return res;
};

glm::vec3 BBoxf::getSize() {
  glm::vec3 res;

  res.x = this->max.x - this->min.x;
  res.y = this->max.y - this->min.y;
  res.z = this->max.z - this->min.z;

  return res;
};

void BBoxf::setCenter(glm::vec3 point) {
  glm::vec3 halfSize = this->getSize() * 0.5f;

  glm::vec3 min = point - halfSize;
  glm::vec3 max = point + halfSize;

  this->min = Vector3f::fromGLM(min);
  this->max = Vector3f::fromGLM(max);
};
void BBoxf::setSize(glm::vec3 size) {
  glm::vec3 center = this->getCenter();
  glm::vec3 halfSize = size * 0.5f;

  glm::vec3 min = center - halfSize;
  glm::vec3 max = center + halfSize;

  this->min = Vector3f::fromGLM(min);
  this->max = Vector3f::fromGLM(max);
};

void Vector2f::set(float x, float y) {
  this->x = x;
  this->y = y;
};

void Vector2f::set(Vector2f vector) {
  this->x = vector.x;
  this->y = vector.y;
};

glm::vec2 Vector2f::toGLM() {
  glm::vec2 result(this->x, this->y);

  return result;
};

Vector2f Vector2f::fromGLM(glm::vec2 vec) {
  Vector2f result = {vec.x, vec.y};

  return result;
};


float Vector3f::dot(Vector3f vector) {
  return (this->x * vector.x) + (this->y * vector.y) + (this->z * vector.z);
};

float Vector3f::length() {
  return sqrt(this->lengthSq());
};

float Vector3f::lengthSq() {
  return (this->x * this->x) + (this->y * this->y) + (this->z * this->z);
};

float Vector3f::angleTo(Vector3f vector) {
  float denominator = sqrt(this->lengthSq() * vector.lengthSq());
  if (denominator == 0.0f) {
    return M_PI * 0.5f;
  }

  float theta = this->dot(vector) / denominator;

  return acos(std::min(std::max(theta, -1.0f), 1.0f));
};

bool Vector3f::angleLess90(Vector3f vector) { 
  return (this->dot(vector) >= 0.0);
};

float Vector3f::distanceTo(Vector3f vector) {
  float dx = this->x - vector.x;
  float dy = this->y - vector.y;
  float dz = this->z - vector.z;

  return sqrt((dx*dx) + (dy*dy) + (dz*dz));
};

bool Vector3f::equals(Vector3f vector) {
  //return ( ( vector.x == this->x ) && ( vector.y == this->y ) && ( vector.z == this->z ) );
  return this->distanceTo(vector) < 0.0001f;
};

void Vector3f::divideScalar(float value) {
  this->x /= value;
  this->y /= value;
  this->z /= value;
};

void Vector3f::normalize() {
  float length = this->length();

  if (length != 0.0f) {
    this->divideScalar(length);
  } else {
    this->x = 0.0f;
    this->y = 1.0f;
    this->z = 0.0f;
  }
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

void Vector3f::sub(Vector3f vectorA, Vector3f vectorB) {
  this->x = vectorA.x - vectorB.x;
  this->y = vectorA.y - vectorB.y;
  this->z = vectorA.z - vectorB.z;
};

void Vector3f::set(float x, float y, float z) {
  this->x = x;
  this->y = y;
  this->z = z;
};

void Vector3f::set(Vector3f vector) {
  this->x = vector.x;
  this->y = vector.y;
  this->z = vector.z;
};

void Vector3f::cross(Vector3f vector) {
  float ax = this->x;
  float ay = this->y;
  float az = this->z;

  float bx = vector.x;
  float by = vector.y;
  float bz = vector.z;

  this->x = (ay * bz) - (az * by);
  this->y = (az * bx) - (ax * bz);
  this->z = (ax * by) - (ay * bx);
};

void Vector3f::lerp(Vector3f a, Vector3f b, float delta) {
  this->x = a.x + (b.x - a.x) * delta;
  this->y = a.y + (b.y - a.y) * delta;
  this->z = a.z + (b.z - a.z) * delta;
};

float Vector3f::deltaX(Vector3f a, Vector3f b, float x) {
  return (x - a.x) / (b.x - a.x);
};

float Vector3f::deltaY(Vector3f a, Vector3f b, float y) {
  return (y - a.y) / (b.y - a.y);
};

float Vector3f::deltaZ(Vector3f a, Vector3f b, float z) {
  return (z - a.z) / (b.z - a.z);
};

void Vector3f::lerpToX(Vector3f a, Vector3f b, float x) {
  this->lerp(a, b, Vector3f::deltaX(a, b, x));
};

void Vector3f::lerpToY(Vector3f a, Vector3f b, float y) {
  this->lerp(a, b, Vector3f::deltaY(a, b, y));
};

void Vector3f::lerpToZ(Vector3f a, Vector3f b, float z) {
  this->lerp(a, b, Vector3f::deltaZ(a, b, z));
};

glm::vec3 Vector3f::toGLM() {
  glm::vec3 result((float) this->x, (float) this->y, (float) this->z);

  return result;
};

Vector3f Vector3f::fromGLM(glm::vec3 vec) {
  Vector3f result = {(float) vec.x, (float) vec.y, (float) vec.z};

  return result;
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
    mesh->material->diffuseMapImage.free();
  });

  std::cout << "Memory has been cleaned" << std::endl;
};


Vertex::Vertex() {};
Vertex::Vertex(Vector3f vector) {
  this->position = vector.clone();
};

bool Vertex::hasNeighbor(VertexPtr vertex) {
  for (VertexPtr &target : this->neighbors) // access by reference to avoid copying
  {
    if (target->id == vertex->id) {
      return true;
    }
  }

  return false;
};

void Vertex::addUniqueNeighbor(VertexPtr vertex) {
  if (!this->hasNeighbor(vertex)) {
    this->neighbors.push_back(vertex);
  }
};

void Vertex::removeIfNonNeighbor(VertexPtr vertex) {
  if (this->hasNeighbor(vertex)) {
    bool isPartOfFace = false;

    for (TrianglePtr &target : this->faces) // access by reference to avoid copying
    {
      if (target->hasVertex(vertex)) {
        isPartOfFace = true;
        break;
      }
    }

    if (!isPartOfFace) {
      this->neighbors.erase(
        std::remove_if(
          this->neighbors.begin(), 
          this->neighbors.end(),
          [&](VertexPtr v){ return v->id == vertex->id; }
        ),
        this->neighbors.end()
      );
    }
  }
};

void Vertex::removeTriangle(TrianglePtr triangle) {
  this->faces.erase(
    std::remove_if(
      this->faces.begin(), 
      this->faces.end(),
      [&](TrianglePtr t){ return t->id == triangle->id; }
    ),
    this->faces.end()
  );
  // std::remove(this->faces.begin(), this->faces.end(), triangle),
};

Triangle::Triangle(VertexPtr v1, VertexPtr v2, VertexPtr v3, Face f) {
  this->v1 = v1;
  this->v2 = v2;
  this->v3 = v3;

  this->face = f;

  this->computeNormal();

  this->v1->addUniqueNeighbor(this->v2);
  this->v1->addUniqueNeighbor(this->v3);

  this->v2->addUniqueNeighbor(this->v1);
  this->v2->addUniqueNeighbor(this->v3);

  this->v3->addUniqueNeighbor(this->v1);
  this->v3->addUniqueNeighbor(this->v2);
};

void Triangle::init() {
  this->v1->faces.push_back(shared_from_this());
  this->v2->faces.push_back(shared_from_this());
  this->v3->faces.push_back(shared_from_this());
};

void Triangle::computeNormal() {
  Vector3f vA = this->v1->position;
  Vector3f vB = this->v2->position;
  Vector3f vC = this->v3->position;

  Vector3f _ab;
  Vector3f _cb;

  _cb.sub(vC, vB);
  _ab.sub(vA, vB);

  _cb.cross(_ab);

  if (_cb.length() != 0.0f) {
    _cb.normalize();
  }

  this->normal.set(_cb);
};

bool Triangle::hasVertex(VertexPtr vertex) {
  return this->v1->id == vertex->id || this->v2->id == vertex->id || this->v3->id == vertex->id;
};

void Triangle::replaceVertex(VertexPtr oldVertex, VertexPtr newVertex) {
  newVertex->geometricError += oldVertex->position.distanceTo(newVertex->position);

  if (oldVertex->id == this->v1->id) {
    this->v1 = newVertex;
  } else if (oldVertex->id == this->v2->id) {
    this->v2 = newVertex;
  } else if (oldVertex->id == this->v3->id) {
    this->v3 = newVertex;
  }

  oldVertex->removeTriangle(shared_from_this());
  newVertex->faces.push_back(shared_from_this());

  oldVertex->removeIfNonNeighbor(this->v1);
  this->v1->removeIfNonNeighbor(oldVertex);

  oldVertex->removeIfNonNeighbor(this->v2);
  this->v2->removeIfNonNeighbor(oldVertex);

  oldVertex->removeIfNonNeighbor(this->v3);
  this->v3->removeIfNonNeighbor(oldVertex);

  this->v1->addUniqueNeighbor(this->v2);
  this->v1->addUniqueNeighbor(this->v3);

  this->v2->addUniqueNeighbor(this->v1);
  this->v2->addUniqueNeighbor(this->v3);

  this->v3->addUniqueNeighbor(this->v1);
  this->v3->addUniqueNeighbor(this->v2);

  this->computeNormal();
};

Vec3Result math::clothestTrianglePoint(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  Vec3Result result;
  result.data = glm::vec3(0.0f);

  glm::vec3 diff1 = p - a;
  glm::vec3 diff2 = p - b;
  glm::vec3 diff3 = p - c;

  float distSq1 = glm::length(diff1);
  float distSq2 = glm::length(diff2);
  float distSq3 = glm::length(diff3);

  float min = std::min(distSq1, distSq2);
  min = std::min(min, distSq3);

  if (min == distSq1) {
    result.data = a;
  } else if (min == distSq2) {
    result.data = b;
  } else {
    result.data = c;
  }

  return result;
};

/**
 * Find the closest orthogonal projection of a point p onto a triangle given by three vertices
 * a, b and c. Returns either the projection point, or null if the projection is not within
 * the triangle.
 */
Vec3Result math::clothestTrianglePointOld(glm::vec3 p, glm::vec3 a, glm::vec3 b, glm::vec3 c) {
  Vec3Result result;
  result.data = glm::vec3(0.0f);
  // Find the normal to the plane: n = (b - a) x (c - a)
  // Vector3d n = b.sub(a, new Vector3d()).cross(c.sub(a, new Vector3d()));
  glm::vec3 n = glm::cross(b - a, c - a);

  // Normalize normal vector
  double nLen = glm::length(n);
  if (nLen < 1.0e-30) {
    result.hasData = false;
    return result;  // Triangle is degenerate
  } else {
    //n.mul(1.0f / nLen);
    n *= 1.0f / nLen;
  }

  //    Project point p onto the plane spanned by a->b and a->c.
  //
  //    Given a plane
  //
  //        a : point on plane
  //        n : *unit* normal to plane
  //
  //    Then the *signed* distance from point p to the plane
  //    (in the direction of the normal) is
  //
  //        dist = p . n - a . n
  //
  // double dist = p.dot(n) - a.dot(n);
  float dist = glm::dot(p, n) - glm::dot(a, n);

  // Project p onto the plane by stepping the distance from p to the plane
  // in the direction opposite the normal: proj = p - dist * n
  // Vector3d proj = p.add(n.mul(-dist, new Vector3d()), new Vector3d());
  glm::vec3 proj = p + (dist * n);// Negate normal

  // Find out if the projected point falls within the triangle -- see:
  // http://blackpawn.com/texts/pointinpoly/default.html

  // Compute edge vectors        
  double v0x = c.x - a.x;
  double v0y = c.y - a.y;
  double v0z = c.z - a.z;
  double v1x = b.x - a.x;
  double v1y = b.y - a.y;
  double v1z = b.z - a.z;
  double v2x = proj.x - a.x;
  double v2y = proj.y - a.y;
  double v2z = proj.z - a.z;

  // Compute dot products
  double dot00 = v0x * v0x + v0y * v0y + v0z * v0z;
  double dot01 = v0x * v1x + v0y * v1y + v0z * v1z;
  double dot02 = v0x * v2x + v0y * v2y + v0z * v2z;
  double dot11 = v1x * v1x + v1y * v1y + v1z * v1z;
  double dot12 = v1x * v2x + v1y * v2y + v1z * v2z;

  // Compute barycentric coordinates (u, v) of projection point
  double denom = (dot00 * dot11 - dot01 * dot01);
  if (std::abs(denom) < 1.0e-30) {
    result.hasData = false;
    return result; // Triangle is degenerate
  }
  double invDenom = 1.0 / denom;
  double u = (dot11 * dot02 - dot01 * dot12) * invDenom;
  double v = (dot00 * dot12 - dot01 * dot02) * invDenom;

  // Check barycentric coordinates
  bool inTriangle = (u >= 0.0) && (v >= 0.0) && (u + v < 1.0);
  /*
  if ((u >= 0) && (v >= 0) && (u + v < 1)) {
      // Nearest orthogonal projection point is in triangle
      return proj;
  } else {
      // Nearest orthogonal projection point is outside triangle
      result.hasData = false;
      return result;
  }
  */
  if (!inTriangle) {
    // Nearest orthogonal projection point is outside triangle
    result.hasData = false;
    return result;
  }

  // Nearest orthogonal projection point is in triangle
  result.data = proj;
  return result;
}

float math::triangleIntersection(glm::vec3 origin, glm::vec3 dir, glm::vec3 v0, glm::vec3 v1, glm::vec3 v2) {
  /*
  const float EPSILON = 0.0000001;
  float a,f,u,v;
  glm::vec3 edge1 = v1 - v0;
  glm::vec3 edge2 = v2 - v0;
  glm::vec3 h = glm::cross(dir, edge2);
  a = glm::dot(edge1, h);
  if (a > -EPSILON && a < EPSILON)
      return 0.0f;    // This ray is parallel to this triangle.
  f = 1.0f / a;
  glm::vec3 s = origin - v0;
  u = f * glm::dot(s, h);
  if (u < 0.0 || u > 1.0)
      return 0.0f;
  glm::vec3 q = glm::cross(s, edge1);
  v = f * glm::dot(dir, q);
  if (v < 0.0 || u + v > 1.0)
      return 0.0;
  // At this stage we can compute t to find out where the intersection point is on the line.
  float t = f * glm::dot(edge2, q);
  if (t > EPSILON) // ray intersection
  {
    // outIntersectionPoint = rayOrigin + rayVector * t;
    return t;
  }
  else // This means that there is a line intersection but not a ray intersection.
      return 0.0f;
  */

  glm::vec3 _edge1 = v1 - v0;
	glm::vec3 _edge2 = v2 - v0;
	glm::vec3 _normal = glm::cross( _edge1, _edge2 );

  float DdN = glm::dot( dir, _normal );
	float sign;

  if ( DdN > 0.0f ) {

    sign = 1.0f;

  } else if ( DdN < 0.0f ) {

    sign = - 1.0f;
    DdN = - DdN;

  } else {

    return 0.0f;

  }

  glm::vec3 _diff = origin - v0;

  float DdQxE2 = sign * glm::dot( dir, glm::cross( _diff, _edge2 ) );

  // b1 < 0, no intersection
  if ( DdQxE2 < 0.0f ) {

    return 0.0f;

  }

  float DdE1xQ = sign * glm::dot( dir, glm::cross( _edge1, _diff ) );

  // b2 < 0, no intersection
  if ( DdE1xQ < 0.0f ) {

    return 0.0f;

  }

  // b1+b2 > 1, no intersection
  if ( DdQxE2 + DdE1xQ > DdN ) {

    return 0.0f;

  }

  // Line intersects triangle, check if ray does.
  float QdN = - sign * glm::dot( _diff, _normal );

  // t < 0, no intersection
  if ( QdN < 0.0f ) {

    return 0.0f;

  }

  // Ray intersects triangle.
  return QdN / DdN;

};

float math::triangleIntersection(Vector3f origin, Vector3f dir, Vector3f v0, Vector3f v1, Vector3f v2) {
  return math::triangleIntersection(
    origin.toGLM(),
    dir.toGLM(),
    v0.toGLM(),
    v1.toGLM(),
    v2.toGLM()
  );
};
