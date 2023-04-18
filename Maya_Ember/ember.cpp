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

void EMBER::SetInitBound(AABB bound)
{
	initBound = bound;
}

void EMBER::BuildBSPTree()
{
	// Temp test code
	//std::vector<int> WNV{ 0, 0 };
	//Segment segment{ Line{Plane(0, 1, 0, 0), Plane(0, 0, 1, 0)}, Plane(-1, 0, 0, BigInt(BIG_NUM_STR)),  Plane(1, 0, 0, 0) };
	//Point st = intersect(segment.line.p1, segment.line.p2, segment.bound1);
	//Point ed = intersect(segment.line.p1, segment.line.p2, segment.bound2);
	//if (st.isValid() && ed.isValid())
	//{
	//	drawPosition(st.getPosition());
	//	drawPosition(ed.getPosition());
	//}
	//WNV = TraceSegment(polygons, segment, WNV);
	//printStr("trace result");
	//printNum(WNV[0]);



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