#include "ember.h"

using namespace ember;

EMBER::EMBER()
{
}

void EMBER::ReadMeshData(std::vector<std::vector<ivec3>> &vertices, std::vector<ivec3> &normals)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		polygons.push_back(Polygon::fromPositionNormal(vertices[i], normals[i], meshId));
	}
	meshId++;
}

void EMBER::SetInitBound(AABB bound)
{
	initBound = bound;
}

void EMBER::BuildBSPTree()
{
	// Build root node
	BSPNode *root = new BSPNode();
	root->bound = initBound;
	root->polygons = polygons;

	// Create initial ref point in the min corner of AABB  
	RefPoint refPoint;
	refPoint.pos = initBound.min;
	refPoint.WNV = std::vector<int>(meshId, 0);
	root->refPoint = refPoint;

	// Build BSP tree based on root node
	bspTree.Build(root);
}