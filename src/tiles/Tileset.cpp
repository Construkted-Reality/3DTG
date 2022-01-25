#include "./Tileset.h"

Tileset::Tileset(IdGenerator::ID rootId) {
  this->root = std::make_shared<Tile>();
  this->root->id = rootId;
  this->root->refine = TileRefine::REPLASE;

  /*
  this->root->transform = glm::mat4(
    1.0f, 0.0f, 0.0f, 0.0f,
    0.0f, 0.0f, -1.0f, 0.0f,
    0.0f, 1.0f, 0.0f, 0.0f,
    0.0f, 0.0f, 0.0f, 1.0f
  );
  */
};

void Tileset::traverse(Tile::TileCallback fn) {
  fn(this->root);
  this->root->traverse(fn);
};

void Tileset::setRootGeometricError(float error) {
  this->root->geometricError = error;
  this->geometricError = this->root->geometricError;
};

void Tileset::computeRootGeometricError() {
  unsigned int i = 0;
  for (std::shared_ptr<Tile> &child : this->root->children) {
    if (i == 0) {
      this->root->geometricError = child->geometricError;
    } else {
      this->root->geometricError += child->geometricError;
    }

    i++;
  }

  if (i > 0) {
    this->root->geometricError /= (float) i;
  }

  this->geometricError = this->root->geometricError;
};

void Tileset::computeRootBoundingVolume() {
  this->root->boundingVolume = std::make_shared<TileBoundingVolume>();
  this->root->boundingVolume->box = std::make_shared<TileBoundingBox>();

  unsigned int i = 0;
  for (std::shared_ptr<Tile> &child : this->root->children) {
    if (i == 0) {
      this->root->boundingVolume->box->center = child->boundingVolume->box->center;
      this->root->boundingVolume->box->xHalf = child->boundingVolume->box->xHalf;
      this->root->boundingVolume->box->yHalf = child->boundingVolume->box->yHalf;
      this->root->boundingVolume->box->zHalf = child->boundingVolume->box->zHalf;
    } else {
      this->root->boundingVolume->box->center += child->boundingVolume->box->center;
      this->root->boundingVolume->box->xHalf += child->boundingVolume->box->xHalf;
      this->root->boundingVolume->box->yHalf += child->boundingVolume->box->yHalf;
      this->root->boundingVolume->box->zHalf += child->boundingVolume->box->zHalf;
    }

    i++;
  }

  if (i > 0) {
    this->root->boundingVolume->box->center /= (float) i;
    this->root->boundingVolume->box->xHalf /= (float) i;
    this->root->boundingVolume->box->yHalf /= (float) i;
    this->root->boundingVolume->box->zHalf /= (float) i;
  }
};

std::shared_ptr<Tile> Tileset::findTileById(IdGenerator::ID id) {
  if (this->root->id == id) {
    return this->root;
  }

  return this->root->findTileById(id);
};

nlohmann::json Tileset::toJSON() {
  nlohmann::json result;

  result["asset"] = nlohmann::json::object();
  result["asset"]["version"] = this->asset.version;

  result["geometricError"] = (double) this->geometricError;

  result["root"] = this->root->toJSON();

  return result;
};
