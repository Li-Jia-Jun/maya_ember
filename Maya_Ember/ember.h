#pragma once
#include "bsp.h"


namespace ember
{
	class EMBER
	{
	public:
		EMBER();

		void ReadMeshData(std::vector<std::vector<ivec3>>& vertices, std::vector<ivec3>& normals);

		void SetInitBound(AABB bound);

		void BuildBSPTree();


	private:
		int meshId;							// Incremental count of meshes
		std::vector<Polygon*> polygons;		// Initial polygon soup
		AABB initBound;						// Initial bound
		BSPTree bspTree;
	};
}

