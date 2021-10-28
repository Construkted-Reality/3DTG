#ifndef __TILE_H__
#define __TILE_H__

#include <memory>
#include <string>
#include <vector>
#include <functional>

#include <json/json.hpp>

#include "./TileBoundingVolume.h"
#include "./TileContent.h"
#include "./TileStructures.h"

#include "./../helpers/IdGenerator.h"

class Tile {
  public:
    typedef std::function<void (std::shared_ptr<Tile>)> TileCallback;
    typedef std::function<bool (std::shared_ptr<Tile>)> TileSearchCallback;

    std::shared_ptr<TileBoundingVolume> boundingVolume = NULL;// Required
    float geometricError = 0.0f;// Required
    Refine refine = NULL;
    std::shared_ptr<TileContent> content = NULL;
    std::vector<std::shared_ptr<Tile>> children;

    IdGenerator::ID id;
    void traverse(TileCallback fn);
    std::shared_ptr<Tile> search(TileSearchCallback fn);
    std::shared_ptr<Tile> findTileById(IdGenerator::ID id);
    nlohmann::json toJSON();
};

#endif // __TILE_H__