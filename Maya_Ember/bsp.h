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


	// BSP for AABB subdivision

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
		void LeafTask(BSPNode* leaf);
		RefPoint TraceRefPoint(BSPNode* node, int axis);

	private:
		std::vector<BSPNode*> nodes; // Element 0 is root
	};



	// Local BSP for leaf node

	struct LocalBSPNode
	{
		Polygon* polygon;	// For leaf node
		Plane plane;		// For inner node

		LocalBSPNode* leftChild;
		LocalBSPNode* rightChild;
	};

	class LocalBSPTree
	{
	public:
		LocalBSPTree();
		~LocalBSPTree();

		void AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s);

	private:
		std::vector<LocalBSPNode*> nodes;
		Polygon* polygon;
	};
}