#pragma once
#include "bsp.h"


namespace ember
{
	class EMBER
	{
	public:
		EMBER();

		void ReadMeshData(std::vector<std::vector<ivec3>>& vertices, std::vector<ivec3>& normals);

		void SetInitBounds(AABB bound, AABB bound01, AABB bound02);

		void SetMode(int mode);

		void BuildBSPTree();

	private:
		int meshId;							// Incremental count of meshes
		std::vector<Polygon*> polygons;		// Initial polygon soup
		AABB initBound;						// Initial bound
		AABB bound01;						// Bound for the first mesh
		AABB bound02;						// Bound for the second mesh
		BSPTree bspTree;

		int mode;							// 0 Union, 1 Intersection, 2 Difference
	};
}

