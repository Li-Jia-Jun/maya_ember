#pragma once
#include "util.h"
#include <thread>
#include <mutex>


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
		void SetMeshBounds(AABB bound01, AABB bound02);
		void Build(BSPNode* rootNode, int mode);
		void Split(BSPNode* node);
		void LeafTask(BSPNode* leaf);
		void BuildLocalBSP(BSPNode* leaf);
		void FaceClassification(BSPNode* leaf);
		Point FindPolygonInteriorSimple(Polygon* polygon);
		Point FindPolygonInteriorComplex(Polygon* polygon);
		RefPoint TraceRefPoint(BSPNode* node, int axis);
		//std::vector<Segment> FindPathBackToRefPoint(RefPoint ref, Point x);
		//Segment FindPathBackToRefPoint(RefPoint ref, Point x);
		Segment FindPathBackToRefPoint2(RefPoint ref, Point x);

		void WNVUnion(Polygon* polygon, std::vector<int> WNV);
		void WNVIntersection(Polygon* polygon, std::vector<int> WNV);
		void WNVBoolean(Polygon* polygon, std::vector<int> WNV);
		
	private:
		std::mutex mutex;
		AABB bound01;
		AABB bound02;
		std::vector<BSPNode*> nodes;				// Element 0 is root node
		std::vector<Polygon*> outputPolygons;		// The final output (boolean)

		int mode;									// Same as EMBER
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
		void Build(BSPNode* leaf, AABB& bound01, AABB& bound02);
		void AddSegment(LocalBSPNode* node, Point v0, Point v1, Plane s, int otherMark);
		//void AddSegment(LocalBSPNode* node, Segment segment, Plane s, int otherMark);
		void CollectPolygons(std::vector<Polygon*>& container);
		std::vector<Segment> IntersectWithPolygon(Polygon* p2);
		void drawlocalBSPTree();

	private:
		std::vector<LocalBSPNode*> nodes;		// Element 0 is root node
		int mark;
	};
}