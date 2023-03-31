#include "bsp.h"

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

	// Build tree recursively
	std::vector<BSPNode*> toTraverse;
	toTraverse.push_back(rootNode);
	while (!nodes.empty())
	{	
		BSPNode* node = toTraverse.back();
		toTraverse.pop_back();

		// Leaf node determination
		if (node->polygons.size() <= LEAF_POLYGON_COUNT)
			continue;

		// Split
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
	}

	// Handle leaf nodes
	for (int i = 0; i < nodes.size(); i++)
	{
		BSPNode* leaf = nodes[i];
		if (leaf->leftChild != nullptr || leaf->rightChild != nullptr)
		{
			continue;
		}

		LeafTask(leaf);
	}
}

void BSPTree::LeafTask(BSPNode* leaf)
{
	// Create local BSP tree for every polygon
	for (int i = 0; i < leaf->polygons.size(); i++)
	{
		LocalBSPTree* localTree = new LocalBSPTree(i, leaf);
		leaf->localTrees.push_back(localTree);
	}
}

void BSPTree::Split(BSPNode* node)
{
	// Split by the midpoint of the longest length
	int axis, midValue;
	Plane splitPlane;
	ivec3 min = node->bound.min;
	ivec3 max = node->bound.max;
	int boundX = max.x - min.x;
	int boundY = max.y - min.y;
	int boundZ = max.z - min.z;
	if (boundX > boundY && boundX > boundZ)
	{
		axis = 0;
		midValue = int((min.x + max.x) * 0.5f);
		splitPlane = Plane::fromPositionNormal(ivec3{ midValue, min.y, min.z }, ivec3{ 1, 0, 0 });
	}
	else if (boundY > boundX && boundY > boundZ)
	{
		axis = 1;
		midValue = int((min.y + max.y) * 0.5f);
		splitPlane = Plane::fromPositionNormal(ivec3{ min.x, midValue, min.z }, ivec3{ 0, 1, 0 });
	}
	else
	{
		axis = 2;
		midValue = int((min.z + max.z) * 0.5f);
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
	// - Max bound changes 
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
	// - Min bound changes
	// - Trace new RefPoint
	if (rightPolygons.size() > 0)
	{
		BSPNode* rightNode = new BSPNode();
		rightNode->polygons = rightPolygons;

		ivec3 adjust = ivec3{ AABB_ADJUST, AABB_ADJUST, AABB_ADJUST };
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

		// Apply adjustment to guarantee new ref point will not be on any polygon surface
		rightNode->bound.min = rightNode->bound.min - adjust; 

		rightNode->refPoint = TraceRefPoint(node, axis);
		node->rightChild = rightNode;
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
	Segment traceSegment = getAxisSegmentFromPositions(node->refPoint.pos, refPoint.pos, axis);

	// Trace refPoint through every polygon
	std::vector<Polygon*> polygons = node->leftChild->polygons;
	for (int i = 0; i < polygons.size(); i++)
	{
		bool hasIntersect = intersectSegmentPolygon(polygons[i], traceSegment).x4 != 0;

		if (hasIntersect)
		{
			ivec3 segmentDir = refPoint.pos - node->refPoint.pos;
			
			int sign = ivec3::dot(segmentDir, polygons[i]->support.getNormal());
			if (sign > 0)
			{
				// Trace is going out
				int meshId = polygons[i]->meshId;
				refPoint.WNV[meshId] = refPoint.WNV[meshId] - 1;
			}
			else
			{
				// Trace is going in
				int meshId = polygons[i]->meshId;
				refPoint.WNV[meshId] = refPoint.WNV[meshId] + 1;
			}
		}
	}

	return refPoint;
}


// ========== Local BSP ============

LocalBSPTree::LocalBSPTree(int index, BSPNode* leaf)
{
	LocalBSPNode* root = new LocalBSPNode();
	root->polygon = leaf->polygons[mark];
	nodes.push_back(root);
	mark = index; // The mark is its index in the leaf BSP node

	// Add segment by all other polygons in leaf node
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
		int c0 = classify(v0, node->plane);
		int c1 = classify(v1, node->plane);

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
		Point point = intersectSegmentPolygon(p2, p1->getSegment(i));

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
				segments.push_back(p2->getSegment(i));
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
			Segment segment;
			segment.line = Line{ p1->support, p2->support };
			ivec3 nor = ivec3::cross(p1->support.getNormal(), p2->support.getNormal());
			segment.bound1 = Plane::fromPositionNormal(points[0].getPosition(), nor);
			segment.bound2 = Plane::fromPositionNormal(points[1].getPosition(), nor);
			
			segments.push_back(segment);
		}
		else
		{
			// C1 or C2: no intersection or degenerate segment (ignore)
		}
	}

	return segments;
}