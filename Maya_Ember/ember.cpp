#include "ember.h"

using namespace ember;

EMBER::EMBER(std::vector<Mesh>& inputs)
{
	for (int i = 0; i < inputs.size(); i++)
	{
		meshes.push_back(inputs[i]);
	}
}

void EMBER::BuildBSPTree()
{
	// Build root node
	AABB initBound = GetMeshesBound();
	BSPNode root;
	root.bound = initBound;
	
	// Build tree recursively

}

AABB EMBER::GetMeshesBound()
{
	// LEO:: compute a bounding box for all meshes

	return AABB();
}