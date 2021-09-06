#ifndef __SIMPLIFIER_H__
#define __SIMPLIFIER_H__

#include <vector>
#include <map>
#include <string>
#include <variant>
#include <functional>
#include <memory>
#include <cstdlib>
#include <cmath>
#include <algorithm>

#include "./../loaders/Loader.h"

/**
 * This was used - http://www.melax.com/polychop/
 * Thus should be used to fix UVs decimation - https://github.com/mrdoob/three.js/issues/5806
 * Implementation - https://codesandbox.io/s/23p6j1ow9j?file=/simplifyModifier.js:337-367
 */
namespace simplifier {
  static constexpr unsigned int lowerLimit = 50;
  // GroupObject modify(GroupObject &group, unsigned int verticesToRemove);
  GroupObject modify(GroupObject &group, float verticesCountModifier);
  MeshObject modify(MeshObject &mesh, float verticesCountModifier);
  float computeEdgeCollapseCost(VertexPtr u, VertexPtr v);
  void computeEdgeCostAtVertex(VertexPtr v);
  void removeVertex(VertexPtr v, std::vector<VertexPtr> &vertices);
  void removeFace(TrianglePtr f, std::vector<TrianglePtr> &faces);
  void collapse(std::vector<VertexPtr> &vertices, std::vector<TrianglePtr> &faces, VertexPtr u, VertexPtr v);
  VertexPtr minimumCostEdge(std::vector<VertexPtr> &vertices);
}

#endif // __SIMPLIFIER_H__