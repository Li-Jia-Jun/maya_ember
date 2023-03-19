#pragma once

#include "util.h"
#include <vector>

namespace ember
{
	struct ivec3
	{
		int x, y, z;
	};

	struct Point
	{
		// Homogenerous coordinate (obtained from interset())
		int x1, x2, x3, x4;
	};

	struct Plane
	{
		// ax + by + cz + d = 0
		int a, b, c, d;

		ivec3 normal()
		{
			return { a, b, c };
		}
	};

	struct Line
	{
		Plane p1, p2;
	};

	struct Segment
	{
		Line line;
		Plane bound1, bound2;
	};

	struct Polygon
	{
		int meshId;
		Plane support;
		std::vector<Segment> segments;
	};

	class Mesh
	{
		int id; // Id also indicates its index in WNTV(delta w)
		std::vector<Polygon> polygons;
	};
}


