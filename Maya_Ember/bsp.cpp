#include "bsp.h"
#include <cstdlib>
#include <time.h>
using namespace ember;


// ========== BSP ============

BSPTree::BSPTree()
{

}

BSPTree::~BSPTree()
{
	// TODO:: Recursively delete tree node 
}

void BSPTree::Build(BSPNode* rootNode)
{
	nodes.clear();
	nodes.push_back(rootNode);

	// Create tree nodes recursively (in breadth first order)
	std::vector<BSPNode*> toTraverse;
	toTraverse.push_back(rootNode);
	int tempCount = 0;
	while (!toTraverse.empty())
	{	
		BSPNode* node = toTraverse.back();
		toTraverse.pop_back();

		// Leaf node determination
		if (node->polygons.size() <= LEAF_POLYGON_COUNT)
			continue;

		// Split AABB
		Split(node);

		// Collect new nodes
		if (node->leftChild != nullptr)
		{
			toTraverse.push_back(node->leftChild);
			nodes.push_back(node->leftChild);
		}
		if (node->rightChild != nullptr)
		{
			toTraverse.push_back(node->rightChild);
			nodes.push_back(node->rightChild);
		}

		if (tempCount++ >= 20)
		{
			break;
		}
	}

	// Draw all the sub bounding boxes by iterating all the nodes and draw their AABB
	//for (int i = 0; i < nodes.size(); ++i)
	//{
	//	drawBoundingBox(nodes[i]->bound);
	//}

	// Handle leaf node
	//for (int i = 0; i < nodes.size(); i++)
	//{
	//	BSPNode* leaf = nodes[i];
	//	if (leaf->leftChild != nullptr || leaf->rightChild != nullptr)
	//	{
	//		continue;
	//	}

	//	BuildLocalBSP(leaf);
	//	FaceClassification(leaf);
	//}
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
		
		//printPolygon(*polygon);

		// 2. Pick a point inside polygon
		Point x = FindPolygonInteriorSimple(polygon);
		if (x.x4 == 0)
		{
			x = FindPolygonInteriorComplex(polygon);
		}

		//printPoint(x);

		// LEO::To be updated with the new segment method
		// Find path from x to ref point
		//std::vector<Segment> segments = FindPathBackToRefPoint(leaf->refPoint, x);
		Segment segment = FindPathBackToRefPoint(leaf->refPoint, x);
		
		// Update WNV
		//std::vector<int> WNV = leaf->refPoint.WNV;
		//for (int j = 0; j < segments.size(); j++)
		//{
		//	for (int k = 0; k < candidates.size(); k++)
		//	{
		//		if (i == k)
		//			continue;

		//		WNV = TraceSegment(candidates[k], segments[j], WNV);
		//	}
		//}
		
		std::vector<int> WNV = leaf->refPoint.WNV;

			for (int k = 0; k < candidates.size(); k++)
			{
				if (i == k)
					continue;

				WNV = TraceSegment(candidates[k], segment, WNV);
			}

		if (WNVBoolean(WNV))
		{
			outputPolygons.push_back(polygon);
		}
		//printPolygon(*polygon);
	}
}

Point BSPTree::FindPolygonInteriorSimple(Polygon* polygon)
{
	// Find polygon interior x by COM and axis line intersection
	// This is relatively computaional friendly

	ivec3 c = getRoundedPolygonCOM(polygon);

	// Pick the cloest axis that aligns with polygon normal
	int axis = getCloestAxis(polygon->support.getNormal());

	//char buffer[128];
	//sprintf_s(buffer, "Find inside point simple, com and axis: %i %i %i %i", c.x, c.y, c.z, axis);
	//MString debug(buffer);
	//MGlobal::displayInfo(buffer);

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
		leaf->localTrees.push_back(localTree);
	}
}

void BSPTree::Split(BSPNode* node)
{
	// Split by the midpoint of the longest length
	// The split plane always orient along positive axis
	int axis;
	long long int midValue;
	Plane splitPlane;
	ivec3 min = node->bound.min;
	ivec3 max = node->bound.max;
	long long int boundX = max.x - min.x;
	long long int boundY = max.y - min.y;
	long long int boundZ = max.z - min.z;
	if (boundX > boundY && boundX > boundZ)
	{
		axis = 0;
		midValue = long long int((min.x + max.x) * 0.5f);
		splitPlane = Plane::fromPositionNormal(ivec3{ midValue, min.y, min.z }, ivec3{ 1, 0, 0 });
	}
	else if (boundY > boundX && boundY > boundZ)
	{
		axis = 1;
		midValue = long long int((min.y + max.y) * 0.5f);
		splitPlane = Plane::fromPositionNormal(ivec3{ min.x, midValue, min.z }, ivec3{ 0, 1, 0 });
	}
	else
	{
		axis = 2;
		midValue = long long int((min.z + max.z) * 0.5f);
		splitPlane = Plane::fromPositionNormal(ivec3{ min.x, min.y, midValue }, ivec3{ 0, 0, 1 });
	}

	//printStr("split plane:");
	//printPlane(splitPlane);
	

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
		node->leftChild = leftNode;
	}

	// Right Node
	// - Min bound changes to middle points
	// - Trace new RefPoint
	//if (rightPolygons.size() > 0)
	//{
	//	BSPNode* rightNode = new BSPNode();
	//	rightNode->polygons = rightPolygons;

	//	switch (axis)
	//	{
	//	case 0:
	//		rightNode->bound = AABB{ ivec3{midValue, min.y, min.z}, max };
	//		break;
	//	case 1:
	//		rightNode->bound = AABB{ ivec3{min.x, midValue, min.z}, max };
	//		break;
	//	case 2:
	//		rightNode->bound = AABB{ ivec3{min.x, min.y, midValue}, max };
	//		break;
	//	default:
	//		break;
	//	}

	//	rightNode->refPoint = TraceRefPoint(node, axis);
	//	node->rightChild = rightNode;
	//}
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
	std::vector<Polygon*> polygons = node->leftChild->polygons;
	for (int i = 0; i < polygons.size(); i++)
	{
		refPoint.WNV = TraceSegment(polygons[i], segment, refPoint.WNV);
	}

	return refPoint;
}

std::vector<int> BSPTree::TraceSegment(Polygon* polygon, Segment segment, std::vector<int> WNV)
{
	bool hasIntersect = intersectSegmentPolygon(polygon, segment).x4 != 0;
	if (hasIntersect)
	{
		Point st = intersect(segment.line.p1, segment.line.p2, segment.bound1);
		Point ed = intersect(segment.line.p1, segment.line.p2, segment.bound2);
		ivec3 dir = ed.getPosition() - st.getPosition();

		long long int sign = ivec3::dot(dir, polygon->support.getNormal());
		if(sign > 0)
		{
			// Segment is going out
			WNV[polygon->meshId] = WNV[polygon->meshId] - 1;
		}
		else if(sign < 0)
		{
			// Segment is going in
			WNV[polygon->meshId] = WNV[polygon->meshId] + 1;
		}
		else
		{
			// Segment lays on the polygon (impossible under our assumption?)
		}	
	}

	return WNV;
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


Segment BSPTree::FindPathBackToRefPoint(RefPoint ref, Point x)
{
	// 3. Trace x back to ref point
	//	by constructing 1 or 2 points in between
	// (no point in between is impossible because a part of polygon would have been outside AABB)
	ivec3 xPos = x.getPosition();
	ivec3 refPos = ref.pos;

	std::vector<Plane> xPlanes;
	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 1, 0, 0 }));
	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 1, 0 }));
	xPlanes.push_back(Plane::fromPositionNormal(xPos, { 0, 0, 1 }));

	std::vector<Plane> refPlanes;
	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 1, 0, 0 }));
	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 1, 0 }));
	refPlanes.push_back(Plane::fromPositionNormal(ref.pos, { 0, 0, 1 }));


	ivec3 dir = xPos - refPos;
	// should normalize the normal
	//float length = sqrt(dir.x * dir.x + dir.y * dir.y + dir.z * dir.z);


	// A point on the line 
	ivec3 p = xPos;

	// normal vector for the first plane that is not parallel to the line
	ivec3 n1{ 1, 0, 0 };
	// should check if they are parallel here

	long long int d1 = ivec3::dot(n1, p);

	// first plane 
	Plane p1( n1.x, n1.y, n1.z, d1 );

	// normal for the second plane that is not parallel to the line
	ivec3 n2{ 0, 1, 0 };

	long long int d2 = ivec3::dot(n2, p);

	// second plane
	Plane p2( n2.x, n2.y, n2.z, d2 );


	//return getSegmentfromPlanes(p0, p1, p2, refPlanes[pick0]);
	return getSegmentfromPlanes(p1, p2, refPlanes[2], xPlanes[2]);
}

bool BSPTree::WNVBoolean(std::vector<int> WNV)
{
	// Temporary method to map WNV (assumes we only have 2 meshes for now)
	if (WNV[0] != 0 && WNV[1] == 0)
	{
		return true;
	}

	return false;
}

// ========== Local BSP ============

LocalBSPTree::LocalBSPTree(int index, BSPNode* leaf)
{
	LocalBSPNode* root = new LocalBSPNode();
	root->polygon = leaf->polygons[index];
	//printPolygon(*(root->polygon));
	nodes.push_back(root);
	mark = index; // The mark is its index in the leaf BSP node

	// Add segment by intersecting with all other polygons in the same leaf node
	for (int i = 0; i < leaf->polygons.size(); i++)
	{
		if (i == mark)
		{
			continue;
		}

		std::vector<Segment> segments = IntersectWithPolygon(leaf->polygons[i]);
		
		for (int j = 0; j < segments.size(); j++)
		{
			Point v0 = intersect(segments[j].line.p1, segments[j].line.p2, segments[j].bound1);
			Point v1 = intersect(segments[j].line.p1, segments[j].line.p2, segments[j].bound2);
			Plane s = leaf->polygons[i]->support;
			
			AddSegment(root, v0, v1, s, i);
		}
	}
}

LocalBSPTree::~LocalBSPTree()
{
}

void LocalBSPTree::AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s, int otherMark)
{
	if (node == nullptr)
	{
		return;
	}

	Polygon* polygon = nodes[0]->polygon;

	bool isLeaf = node->leftChild == nullptr && node->rightChild == nullptr;
	if(isLeaf)
	{
		auto pairs = splitPolygon(polygon, s);
		node->plane = s; // Current node becomes inner
		
		LocalBSPNode* leftNode = new LocalBSPNode();
		LocalBSPNode* rightNode = new LocalBSPNode();
		leftNode->polygon = pairs.first;
		rightNode->polygon = pairs.second;

		// Mark the node as not used in final output
		if (node->disable || otherMark < mark)
		{
			leftNode->disable = true;
			leftNode->disable = true;
		}

		node->leftChild = leftNode;
		node->rightChild = rightNode;

		nodes.push_back(leftNode);
		nodes.push_back(rightNode);
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
			Point v = intersect(s, node->plane, polygon->support);
			AddSegment(node->leftChild, v0, v, s, otherMark);
			AddSegment(node->rightChild, v, v1, s, otherMark);
		}
		else if (c0 > 0 && c1 < 0)
		{
			Point v = intersect(s, node->plane, polygon->support);
			AddSegment(node->leftChild, v, v1, s, otherMark);
			AddSegment(node->rightChild, v0, v, s, otherMark);
		}
	}
}

std::vector<Segment> LocalBSPTree::IntersectWithPolygon(Polygon* p2)
{
	Polygon* p1 = nodes[0]->polygon;
	std::vector<Segment> segments;

	// Check how many distinct points p1 will intersect with p2
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
			ivec3 pos1 = points[0].getPosition();
			ivec3 pos2 = point.getPosition();
			if (pos1 != pos2)
			{
				points.push_back(point);
				break;  // 2 intersect points are sufficient to distinguish 
						// between intersecting on a point or an overlapping area
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
			Plane bound1 = Plane::fromPositionNormal(points[0].getPosition(), nor);
			Plane bound2 = Plane::fromPositionNormal(points[1].getPosition(), nor);
			
			segments.push_back(getSegmentfromPlanes(p1->support, p2->support, bound1, bound2));
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