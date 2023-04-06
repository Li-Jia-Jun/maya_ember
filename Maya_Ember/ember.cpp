#include "ember.h"
using namespace ember;

EMBER::EMBER()
{
}

void EMBER::ReadMeshData(std::vector<std::vector<ivec3>> &vertices, std::vector<ivec3> &normals)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		// LEO::changed this from Polygon::fromPositionNormal to ember::fromPositionNormal

		//for (int j = 0; j < vertices[i].size(); j++)
		//{
		//	printIvec3(vertices[i][j]);
		//}
		// printIvec3(normals[i]);
		polygons.push_back(ember::fromPositionNormal(vertices[i], normals[i], meshId));
	}
	meshId++;

	//for (int i = 0; i < polygons.size(); i++)
	//{
	//	printPolygon(*polygons[i]);
	//}
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