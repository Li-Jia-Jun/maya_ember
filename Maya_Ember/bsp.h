#pragma once

#include "util.h"


namespace ember
{
	class EMBER;

	struct AABB 
	{
		//Point min, max;
		ivec3 min, max;
	};

	struct RefPoint
	{
		ivec3 pos;				
		std::vector<int> WNV;
	};

	struct BSPNode
	{
		AABB bound;
		RefPoint refPoint;
		std::vector<Polygon*> polygons;

		BSPNode* leftChild;
		BSPNode* rightChild;
	};

	class BSPTree
	{
	public:
		BSPTree();
		~BSPTree();

		void Build(BSPNode* rootNode);

		void Split(BSPNode* node);

		RefPoint TraceRefPoint(BSPNode* node);

	private:
		BSPNode* root;
		
		EMBER* EMBER;
	};
}