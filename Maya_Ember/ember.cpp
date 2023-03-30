#include "ember.h"

using namespace ember;

EMBER::EMBER()
{
}

void EMBER::ReadMeshData(std::vector<std::vector<ivec3>> &vertices, std::vector<ivec3> &normals)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		polygons.push_back(new Polygon(getPlaneBasedPolygon(vertices[i], normals[i], meshId)));
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

	RefPoint refPoint;
	refPoint.pos = initBound.min;
	refPoint.WNV = std::vector<int>(meshId, 0);
	root->refPoint = refPoint;

	// Build BSP tree based on root node
	bspTree.Build(root);
}

void EMBER::AddPolygon(Polygon* p)
{
	polygons.push_back(p);
}