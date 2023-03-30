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
		Polygon* polygon = polygons[i];
		
		bool isValid = true;
		
		Point x = intersect(polygon->support, traceSegment.line.p1, traceSegment.line.p2);

		// If intersection is unique
		if (x.x4 == 0)
		{
			isValid = false;
		}

		// If intersection point is within segment
		int c1 = classify(x, traceSegment.bound1);
		int c2 = classify(x, traceSegment.bound2);
		if (c1 > 0 || c2 > 0)
		{
			isValid = false;
		}

		// If the intersection point is inside polygon
		for (int j = 0; isValid && j < polygon->bounds.size(); j++)
		{
			int c = classify(x, polygon->bounds[j]);
			if (c > 0)
			{
				isValid = false;
				break;
			}
		}

		if (isValid)
		{
			ivec3 segmentDir = refPoint.pos - node->refPoint.pos;
			
			int sign = ivec3::dot(segmentDir, polygon->support.normal());
			if (sign > 0)
			{
				// Trace is going out
				int meshId = polygon->meshId;
				refPoint.WNV[meshId] = refPoint.WNV[meshId] - 1;
			}
			else
			{
				// Trace is going in
				int meshId = polygon->meshId;
				refPoint.WNV[meshId] = refPoint.WNV[meshId] + 1;
			}
		}
	}

	return refPoint;
}


// ========== Local BSP ============

LocalBSPTree::LocalBSPTree()
{
}

LocalBSPTree::~LocalBSPTree()
{
}

void LocalBSPTree::AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s)
{
	if (node == nullptr)
	{
		return;
	}

	bool isLeaf = node->leftChild == nullptr && node->rightChild == nullptr;

	if(isLeaf)
	{
		auto pairs = splitPolygon(polygon, s);
		node->plane = s; // Current node becomes inner
		
		LocalBSPNode* leftNode = new LocalBSPNode();
		LocalBSPNode* rightNode = new LocalBSPNode();
		leftNode->polygon = pairs.first;
		rightNode->polygon = pairs.second;

		node->leftChild = leftNode;
		node->rightChild = rightNode;
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
			AddSegment(node->leftChild, v0, v1, s);
		}
		else if (c0 >= 0 && c1 >= 0)
		{
			AddSegment(node->rightChild, v0, v1, s);
		}
		else if (c0 < 0 && c1 > 0)
		{
			Point v = intersect(s, node->plane, polygon->support);
			AddSegment(node->leftChild, v0, v, s);
			AddSegment(node->rightChild, v, v1, s);
		}
		else if (c0 > 0 && c1 < 0)
		{
			Point v = intersect(s, node->plane, polygon->support);
			AddSegment(node->leftChild, v, v1, s);
			AddSegment(node->rightChild, v0, v, s);
		}
	}
}