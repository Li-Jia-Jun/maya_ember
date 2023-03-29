#include "ember.h"

using namespace ember;

EMBER::EMBER()
{
}

void EMBER::ReadMeshDataFromTriangles(std::vector<std::vector<ivec3>> &vertices, std::vector<ivec3> &normals)
{
	// Add all triangles to polygon soup
	for (int i = 0; i < vertices.size(); i++)
	{
		std::vector<ivec3> pos{vertices[i][0], vertices[i][1], vertices[i][2]}; 
		polygons.push_back(new Polygon(getPlaneBasedPolygonFromTriangle(pos, normals[i], meshId)));
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