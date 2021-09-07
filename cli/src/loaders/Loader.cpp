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

void Group::free() {
  this->traverse([&](MeshObject mesh){
    mesh->free();
  });

  this->meshes.clear();
  this->children.clear();
};

void Mesh::finish() {
  this->hasNormals = this->normal.size() > 0;
  this->hasUVs = this->uv.size() > 0;

  // std::cout << "Normals length: " << this->normal.size() << std::endl;

  // std::cout << "Loading finished: " << this->name << std::endl;
};

void Mesh::free() {
  this->uv.clear();
  this->position.clear();
  this->normal.clear();
  this->faces.clear();
  this->material.name = "";

  if (this->material.diffuseMapImage.data != NULL) {
    this->material.diffuseMapImage.free();
  }
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
    positionMap[face.positionIndices[0]] = position[face.positionIndices[0]];
    positionMap[face.positionIndices[1]] = position[face.positionIndices[1]];
    positionMap[face.positionIndices[2]] = position[face.positionIndices[2]];

    if (this->hasNormals) {
      normalMap[face.normalIndices[0]] = normal[face.normalIndices[0]];
      normalMap[face.normalIndices[1]] = normal[face.normalIndices[1]];
      normalMap[face.normalIndices[2]] = normal[face.normalIndices[2]];
    }

    if (this->hasUVs) {
      uvMap[face.uvIndices[0]] = uv[face.uvIndices[0]];
      uvMap[face.uvIndices[1]] = uv[face.uvIndices[1]];
      uvMap[face.uvIndices[2]] = uv[face.uvIndices[2]];
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

void Vector2f::set(float x, float y) {
  this->x = x;
  this->y = y;
};

void Vector2f::set(Vector2f vector) {
  this->x = vector.x;
  this->y = vector.y;
};


float Vector3f::dot(Vector3f vector) {
  return (this->x * vector.x) + (this->y * vector.y) + (this->z * vector.z);
};

float Vector3f::length() {
  return sqrt((this->x * this->x) + (this->y * this->y) + (this->z * this->z));
};

float Vector3f::distanceTo(Vector3f vector) {
  float dx = this->x - vector.x;
  float dy = this->y - vector.y;
  float dz = this->z - vector.z;

  return sqrt((dx*dx) + (dy*dy) + (dz*dz));
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


