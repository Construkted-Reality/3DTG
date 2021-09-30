#ifndef __TILESET_H__
#define __TILESET_H__

#include <functional>
#include <memory>

#include <json/json.hpp>

#include "./../helpers/IdGenerator.h"
#include "./TileAsset.h"
#include "./Tile.h"


class Tileset {
  public:
    Tileset(IdGenerator::ID rootId);
    

    TileAsset asset;// Required
    float geometricError = 0.0f;// Required
    std::shared_ptr<Tile> root;// Required

    void traverse(Tile::TileCallback fn);
    void computeRootGeometricError();
    std::shared_ptr<Tile> search(Tile::TileSearchCallback fn);
    std::shared_ptr<Tile> findTileById(IdGenerator::ID id);
    nlohmann::json toJSON();
};

#endif // __TILESET_H__