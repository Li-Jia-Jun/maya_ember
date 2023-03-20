#pragma once

#include "bsp.h"


namespace ember
{
	class EMBER
	{
	public:
		EMBER(std::vector<Mesh>& inputs);

		void BuildBSPTree();
		
		AABB GetMeshesBound();

	private:
		std::vector<Mesh> meshes; // All the meshes in this EMBER operation
		BSPTree bspTree;		
		
	};
}

