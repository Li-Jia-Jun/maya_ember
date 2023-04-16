#pragma once
#include "util.h"

namespace ember
{
	class LocalBSPTree;


	// Global BSP for AABB subdivision

	struct BSPNode
	{
		AABB bound;
		RefPoint refPoint;
		std::vector<Polygon*> polygons;
		std::vector<LocalBSPTree*> localTrees;

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
		void BuildLocalBSP(BSPNode* leaf);
		void FaceClassification(BSPNode* leaf);
		Point FindPolygonInteriorSimple(Polygon* polygon);
		Point FindPolygonInteriorComplex(Polygon* polygon);
		RefPoint TraceRefPoint(BSPNode* node, int axis);
		// LEO::to be updated with the new segment method
		//std::vector<Segment> FindPathBackToRefPoint(RefPoint ref, Point x);
		Segment FindPathBackToRefPoint(RefPoint ref, Point x);

		bool WNVBoolean(std::vector<int> WNV);
		
	private:
		std::vector<BSPNode*> nodes;				// Element 0 is root node
		std::vector<Polygon*> outputPolygons;		// The final output (boolean)
	};

	// Local BSP for leaf node of the Global BSP

	struct LocalBSPNode
	{
		Polygon* polygon;	// For leaf node
		Plane plane;		// For inner node
		bool disable;

		LocalBSPNode* leftChild = nullptr;
		LocalBSPNode* rightChild = nullptr;
	};

	class LocalBSPTree
	{
	public:
		LocalBSPTree(int index, BSPNode* leaf);
		~LocalBSPTree();
		void AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s, int otherMark);	
		void CollectPolygons(std::vector<Polygon*>& container);
		std::vector<Segment> IntersectWithPolygon(Polygon* p2);

	private:
		std::vector<LocalBSPNode*> nodes;		// Element 0 is root node
		int mark;
	};
}