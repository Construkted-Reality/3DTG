#include "./simplifier.h"

GroupObject simplifier::modify(GroupObject &group, float verticesCountModifier) {
  GroupObject result = GroupObject(new Group());
  result->name = group->name;

  group->traverse([&](MeshObject mesh){
    MeshObject target = simplifier::modify(mesh, verticesCountModifier);

    // result->geometricError += target->geometricError;
    result->meshes.push_back(target);
  });

  // result->geometricError /= float(result->meshes.size());

  return result;
};

MeshObject simplifier::modify(MeshObject &mesh, float verticesCountModifier) {
  if (mesh->position.size() < simplifier::lowerLimit) {
    // return mesh;
  }

  MeshObject resultMesh = MeshObject(new Mesh());
  resultMesh->material = mesh->material;
  resultMesh->name = mesh->name;
  // resultMesh->material->diffuseMapImage = mesh->material->diffuseMapImage;

  // unsigned int count = floor(float(mesh->position.size()) * verticesCountModifier);
  // count = count + (3 - (count % 3));

  // if (count > mesh->position.size()) {
  //   count = mesh->position.size();
  // }

  std::vector<VertexPtr> vertices;
  std::map<unsigned int, VertexPtr> verticesMap;
  std::vector<TrianglePtr> faces;

  unsigned int faceId = 0;

  
  for (Face face : mesh->faces) {
    for (int i = 0; i < 3; i++) {
      if (verticesMap.find(face.positionIndices[i]) == verticesMap.end()) {
        VertexPtr targetVertex = VertexPtr( new Vertex(mesh->position[face.positionIndices[i]]) );
        if (mesh->hasNormals) { targetVertex->normal = mesh->normal[face.normalIndices[i]]; }
        if (mesh->hasUVs)     { targetVertex->uv = mesh->uv[face.uvIndices[i]]; }

        targetVertex->id = face.positionIndices[i];

        vertices.push_back(targetVertex);
        verticesMap[face.positionIndices[i]] = targetVertex;
      }
    }

    TrianglePtr targetFace = TrianglePtr(
      new Triangle(
        verticesMap[face.positionIndices[0]],
        verticesMap[face.positionIndices[1]],
        verticesMap[face.positionIndices[2]],
        face
      )
    );
    targetFace->init();
    targetFace->id = faceId;
    faceId++;
    
    faces.push_back(targetFace);
  }


  for (VertexPtr &vertex : vertices) {
    simplifier::computeEdgeCostAtVertex(vertex);
  }

  // std::cout << "Vertex sorting started" << std::endl;
  // std::sort(vertices.begin(), vertices.end(), [](VertexPtr &lx, VertexPtr &rx){
  //   return lx->collapseCost < rx->collapseCost;
  // });

  for (unsigned int i = 0; i < vertices.size(); i++) {
    vertices[i]->positionId = i;
  }
  // std::cout << "Vertex sorting finished" << std::endl;

  

  VertexPtr nextVertex;

  nextVertex = simplifier::minimumCostEdge(vertices);

  // std::cout << "Mesh simplification started" << std::endl;

  // unsigned int processed = 0;

  while (nextVertex != NULL && nextVertex->collapseCost < verticesCountModifier) { // 50.0f //  && nextVertex->collapseCost < verticesCountModifier
    // std::cout << "Vertex cost: " << nextVertex->collapseCost << std::endl;
    // std::cout << "Processing, target cost: " << nextVertex->collapseCost << std::endl;

    simplifier::collapse(vertices, faces, nextVertex, nextVertex->collapseNeighbor);

    nextVertex = simplifier::minimumCostEdge(vertices);
    // processed++;

    // if ((processed % 100) == 0)
    // std::cout << "Vertices processed: " << processed << std::endl;
  }

  // std::cout << "Mesh simplification has been finished" << std::endl;

  resultMesh->geometricError = 0.0f;

  for (unsigned int i = 0; i < vertices.size(); i++) {
    resultMesh->position.push_back(vertices[i]->position);

    if (mesh->hasNormals) {
      resultMesh->normal.push_back(vertices[i]->normal);
    }
    if (mesh->hasUVs) {
      resultMesh->uv.push_back(vertices[i]->uv);
    }

    resultMesh->geometricError += vertices[i]->geometricError;
    // resultMesh->geometricError = std::max(vertices[i]->geometricError, resultMesh->geometricError);

    //vertices[i].id = i;
    vertices[i]->positionId = i;
  }

  if (vertices.size() > 0) {
    resultMesh->geometricError /= float(vertices.size());
  } else {
    resultMesh->geometricError = 0.0f;
  }

  for (unsigned int i = 0; i < faces.size(); i++) {
    Face face;

    face.positionIndices[0] = faces[i]->v1->positionId;
    face.positionIndices[1] = faces[i]->v2->positionId;
    face.positionIndices[2] = faces[i]->v3->positionId;

    if (mesh->hasNormals) {
      face.normalIndices[0] = faces[i]->v1->positionId;
      face.normalIndices[1] = faces[i]->v2->positionId;
      face.normalIndices[2] = faces[i]->v3->positionId;
    }
    if (mesh->hasUVs) {
      face.uvIndices[0] = faces[i]->v1->positionId;
      face.uvIndices[1] = faces[i]->v2->positionId;
      face.uvIndices[2] = faces[i]->v3->positionId;
    }

    resultMesh->faces.push_back(face);
  }

  resultMesh->finish();

  vertices.clear();
  faces.clear();

  resultMesh->computeBoundingBox();

  return resultMesh;
};

float simplifier::computeEdgeCollapseCost(VertexPtr u, VertexPtr v) {
  float edgeLength = glm::distance(v->position, u->position);//v->position.distanceTo(u->position);
  float curvature  = 0.0f;


  std::vector<TrianglePtr> sideFaces;

  for (TrianglePtr &face : u->faces) // access by reference to avoid copying
  {
    if (face->hasVertex(v)) {
      sideFaces.push_back(face);
    }
  }

  for (TrianglePtr &face : u->faces) // access by reference to avoid copying
  {
    float minCurvature = 1.0f;
    
    for (TrianglePtr &sideFace : sideFaces) // access by reference to avoid copying
    {
      float dot = glm::dot(face->normal, sideFace->normal);//face->normal.dot(sideFace->normal);
      minCurvature = std::min(minCurvature, (1.001f - dot) / 2.0f);
    }

    curvature = std::max(curvature, minCurvature);
  }

  if (sideFaces.size() < 2) {
    // curvature = 1.0f;
    return 999999.0f;
  }
  

  return edgeLength * curvature;
};

void simplifier::computeEdgeCostAtVertex(VertexPtr v) {
  if (v->neighbors.size() == 0) {
    v->collapseNeighbor = NULL;
    v->collapseCost = -0.01f;
  } else {
    v->collapseCost = 100000.0f;
	  v->collapseNeighbor = NULL;

    for (VertexPtr &vertex : v->neighbors) // access by reference to avoid copying
    {
      float collapseCost = simplifier::computeEdgeCollapseCost(v, vertex);

      if (v->collapseNeighbor == NULL) {
        v->collapseNeighbor = vertex;
        v->collapseCost = collapseCost;
        v->minCost = collapseCost;
        v->totalCost = 0.0f;
        v->costCount = 0;
      }

      v->costCount ++;
      v->totalCost += collapseCost;

      if (collapseCost < v->minCost) {
        v->collapseNeighbor = vertex;
        v->minCost = collapseCost;
      }
    }

    v->collapseCost = v->totalCost / float(v->costCount);
  }
};

void simplifier::removeVertex(VertexPtr v, std::vector<VertexPtr> &vertices) {
  while (v->neighbors.size()) {
    VertexPtr last = v->neighbors.back();
    v->neighbors.pop_back();

    last->neighbors.erase(
      std::remove_if(
        last->neighbors.begin(), 
        last->neighbors.end(),
        [&](VertexPtr vertex){return vertex->id == v->id;}
      ),
      last->neighbors.end()
    );

    
  }

  vertices.erase(
    std::remove_if(
      vertices.begin(), 
      vertices.end(),
      [&](VertexPtr vertex){return vertex->id == v->id;}
    ),
    vertices.end()
  );
};

void simplifier::removeFace(TrianglePtr f, std::vector<TrianglePtr> &faces) {
  faces.erase(
    std::remove_if(
      faces.begin(),
      faces.end(),
      [&](TrianglePtr triangle){return triangle->id == f->id;}
    ),
    faces.end()
  );

  if (f->v1 != NULL) {
    f->v1->faces.erase(
      std::remove_if(
        f->v1->faces.begin(), 
        f->v1->faces.end(),
        [&](TrianglePtr triangle){return triangle->id == f->id;}
      ),
      f->v1->faces.end()
    );
  }

  if (f->v2 != NULL) {
    f->v2->faces.erase(
      std::remove_if(
        f->v2->faces.begin(), 
        f->v2->faces.end(),
        [&](TrianglePtr triangle){return triangle->id == f->id;}
      ),
      f->v2->faces.end()
    );
  }

  if (f->v3 != NULL) {
    f->v3->faces.erase(
      std::remove_if(
        f->v3->faces.begin(),
        f->v3->faces.end(),
        [&](TrianglePtr triangle){return triangle->id == f->id;}
      ),
      f->v3->faces.end()
    );
  }

  // TODO optimize this!
	std::vector<VertexPtr> vs = { f->v1, f->v2, f->v3 };

	for ( int i = 0; i < 3; i ++ ) {

		VertexPtr v1 = vs[ i ];
		VertexPtr v2 = vs[ ( i + 1 ) % 3 ];

		if ( v1 == NULL || v2 == NULL ) continue;

		v1->removeIfNonNeighbor( v2 );
		v2->removeIfNonNeighbor( v1 );

	}

  /*
  f->v1->removeIfNonNeighbor(f->v2);
  f->v2->removeIfNonNeighbor(f->v1);

  f->v2->removeIfNonNeighbor(f->v3);
  f->v3->removeIfNonNeighbor(f->v2);

  f->v3->removeIfNonNeighbor(f->v1);
  f->v1->removeIfNonNeighbor(f->v3);
  */
};

void simplifier::resortVertex(std::vector<VertexPtr> &vertices, VertexPtr &v, float oldCost) {
  // positionId
  unsigned int i = v->positionId;

  if (oldCost > v->collapseCost) {
    // New cost is smaller, move closer to start
    i -= 1;// Take prev
    while (i >= 0) {
      if (vertices[i]->collapseCost < v->collapseCost) {
        if ((i + 1) != v->positionId) {
          unsigned int index = v->positionId;

          vertices[i + 1]->positionId = v->positionId;
          v->positionId = i + 1;

          std::swap(vertices[index], vertices[i + 1]);
        }

        break;
      }

      i--;
    }
  } else if (oldCost > v->collapseCost) {
    // New cost is greater, move far from start
    i += 1;// Take next
    while (i < vertices.size()) {
      if (vertices[i]->collapseCost > v->collapseCost) {
        if ((i - 1) != v->positionId) {
          unsigned int index = v->positionId;

          vertices[i - 1]->positionId = v->positionId;
          v->positionId = i - 1;

          std::swap(vertices[index], vertices[i - 1]);
        }

        break;
      }

      i++;
    }
  }
  // Else nothing changed 
};

void simplifier::collapse(std::vector<VertexPtr> &vertices, std::vector<TrianglePtr> &faces, VertexPtr u, VertexPtr v) {
  if (v == NULL) {
    simplifier::removeVertex(u, vertices);
  } else {
    std::vector<VertexPtr> tmpVertices;

    for (VertexPtr &vertex : u->neighbors) // access by reference to avoid copying
    {
      tmpVertices.push_back(vertex);
    }

    // delete triangles on edge uv:
    for (int i = u->faces.size() - 1; i >= 0; i --) {
      if (u->faces[ i ]->hasVertex(v)) {
        simplifier::removeFace(u->faces[ i ], faces);
      }
    }

    // update remaining triangles to have v instead of u
    for (int i = u->faces.size() - 1; i >= 0; i --) {
      // if (u->faces[ i ]->hasVertex(v)) {
        u->faces[ i ]->replaceVertex(u, v);
      // }
    }

    // float oldVCost = v->collapseCost;
    // simplifier::resortVertex(vertices, v, oldVCost);

    simplifier::removeVertex(u, vertices);


    // recompute the edge collapse costs in neighborhood
    for (VertexPtr &vertex : tmpVertices) {
      // float oldCost = vertex->collapseCost;
      simplifier::computeEdgeCostAtVertex(vertex); // HERE might be a bug of copying
      // simplifier::resortVertex(vertices, vertex, oldCost);
    }
  }
};

VertexPtr simplifier::minimumCostEdge(std::vector<VertexPtr> &vertices) {
  if (vertices.size() == 0) return NULL;

  VertexPtr least = vertices[0];

  for (VertexPtr &vertex : vertices) // access by reference to avoid copying
  {
    if ( vertex->collapseCost < least->collapseCost ) {
			least = vertex;
		}
  }

  return least;
};