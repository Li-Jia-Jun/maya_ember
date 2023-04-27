#include "ember.h"
using namespace ember;

EMBER::EMBER()
{
}

void EMBER::ReadMeshData(std::vector<std::vector<ivec3>> &vertices, std::vector<ivec3> &normals)
{
	for (int i = 0; i < vertices.size(); i++)
	{
		polygons.push_back(ember::fromPositionNormal(vertices[i], normals[i], meshId));
	}

	// test
	//printStr("test point in polygon");
	//Point testPoint(0, BIG_NUM, 0, 1);
	//drawPosition(testPoint.getPosition());
	//for (int i = 0; i < polygons.size(); i++)
	//{
	//	Polygon* polygon = polygons[i];		
	//	for (int j = 0; j < polygon->bounds.size(); j++)
	//	{
	//		int c = classify(testPoint, polygon->bounds[j]);
	//		printNum(c);
	//	}
	//}

	//printStr("test2 point in polygon");
	//Point testPoint2(BIG_NUM * 2, 0, 0, 1);
	//drawPosition(testPoint2.getPosition());
	//for (int i = 0; i < polygons.size(); i++)
	//{
	//	Polygon* polygon = polygons[i];
	//	for (int j = 0; j < polygon->bounds.size(); j++)
	//	{
	//		int c = classify(testPoint2, polygon->bounds[j]);
	//		printNum(c);
	//	}
	//}

	meshId++;
}

void EMBER::SetInitBounds(AABB bound, AABB bound01, AABB bound02)
{
	initBound = bound;
	this->bound01 = bound01;
	this->bound02 = bound02;

	bspTree.SetMeshBounds(bound01, bound02);
}

void EMBER::SetMode(int mode)
{
	this->mode = mode;
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
	bspTree.Build(root, mode);

	//printStr("thread count = ");
	//printNum(std::thread::hardware_concurrency());
}