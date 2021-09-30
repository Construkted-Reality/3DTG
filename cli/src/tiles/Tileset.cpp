#include "./Tileset.h"

Tileset::Tileset(IdGenerator::ID rootId) {
  this->root = std::make_shared<Tile>();
  this->root->id = rootId;
};

void Tileset::traverse(Tile::TileCallback fn) {
  fn(this->root);
  this->root->traverse(fn);
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

  result["geometricError"] = this->geometricError;

  result["root"] = this->root->toJSON();

  return result;
};
