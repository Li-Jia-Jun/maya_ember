#include "geometry.h"
#include "util.h"

using namespace ember;

Polygon::Polygon(int meshId, Plane support, std::vector<Plane> bounds) : 
	meshId(meshId), support(support), bounds(bounds)
{
	int count = bounds.size();
	for (int i = 0; i < count; i++)
	{
		// Cache points
		Point point = intersect(support, bounds[i], bounds[(i + 1) % count]);
		points.push_back(point);
		
		// Compute AABB
		ivec3 pos = point.getPosition();

		if (i == 0)
		{
			aabb.min = pos;
			aabb.max = pos;
			continue;
		}

		if (pos.x < aabb.min.x)
		{
			aabb.min.x = pos.x;
		}
		if (pos.y < aabb.min.y)
		{
			aabb.min.y = pos.y;
		}
		if (pos.z < aabb.min.z)
		{
			aabb.min.z = pos.z;
		}
		if (pos.x > aabb.max.x)
		{
			aabb.max.x = pos.x;
		}
		if (pos.y > aabb.max.y)
		{
			aabb.max.y = pos.y;
		}
		if (pos.z > aabb.max.z)
		{
			aabb.max.z = pos.z;
		}
	}

	int256_t offset(AABB_OFFSET);
	aabb.min = aabb.min - ivec3{ offset, offset, offset };
	aabb.max = aabb.max + ivec3{ offset, offset, offset };
}

Segment::Segment(Line line, Plane b1, Plane b2) :
	line(line), bound1(b1), bound2(b2), p1(intersect(line.p1, line.p2, b1)), p2(intersect(line.p1, line.p2, b2))
{
	
}