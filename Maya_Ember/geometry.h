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

		bool operator==(const ivec3& b)
		{
			return x == b.x && y == b.y && z == b.z;
		}

		bool operator!=(const ivec3& b)
		{
			return !((*this) == b);
		}

		static ivec3 cross(const ivec3& a, const ivec3& b)
		{
			int i = a.y * b.z - a.z * b.y;
			int j = a.z * b.x - a.x * b.z;
			int k = a.x * b.y - a.y * b.x;
			return ivec3{i, j, k};
		}

		static int dot(const ivec3& a, const ivec3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
	};

	struct Point
	{
		// Homogenerous coordinate (obtained from intersect())
		int x1, x2, x3, x4;

		bool isValid()
		{
			return x4 != 0;
		}

		ivec3 getPosition()
		{
			// Cramer's rule
			return ivec3{ int(-x1 / x4), int(-x2 / x4), int(-x3 / x4) };
		}
	};

	struct Plane
	{
		// ax + by + cz + d = 0
		int a, b, c, d;

		// Get non-normalized normal
		ivec3 getNormal()
		{
			return { a, b, c };
		}

		static Plane fromPositionNormal(ivec3 p, ivec3 nor)
		{
			return Plane {nor.x, nor.y, nor.z,
				-(nor.x * p.x + nor.y * p.y + nor.z * p.z)};
		}

		static Plane fromTriangle(ivec3 p1, ivec3 p2, ivec3 p3)
		{
			ivec3 e1 = p1 - p2;
			ivec3 e2 = p1 - p3;

			ivec3 nor = ivec3::cross(e1, e2);

			return Plane::fromPositionNormal(p3, nor);
		}
	};

	struct Line
	{
		Plane p1, p2;
	};

	struct Segment
	{
		Line line;
		Plane bound1, bound2; // Start from bound1 and end at bound2
	};

	struct Polygon
	{
		int meshId;
		Plane support;
		std::vector<Plane> bounds;

		static Polygon* fromPositionNormal(std::vector<ivec3> posVec, ivec3 normal, int meshId)
		{
			// Compute support plane
			ivec3 p0 = posVec[0];
			Plane support = Plane::fromPositionNormal(p0, normal);

			// Compute bound plane for each edge
			// (assume that bound plane is perpendicular to support plane)
			std::vector<Plane> bounds;
			int count = posVec.size();
			for (int i = 0; i < count; i++)
			{
				ivec3 p1 = posVec[i];
				ivec3 p2 = posVec[(i + 1) % count];
				ivec3 p3 = p1 + normal; // p3 is outside support plane
				bounds.push_back(Plane::fromTriangle(p1, p2, p3));
			}

			return new Polygon{ meshId, support, bounds };
		}
	};
}


