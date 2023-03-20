#pragma once

#include <vector>

namespace ember
{
	struct ivec3
	{
		int x, y, z;

		int operator[](int i)
		{
			if (i == 0) return x;
			else if (i == 1) return y;
			else if (i == 2) return z;
			else return -1;
		}

		ivec3 operator+(const ivec3& b)
		{
			return ivec3{ x + b.x, y + b.y, z + b.z };
		}

		ivec3 operator-(const ivec3& b) 
		{
			return ivec3{ x - b.x, y - b.y, z - b.z };
		}

		ivec3 operator*(const ivec3& b)
		{
			return ivec3{ x * b.x, y * b.y, z * b.z };
		}

		float length()
		{
			return sqrt(x * x + y * y + z * z);
		}

		ivec3 cross(const ivec3& b)
		{
			int i = y * b.z - z * b.y;
			int j = z * b.x - x * b.z;
			int k = x * b.y - y * b.x;
			return ivec3{i, j, k};
		}
	};

	struct Point
	{
		// Homogenerous coordinate (obtained from intersect())
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

		static Plane fromPositionNormal(ivec3 p, ivec3 nor)
		{
			return Plane {nor.x, nor.y, nor.z,
				-(nor.x * p.x + nor.y * p.y + nor.z * p.z)};
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
		std::vector<Plane> bounds;
	};

	struct Mesh
	{
		int id; // Id also indicates its index in WNTV(delta w)
		std::vector<Polygon> polygons;
	};
}


