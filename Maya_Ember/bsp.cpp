#include "bsp.h"
#include "ember.h"

using namespace ember;

BSPTree::BSPTree()
{

}

BSPTree::~BSPTree()
{
	// TODO:: Recursively delete tree node 
}

void BSPTree::Build(BSPNode* rootNode)
{
	this->root = rootNode;

	// Build tree recursively
	std::vector<BSPNode*> nodes;
	nodes.push_back(root);
	while (!nodes.empty())
	{	
		BSPNode* node = nodes.back();
		nodes.pop_back();

		// Determine leaf node
		if (node->polygons.size() <= 25)
			continue;

		// Split
		Split(node);

		// Collect new nodes
		if (node->leftChild != nullptr)
		{
			nodes.push_back(node->leftChild);
		}
		if (node->rightChild != nullptr)
		{
			nodes.push_back(node->rightChild);
		}
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
		Polygon* polygon = node->polygons[i];
		
		std::vector<Plane> leftEdgePlanes;
		std::vector<Plane> rightEdgePlanes;
		bool hasSplit = false;

		// Divide edge by split plane
		int boundSize = polygon->bounds.size();
		for (int j = 0; j < boundSize; j++)
		{
			Plane edgePlane = polygon->bounds[j];
			Plane bound1 = polygon->bounds[(j - 1 + boundSize) % boundSize];
			Plane bound2 = polygon->bounds[(j + 1) % boundSize];
	
			int c1 = classify(intersect(polygon->support, edgePlane, bound1), splitPlane);
			int c2 = classify(intersect(polygon->support, edgePlane, bound2), splitPlane);

			if (c1 <= 0 && c2 <= 0)		
			{
				// Edge to left (no positive values, including all zero)
				leftEdgePlanes.push_back(edgePlane);
			}
			else if (c1 >= 0 && c2 >= 0)	
			{
				// Edge to right (all positive values)
				rightEdgePlanes.push_back(edgePlane);
			}
			else
			{
				// Split and add edge to both sides
				leftEdgePlanes.push_back(edgePlane);
				
				if (!hasSplit)
				{
					leftEdgePlanes.push_back(splitPlane);
					rightEdgePlanes.push_back(splitPlane);
					hasSplit = true;
				}

				rightEdgePlanes.push_back(edgePlane);
			}
		}

		// Collect divided edges to build polygons
		if (!leftEdgePlanes.empty())
		{
			Polygon* newPolygon = new Polygon();
			newPolygon->meshId = polygon->meshId;
			newPolygon->bounds = leftEdgePlanes;
			newPolygon->support = polygon->support;

			// Don't forget to store new polygon to EMBER
			EMBER->AddPolygon(newPolygon); 
		}
		if (!rightEdgePlanes.empty())
		{
			Polygon* newPolygon = new Polygon();
			newPolygon->meshId = polygon->meshId;
			newPolygon->bounds = rightEdgePlanes;
			newPolygon->support = polygon->support;

			// Don't forget to store new polygon to EMBER
			EMBER->AddPolygon(newPolygon);
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
		rightNode->refPoint = TraceRefPoint(node);
		node->rightChild = rightNode;
	}
}

RefPoint BSPTree::TraceRefPoint(BSPNode* node)
{
	// TODO::
	RefPoint refPoint = node->refPoint;
	return refPoint;

}