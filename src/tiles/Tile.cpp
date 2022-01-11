#include "./Tile.h"

void Tile::traverse(TileCallback fn) {
  for (std::shared_ptr<Tile> &child : this->children) {
    fn(child);
    child->traverse(fn);
  }
};

std::shared_ptr<Tile> Tile::search(TileSearchCallback fn) {
  std::shared_ptr<Tile> result = NULL;

  for (std::shared_ptr<Tile> &child : this->children) {
    if (fn(child)) {
      return child;
    } else {
      result = child->search(fn);

      if (result != NULL) {
        return result;
      }
    }
  }

  return result;
};

std::shared_ptr<Tile> Tile::findTileById(IdGenerator::ID id) {
  return this->search([&id](std::shared_ptr<Tile> tile){
    return tile->id == id;
  });
};

nlohmann::json Tile::toJSON() {
  nlohmann::json result;

  if (this->boundingVolume != NULL) {
    result["boundingVolume"] = nlohmann::json::object();

    if (this->boundingVolume->box != NULL) {
      result["boundingVolume"]["box"] = this->boundingVolume->box->toArray();
    } else if (this->boundingVolume->sphere != NULL) {
      result["boundingVolume"]["sphere"] = this->boundingVolume->sphere->toArray();
    } else if (this->boundingVolume->region != NULL) {
      result["boundingVolume"]["region"] = this->boundingVolume->region->toArray();
    } else {
      // Throw error
    }
  }

  std::vector<float> transformValues;
  // transformValues.reserve(16);

  for (unsigned int i = 0; i < 4; i++) {
    // transformValues[(i * 4)] = this->transform[i].x;
    // transformValues[(i * 4) + 1] = this->transform[i].y;
    // transformValues[(i * 4) + 2] = this->transform[i].z;
    // transformValues[(i * 4) + 3] = this->transform[i].w;

    transformValues.push_back(this->transform[i].x);
    transformValues.push_back(this->transform[i].y);
    transformValues.push_back(this->transform[i].z);
    transformValues.push_back(this->transform[i].w);
  }
  result["transform"] = transformValues;


  result["geometricError"] = (this->geometricError > 0.0f) ? (double) this->geometricError : (double) 0.0;

  if (this->refine != NULL) {
    result["refine"] = this->refine;
  }

  if (this->content != NULL) {
    result["content"] = nlohmann::json::object();

    result["content"]["uri"] = this->content->uri;
  }

  std::vector<nlohmann::json> childData;

  for (std::shared_ptr<Tile> &child : this->children) {
    childData.push_back(child->toJSON());
  }

  if (childData.size() > 0) {
    result["children"] = childData;
  }

  return result;
};


/*
struct Tile {
  typedef std::function<void (std::shared_ptr<Tile>)> TileCallback;
  typedef std::function<bool (std::shared_ptr<Tile>)> TileSearchCallback;

  std::shared_ptr<TileBoundingVolume> boundingVolume = NULL;
  float geometricError = 0.0f;
  Refine refine = NULL;
  std::shared_ptr<TileContent> content = NULL;
  std::vector<std::shared_ptr<Tile>> children;

  IdGenerator::ID id;
  void traverse(TileCallback fn);
  void traverseSearch(TileSearchCallback fn);
  std::shared_ptr<Tile> findTileById(IdGenerator::ID id);
  nlohmann::json toJSON();
};
*/