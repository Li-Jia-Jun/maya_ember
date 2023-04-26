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

	meshId++;
}

void EMBER::SetInitBounds(AABB bound, AABB bound01, AABB bound02)
{
	initBound = bound;
	this->bound01 = bound01;
	this->bound02 = bound02;

	bspTree.SetMeshBounds(bound01, bound02);
}

void EMBER::BuildBSPTree()
{
	//// Temp test code
	//drawBoundingBox(polygons[0]->aabb);
	//drawBoundingBox(polygons[1]->aabb);
	//std::vector<int> WNV{ 0, 0 };
	//Segment segment{ Line{Plane(0, 1, 0, 0), Plane(1, 0, 0, 0)}, Plane(0, 0, -1, BigInt("5100000000000000")),  Plane(0, 0, 1, 0) };
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
	//printNum(WNV[1]);


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




	//Plane support= Plane{ BigInt("-98768800000000"), BigInt("7409900000000"), BigInt("-13777100000000"), BigInt("3317718840710000000000000000") };
	//std::vector<Plane> bounds;
	//bounds.push_back(Plane{ BigInt("162450000000000000000"), BigInt("-925565669070000000000000000"), BigInt("-497809052430000000000000000"), BigInt("105094497538729380000000000000000000000000") });
	//bounds.push_back(Plane{ BigInt("-3128674205600000000000000000"), BigInt("-9356921529280000000000000000"), BigInt("17397096924480000000000000000"), BigInt("105094669505468480000000000000000000000000") });
	//bounds.push_back(Plane{ BigInt("0"), BigInt("1"), BigInt("0"), BigInt("0") });
	//bounds.push_back(Plane{ BigInt("3128674842320000000000000000"), BigInt("9356911652400000000000000000"), BigInt("-17397106801360000000000000000"), BigInt("105095695574291376000000000000000000000000") });
	//Polygon* parent = new Polygon(0, support, bounds);
	//drawPolygon(parent);
	//auto pairs = splitPolygon(parent, Plane{ 0, 0, 1, 0 });
	//if (pairs.first != nullptr) { drawPolygon(pairs.first); printPolygon(pairs.first); }
	//if(pairs.second != nullptr) drawPolygon(pairs.second);

	//Point p1 = Point{BigInt("-59166576011249081815388800000000000000000000000000000000"), 0, BigInt("13035089203017448000000000000000000000000000000000"), BigInt("1761394444112551984000000000000000000000000")};
	//Point p2 = Point{BigInt("-31822375840305852810940800000000000000000000000000000000"), BigInt("-13035089203017448000000000000000000000000000000000"), 0, BigInt("947355074137225904000000000000000000000000")};
	//if (isPointEqual(p1, p2))
	//{
	//	printStr("point equal");
	//}
	//else
	//{
	//	printStr("not equal");
	//}

	//support = Plane{ BigInt("-98768800000000"), BigInt("7409900000000"), BigInt("-13777100000000"), BigInt("3317718840710000000000000000") };
	//bounds.clear();
	//bounds.push_back(Plane{ BigInt("0"), BigInt("0"), BigInt("-1"), BigInt("0") });
	//bounds.push_back(Plane{ BigInt("-3128674205600000000000000000"), BigInt("-9356921529280000000000000000"), BigInt("17397096924480000000000000000"), BigInt("105094669505468480000000000000000000000000") });
	//bounds.push_back(Plane{ BigInt("0"), BigInt("1"), BigInt("0"), BigInt("0") });
	//Polygon* p = new Polygon(0, support, bounds);
	// 
	//auto pairs = splitPolygon(p, Plane{ 1, 0, 0, 1 });
	//printPolygon(p);
	//drawPolygon(p);
}