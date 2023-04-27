#include "bsp.h"
#include <cstdlib>
#include <time.h>

#include <ctime>
#include <sstream>

using namespace ember;


// ========== BSP ============

BSPTree::BSPTree()
{

}

BSPTree::~BSPTree()
{
	// TODO:: Recursively delete tree node 
}

void BSPTree::SetMeshBounds(AABB bound01, AABB bound02)
{
	this->bound01 = bound01;
	this->bound02 = bound02;
}

void BSPTree::Build(BSPNode* rootNode, int mode)
{
	this->mode = mode;

	if (mode < 0)
		return;

	nodes.clear();
	nodes.push_back(rootNode);

	clock_t start = std::clock();

	// Create tree nodes recursively (in breadth first order)
	std::queue<BSPNode*> toTraverse;
	toTraverse.push(rootNode);
	int iterCount = 0;
	while (!toTraverse.empty() && iterCount < GLOBAL_BSP_NODE_COUNT)
	{	
		BSPNode* node = toTraverse.front();
		toTraverse.pop();

		// Leaf node determination
		if (node->polygons.size() <= LEAF_POLYGON_COUNT)
			continue;

		// Split AABB
		Split(node);

		// Collect new nodes
		if (node->leftChild != nullptr)
		{
			toTraverse.push(node->leftChild);
			nodes.push_back(node->leftChild);
		}
		if (node->rightChild != nullptr)
		{
			toTraverse.push(node->rightChild);
			nodes.push_back(node->rightChild);
		}
		++iterCount;
	}
	printStr("Global BSP Construction Done");

	// Gather leaf nodes
	std::vector<BSPNode*> leaves;
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->leftChild == nullptr && nodes[i]->rightChild == nullptr)
			leaves.push_back(nodes[i]);
	}

	//// Draw leaf nodes
	//for (int i = 0; i < leaves.size(); i++)
	//{
	//	drawBoundingBox(leaves[i]->bound);
	//	drawPosition(leaves[i]->refPoint.pos);
	//	for (int j = 0; j < leaves[i]->polygons.size(); j++)
	//	{
	//		drawPolygon(leaves[i]->polygons[j]);
	//	}
	//}
	//printStr("WNV vectors for leaf nodes: ");
	//for (int i = 0; i < leaves.size(); i++)
	//{
	//	printVector(leaves[i]->refPoint.WNV);
	//}


	// Handle leaf node
#if USE_MULTI_THREADING
	std::vector<std::thread> threads(leaves.size());
#endif
	for (int i = 0; i < leaves.size(); i++)
	{
		BSPNode* leaf = leaves[i];
		if (leaf->leftChild != nullptr || leaf->rightChild != nullptr)
		{
			continue;
		}
#if USE_MULTI_THREADING
		threads[i] = std::thread(&BSPTree::LeafTask, this, leaf);
#else
		LeafTask(leaf);
#endif
	}
#if USE_MULTI_THREADING
	for (int i = 0; i < threads.size(); i++)
	{
		threads[i].join();
	}
#endif

	double timeLast = (double)(std::clock() - start) / CLOCKS_PER_SEC;
	std::stringstream ss;
	ss << "algorithm time in seconds = ";
	ss << timeLast;
	printStr(ss.str().c_str());

	drawPolygons(outputPolygons);



	// Test code for the second half
	//clock_t start = std::clock();
	//printStr("build bsp start");
	//BuildLocalBSP(rootNode);
	//printStr("build bsp done");
	//double timeLast = (double)(std::clock() - start) / CLOCKS_PER_SEC;
	//std::stringstream ss;
	//ss << "time lasts in seconds = ";
	//ss << timeLast;
	//printStr(ss.str().c_str());
	////for (int i = 0; i < rootNode->localTrees.size(); i++)
	////{
	//// rootNode->localTrees[i]->drawlocalBSPTree();
	////}
	//printStr("face classification start");
	//FaceClassification(rootNode);
	//printStr("face classification done");
	//drawPolygons(outputPolygons);
}

void BSPTree::LeafTask(BSPNode* leaf)
{
	BuildLocalBSP(leaf);
	FaceClassification(leaf);
}

void BSPTree::FaceClassification(BSPNode* leaf)
{
	// 1. Gather all the polygons from leaf nodes of all local bsp trees
	// They satifsfy P1 and P2 so they candidates for the final ouput polygons
	std::vector<Polygon*> candidates;
	for (int i = 0; i < leaf->localTrees.size(); i++)
	{
		leaf->localTrees[i]->CollectPolygons(candidates);
	}

	// 2. Compute WNV for each candidate polygon
	for (int i = 0; i < candidates.size(); i++)
	{
		Polygon* polygon = candidates[i];

		// 2. Pick a point inside polygon
		Point x = FindPolygonInteriorSimple(polygon);
		if (x.x4 == 0)
		{
			x = FindPolygonInteriorComplex(polygon);
		}
		Segment segment = FindPathBackToRefPoint2(leaf->refPoint, x);

		std::vector<int> WNV = leaf->refPoint.WNV;
		WNV = TraceSegment(candidates, segment, WNV, i);

		switch (mode)
		{
		case 0:
			WNVUnion(polygon, WNV);
			break;
		case 1:
			WNVIntersection(polygon, WNV);
			break;
		case 2:
			WNVBoolean(polygon, WNV);
			break;
		default:
			break;
		}
	}
}

Point BSPTree::FindPolygonInteriorSimple(Polygon* polygon)
{
	// Find polygon interior x by COM and axis line intersection
	// This is relatively computaional friendly
	ivec3 c = getRoundedPolygonCOM(polygon);

	// If polygon normal aligns with axis
	// the COM is guaranteed to be on the surface 
	ivec3 dir = polygon->support.getNormal();
	if ((dir.x != 0 && dir.y == 0 && dir.z == 0) ||
		(dir.y != 0 && dir.x == 0 && dir.z == 0) ||
		(dir.z != 0 && dir.x == 0 && dir.y == 0))
	{
		return Point(c.x, c.y, c.z, 1);
	}

	// Pick the cloest axis that aligns with polygon normal
	int axis = getCloestAxis(polygon->support.getNormal());

	// Line polygon intersection
	Line line = getAxisLine(c, axis);
	Point x = intersectLinePolygon(polygon, line);

	return x;
}

Point BSPTree::FindPolygonInteriorComplex(Polygon* polygon)
{
	// Iteratively find a random point inside polygon
	// Not 100 percent valid

	int pointCount = polygon->bounds.size();
	int randomRange = 100;

	srand((unsigned)time(NULL));

	int iter = 0;
	while (true)
	{
		int i = rand() % pointCount;

		Plane p1 = polygon->bounds[i];
		Plane p2 = polygon->bounds[(i + 1) % pointCount];

	//	// Add random offset to move x around
		p1.d = p1.d + rand() % randomRange;
		p2.d = p2.d + rand() % randomRange;
		Point x = intersect(p1, p2, polygon->support);

		if (isPointInPolygon(polygon, x))
			return x;

		if (++iter >= 1000)
			return x;	// Fallback return
	}
}

void BSPTree::BuildLocalBSP(BSPNode* leaf)
{
	for (int j = 0; j < leaf->polygons.size(); j++)
	{
		LocalBSPTree* localTree = new LocalBSPTree(j, leaf);
		localTree->Build(leaf, bound01, bound02);
		leaf->localTrees.push_back(localTree);
		//printStr("one local bsp done");
	}
}

void BSPTree::Split(BSPNode* node)
{
	// Split by the midpoint of the longest length
	// The split plane always orient along positive axis
	int axis;
	BigInt midValue;
	Plane splitPlane;
	ivec3 min = node->bound.min;
	ivec3 max = node->bound.max;
	BigInt boundX = max.x - min.x;
	BigInt boundY = max.y - min.y;
	BigInt boundZ = max.z - min.z;
	if (boundX >= boundY && boundX >= boundZ)
	{
		axis = 0;
		midValue = BigInt((min.x + max.x) / BigInt(2));
		splitPlane = Plane::fromPositionNormal(ivec3{ midValue, min.y, min.z }, ivec3{ 1, 0, 0 });
	}
	else if (boundY >= boundX && boundY >= boundZ)
	{
		axis = 1;
		midValue = BigInt((min.y + max.y) / BigInt(2));
		splitPlane = Plane::fromPositionNormal(ivec3{ min.x, midValue, min.z }, ivec3{ 0, 1, 0 });
	}
	else
	{
		axis = 2;
		midValue = BigInt((min.z + max.z) / BigInt(2));
		splitPlane = Plane::fromPositionNormal(ivec3{ min.x, min.y, midValue }, ivec3{ 0, 0, 1 });
	}

	// Divide polygons by split plane
	std::vector<Polygon*> leftPolygons;
	std::vector<Polygon*> rightPolygons;
	for (int i = 0; i < node->polygons.size(); i++)
	{	
		auto pairs = splitPolygon(node->polygons[i], splitPlane);

		if (pairs.first != nullptr)
		{
			leftPolygons.push_back(pairs.first);
		}
		if (pairs.second != nullptr)
		{
			rightPolygons.push_back(pairs.second);
		}
	}

	// Left Node
	// - Max bound changes to middle point
	// - RefPoint remains the same
	if (leftPolygons.size() > 0)
	{
		BSPNode* leftNode = new BSPNode();
		leftNode->polygons = leftPolygons;
		node->leftChild = leftNode;
		switch (axis)
		{
		case 0:
			leftNode->bound = AABB{ min, ivec3{midValue, max.y, max.z} };
			break;
		case 1:
			leftNode->bound = AABB{ min, ivec3{max.x, midValue, max.z} };
			break;
		case 2:
			leftNode->bound = AABB{ min, ivec3{max.x, max.y, midValue} };
			break;
		default:
			break;
		}		
		leftNode->refPoint = node->refPoint;
	}

	// Right Node
	// - Min bound changes to middle points
	// - Trace new RefPoint
	if (rightPolygons.size() > 0)
	{
		BSPNode* rightNode = new BSPNode();
		rightNode->polygons = rightPolygons;
		node->rightChild = rightNode;
		switch (axis)
		{
		case 0:
			rightNode->bound = AABB{ ivec3{midValue, min.y, min.z}, max };
			break;
		case 1:
			rightNode->bound = AABB{ ivec3{min.x, midValue, min.z}, max };
			break;
		case 2:
			rightNode->bound = AABB{ ivec3{min.x, min.y, midValue}, max };
			break;
		default:
			break;
		}
		rightNode->refPoint = TraceRefPoint(node, axis);
	}
}

RefPoint BSPTree::TraceRefPoint(BSPNode* node, int axis)
{
	RefPoint refPoint;
	refPoint.pos = node->rightChild->bound.min;
	refPoint.WNV = node->refPoint.WNV;

	// Early out, when no polygons on the left node
	if (node->leftChild == nullptr)
	{
		return refPoint;
	}

	// Line from old ref point and new ref point
	// It is assumed to be one of the axis
	Segment segment = getAxisSegmentFromPositions(node->refPoint.pos, refPoint.pos, axis);

	// Trace refPoint through every polygon
	refPoint.WNV = TraceSegment(node->leftChild->polygons, segment, refPoint.WNV);

	return refPoint;
}

//std::vector<Segment> BSPTree::FindPathBackToRefPoint(RefPoint ref, Point x)
//{
//	// 3. Trace x back to ref point
//	//	by constructing 1 or 2 points in between
//	// (no point in between is impossible because a part of polygon would have been outside AABB)
//	ivec3 xPos = x.getPosition();
//	std::vector<Plane> xPlanes;
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 1, 0, 0 }));
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 1, 0 }));
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 0, 1 }));
//
//	std::vector<Plane> refPlanes;
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 1, 0, 0 }));
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 1, 0 }));
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 0, 1 }));
//
//	std::vector<Segment> segments;
//
//	// 3.1 - Pick the first intermediate point
//	int pick0 = 2;
//	Plane p0 = refPlanes[0];
//	Plane p1 = refPlanes[1];
//	Plane p2 = xPlanes[pick0];
//	while (isPlaneEqual(p2, p0) || isPlaneEqual(p2, p1))
//	{
//		pick0 = (pick0 + 1) % 3;
//		p2 = xPlanes[pick0];
//	}
//	segments.push_back(getSegmentfromPlanes(p0, p1, p2, refPlanes[pick0]));
//
//	// 3.2 - Pick the second intermediate point (if it exists)
//	int pick1 = (pick0 + 1) % 3;
//	int remain = (pick1 + 1) % 3;
//	p2 = xPlanes[pick1];
//	if (isPlaneEqual(p2, refPlanes[remain]))
//	{
//		pick1 = (pick1 + 1) % 3;
//		remain = (pick1 + 1) % 3;
//	}
//
//	if (!isPlaneEqual(p2, refPlanes[remain]))
//	{
//	//// When second point exists
//		segments.push_back(getSegmentfromPlanes(xPlanes[pick1], refPlanes[remain], refPlanes[pick1], xPlanes[remain]));
//		if (!isPlaneEqual(refPlanes[remain], xPlanes[remain]))
//		{
//			segments.push_back(getSegmentfromPlanes(xPlanes[pick0], xPlanes[pick1], xPlanes[remain], refPlanes[remain]));
//		}
//	}
//	else
//	{
//	//	// Or simply connect the first intermediate point and x
//		segments.push_back(getSegmentfromPlanes(xPlanes[pick0], xPlanes[pick1], xPlanes[remain], refPlanes[remain]));
//	}
//	return segments;
//}
//Segment BSPTree::FindPathBackToRefPoint(RefPoint ref, Point x)
//{
//	// 3. Trace x back to ref point
//	//	by constructing 1 or 2 points in between
//	// (no point in between is impossible because a part of polygon would have been outside AABB)
//	ivec3 xPos = x.getPosition();
//	ivec3 refPos = ref.pos;
//
//	std::vector<Plane> xPlanes;
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 1, 0, 0 }));
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 1, 0 }));
//	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 0, 1 }));
//
//	std::vector<Plane> refPlanes;
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 1, 0, 0 }));
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 1, 0 }));
//	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 0, 1 }));
//
//
//	ivec3 dir = xPos - refPos;
//	// should normalize the normal
//	//float length = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);
//
//
//	// A point on the line 
//	ivec3 p = xPos;
//
//	// normal vector for the first plane that is not parallel to the line
//	ivec3 n1{ 1, 0, 0 };
//	// should check if they are parallel here
//
//	BigInt d1 = ivec3::dot(n1, p);
//
//	// first plane 
//	Plane p1( n1.x, n1.y, n1.z, d1 );
//
//	// normal for the second plane that is not parallel to the line
//	ivec3 n2{ 0, 1, 0 };
//
//	BigInt d2 = ivec3::dot(n2, p);
//
//	// second plane
//	Plane p2( n2.x, n2.y, n2.z, d2 );
//
//
//	//return getSegmentfromPlanes(p0, p1, p2, refPlanes[pick0]);
//	return getSegmentfromPlanes(p1, p2, refPlanes[2], xPlanes[2]);
//}

Segment BSPTree::FindPathBackToRefPoint2(RefPoint ref, Point x)
{
	// Build a segment from ref and x directly
	ivec3 xPos = x.getPosition();
	ivec3 lineDir = xPos - ref.pos;
	Line line;

	// If segment is aligned with any axis
	if (abs(lineDir.x) > 100 && abs(lineDir.y) < 100 && abs(lineDir.z) < 100)
	{
		line.p1 = Plane::fromPositionNormal(ref.pos, ivec3{ 0, 1, 0 });
		line.p2 = Plane::fromPositionNormal(ref.pos, ivec3{ 0, 0, 1 });
	}
	else if (abs(lineDir.y) > 100 && abs(lineDir.z) < 100 && abs(lineDir.x) < 100)
	{
		line.p1 = Plane::fromPositionNormal(ref.pos, ivec3{ 1, 0, 0 });
		line.p2 = Plane::fromPositionNormal(ref.pos, ivec3{ 0, 0, 1 });
	}
	else if (abs(lineDir.z) > 100 && abs(lineDir.x) < 100 && abs(lineDir.y) < 100)
	{
		line.p1 = Plane::fromPositionNormal(ref.pos, ivec3{ 1, 0, 0 });
		line.p2 = Plane::fromPositionNormal(ref.pos, ivec3{ 0, 1, 0 });
	}
	else
	{
		// Line is not aligned with any axis
		ivec3 dir1 = ivec3::cross(lineDir, ivec3{ 1, 0, 0 });
		ivec3 dir2 = ivec3::cross(lineDir, ivec3{ 0, 1, 0 });
		line.p1 = Plane::fromPositionNormal(ref.pos, dir1);
		line.p2 = Plane::fromPositionNormal(ref.pos, dir2);
	}

	Plane b1 = Plane::fromPositionNormal(ref.pos, ivec3{ -lineDir.x, -lineDir.y, -lineDir.z });
	Plane b2 = Plane::fromPositionNormal(xPos, lineDir);

	return Segment( line, b1 ,b2 );
}

void BSPTree::WNVUnion(Polygon* polygon, std::vector<int> WNV)
{
#if USE_MULTI_THREADING
	const std::lock_guard<std::mutex> lock(mutex);
#endif 

	if (polygon->meshId == 0 && WNV[1] == 0)
	{
		outputPolygons.push_back(polygon);
	}	
	else if (polygon->meshId == 1 && WNV[0] == 0)
	{
		outputPolygons.push_back(polygon);
	}
}

void BSPTree::WNVIntersection(Polygon* polygon, std::vector<int> WNV)
{
#if USE_MULTI_THREADING
	const std::lock_guard<std::mutex> lock(mutex);
#endif 

	if (polygon->meshId == 0 && WNV[1] != 0)
	{
		outputPolygons.push_back(polygon);
	}
	else if (polygon->meshId == 1 && WNV[0] != 0)
	{
		outputPolygons.push_back(polygon);
	}
}

void BSPTree::WNVBoolean(Polygon* polygon, std::vector<int> WNV)
{
	// Temporary method to map WNV (assumes we only have 2 meshes for now)
	
#if USE_MULTI_THREADING
	const std::lock_guard<std::mutex> lock(mutex);
#endif 

	if (polygon->meshId == 0 && WNV[1] == 0)
	{
		outputPolygons.push_back(polygon);
	}
	else if(polygon->meshId == 1 && WNV[0] > 0)
	{
		std::vector<Plane> bounds;
		for (int i = polygon->bounds.size() - 1; i >= 0; i--)
		{
			bounds.push_back(polygon->bounds[i]);
		}
		Polygon* flipPolygon = new Polygon(polygon->meshId, polygon->support, bounds);
		outputPolygons.push_back(flipPolygon);
	}
}

// ========== Local BSP ============

LocalBSPTree::LocalBSPTree(int index, BSPNode* leaf)
{
	LocalBSPNode* root = new LocalBSPNode();
	root->polygon = leaf->polygons[index];
	//drawPolygon(root->polygon);
	nodes.push_back(root);
	mark = index; // The mark is its index in the leaf BSP node
}

LocalBSPTree::~LocalBSPTree()
{
}

void LocalBSPTree::Build(BSPNode* leaf, AABB& bound01, AABB& bound02)
{
	// Early out: check if polygon has intersection with the other mesh at all
	AABB& otherBound = nodes[0]->polygon->meshId == 0 ? bound02 : bound01;
	if (!isAABBIntersect(nodes[0]->polygon->aabb, otherBound))
		return;

	// Add segment by intersecting with all other polygons in the same leaf node
	for (int i = 0; i < leaf->polygons.size(); i++)
	{
		if (i == mark)
		{
			continue;
		}

		// If we can guarantee that input mesh has no self intersections
		if (leaf->polygons[i]->meshId == nodes[0]->polygon->meshId)
			continue;

		// If there is a chance for intersection
		if (!isAABBIntersect(nodes[0]->polygon->aabb, leaf->polygons[i]->aabb))
			continue;

		std::vector<Segment> segments = IntersectWithPolygon(leaf->polygons[i]);
		for (int j = 0; j < segments.size(); j++)
		{
			Point v0 = intersect(segments[j].line.p1, segments[j].line.p2, segments[j].bound1);
			Point v1 = intersect(segments[j].line.p1, segments[j].line.p2, segments[j].bound2);
			Plane s = segments[j].line.p1;
			if (isDirectionEqual(s.getNormal(), nodes[0]->polygon->support.getNormal()))
			{
				s = segments[j].line.p2;
			}

			AddSegment(nodes[0], v0, v1, s, i);
		}
	}
	
}

void LocalBSPTree::AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s, int otherMark)
{
	if (node == nullptr)
	{
		return;
	}

	bool isLeaf = node->leftChild == nullptr && node->rightChild == nullptr;

	if(isLeaf)
	{
		auto pairs = splitPolygon(node->polygon, s);
		node->plane = s; // Current node becomes inner

		LocalBSPNode* leftNode = nullptr;
		LocalBSPNode* rightNode = nullptr;

		if (pairs.first != nullptr)
		{
			leftNode = new LocalBSPNode();
			leftNode->polygon = pairs.first;
		}
		if (pairs.second != nullptr)
		{
			rightNode = new LocalBSPNode();
			rightNode->polygon = pairs.second;
		}

		node->leftChild = leftNode;
		node->rightChild = rightNode;

		if (leftNode != nullptr) nodes.push_back(leftNode);
		if (rightNode != nullptr) nodes.push_back(rightNode);
	}
	else
	{
		long long int c0 = classify(v0, node->plane);
		long long int c1 = classify(v1, node->plane);

		if (c0 == c1 && c0 == 0) // Segment lies on the splitting plane
		{
			return; 
		}
		else if (c0 <= 0 && c1 <= 0)
		{
			AddSegment(node->leftChild, v0, v1, s, otherMark);
		}
		else if (c0 >= 0 && c1 >= 0)
		{
			AddSegment(node->rightChild, v0, v1, s, otherMark);
		}
		else if (c0 < 0 && c1 > 0)
		{
			Point v = intersect(s, node->plane, node->polygon->support);
			AddSegment(node->leftChild, v0, v, s, otherMark);
			AddSegment(node->rightChild, v, v1, s, otherMark);
		}
		else if (c0 > 0 && c1 < 0)
		{
			Point v = intersect(s, node->plane, node->polygon->support);
			AddSegment(node->leftChild, v, v1, s, otherMark);
			AddSegment(node->rightChild, v0, v, s, otherMark);
		}
	}
}

std::vector<Segment> LocalBSPTree::IntersectWithPolygon(Polygon* p2)
{
	Polygon* p1 = nodes[0]->polygon;
	std::vector<Segment> segments;

	// Check how many distinct points p1 p2 intersect with each other
	// POTENTIAL OPTIMIZATION: COMBINE THESE TWO FOR LOOPS
	std::vector<Point> points;
	for (int i = 0; i < p1->bounds.size(); i++)
	{
		Point point = intersectSegmentPolygon(p2, getPolygonSegment(p1, i));

		if (!point.isValid())
			continue;	

		if (points.empty())
		{
			points.push_back(point);
		}
		else
		{
			if (!isPointEqual(points[0], point))
			{
				points.push_back(point);
				break;
			}
		}
	}
	if (points.size() < 2)
	{
		for (int i = 0; i < p2->bounds.size(); i++)
		{
			Point point = intersectSegmentPolygon(p1, getPolygonSegment(p2, i));

			if (!point.isValid())
				continue;

			if (points.empty())
			{
				points.push_back(point);
			}
			else
			{
				if (!isPointEqual(points[0], point))
				{
					points.push_back(point);
					break;
				}
			}
		}
	}
	
	ivec3 nor1 = p1->support.getNormal();
	ivec3 nor2 = p2->support.getNormal();
	bool planeParallel = isDirectionEqual(nor1, nor2);
	if (planeParallel)
	{
		if (points.size() >= 2)
		{
			// C4: intersection forms a polygon (collect all p2's edges)
			for (int i = 0; i < p2->bounds.size(); i++)
			{
				segments.push_back(getPolygonSegment(p2, i));
			}
		}
		else
		{
			// C1 or C2: no intersection or degenerate segment (ignore)
		}
	}
	else
	{
		if (points.size() >= 2)
		{
			// C3: forms a non-degenerate segment (add this segment)
			ivec3 nor = ivec3::cross(p1->support.getNormal(), p2->support.getNormal());

			// Make sure the bound planes orient outward
			ivec3 stPos = points[0].getPosition();
			ivec3 edPos = points[1].getPosition();
			ivec3 stNor = nor;
			ivec3 edNor = nor;
			if (ivec3::dot(nor, edPos - stPos) >= 0)
			{
				stNor = ivec3{ -nor.x, -nor.y, -nor.z };
			}
			else
			{
				edNor = ivec3{ -nor.x, -nor.y, -nor.z };
			}

			Plane bound1 = Plane::fromPositionNormal(stPos, stNor);
			Plane bound2 = Plane::fromPositionNormal(edPos, edNor);
			
			segments.push_back(Segment{ Line{p1->support, p2->support}, bound1, bound2 });
		}
		else
		{
			// C1 or C2: no intersection or degenerate segment (ignore)
		}
	}


	return segments;
}

void LocalBSPTree::CollectPolygons(std::vector<Polygon*>& container)
{
	for (int i = 0; i < nodes.size(); i++)
	{
		LocalBSPNode* node = nodes[i];

		if (node->leftChild != nullptr || node->rightChild != nullptr || node->disable)
			continue;

		container.push_back(node->polygon);
	}
}

void LocalBSPTree::drawlocalBSPTree()
{
	for (int i = 0; i < nodes.size(); i++)
	{
		if (nodes[i]->leftChild == nullptr && nodes[i]->rightChild == nullptr)
		{
			//printStr("A LEAF NODE!");
			//if (!nodes[i]->disable)
			{
				drawPolygon(nodes[i]->polygon);
			}
		}
	}
}