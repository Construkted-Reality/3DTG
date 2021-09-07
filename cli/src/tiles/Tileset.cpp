#include "./Tileset.h"

Tileset::Tileset(IdGenerator::ID rootId) {
  this->root = std::make_shared<Tile>();
  this->root->id = rootId;
};

void Tileset::traverse(Tile::TileCallback fn) {
  fn(this->root);
  this->root->traverse(fn);
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

/*
class Tileset {
  public:
    typedef std::function<void (Tile)> TileCallback;

    TileAsset asset;
    float geometricError = 0.0f;
    std::shared_ptr<Tile> root = std::make_shared<Tile>();

    void traverse(TileCallback fn);
    std::shared_ptr<Tile> findTileById(IdGenerator::ID id);
    nlohmann::json toJSON();
};
*/
