#pragma once

#include "geometry.h"

namespace ember
{
	struct AABB 
	{
		Point min, max;
		std::vector<Polygon&> polygons;
	};

	struct RefPoint
	{
		Point point;
		std::vector<int> WNV;
	};

	struct BSPNode
	{
		AABB bound;
		RefPoint refPoint;

		BSPNode* leftChild;
		BSPNode* rightChild;
	};

	class BSPTree
	{
	public:
		BSPTree();
		
		void Split(BSPNode* node, Plane s);


	public:
		BSPNode* root;
	};
}