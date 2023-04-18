#pragma once
#include <vector>
#include <queue>
#include "BigInt.hpp"
#include "BigFloat.h"

namespace ember
{
	struct ivec3
	{
		BigInt x, y, z;

		BigInt operator[](int i)
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
			BigInt i = a.y * b.z - a.z * b.y;
			BigInt j = a.z * b.x - a.x * b.z;
			BigInt k = a.x * b.y - a.y * b.x;
			return ivec3{i, j, k};
		}

		static BigInt dot(const ivec3& a, const ivec3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
	};

	struct Point
	{
		// Homogenerous coordinate (obtained from intersect())
		BigInt x1, x2, x3, x4;

		Point(BigInt xx1, BigInt xx2, BigInt xx3, BigInt xx4)
		{
			x1 = xx1;
			x2 = xx2;
			x3 = xx3;
			x4 = xx4;
		}

		bool isValid()
		{
			return x4 != 0;
		}

		ivec3 getPosition()
		{
			// Cramer's rule

			return ivec3{ BigInt(x1 / x4), BigInt(x2 / x4), BigInt(x3 / x4) };
		}
	};
	struct AABB
	{
		ivec3 min, max;
	};
	
	struct RefPoint
	{
		ivec3 pos;
		std::vector<int> WNV;
	};

	struct Plane
	{
		// ax + by + cz = d (This is the representation in Cramer's rule
		BigInt a, b, c, d;

		Plane(BigInt aa, BigInt bb, BigInt cc, BigInt dd)
		{
			a = aa;
			b = bb;
			c = cc; 
			d = dd;
		}

		Plane()
		{
			// Default constructor
		}

		// Get non-normalized normal
		ivec3 getNormal()
		{
			return { a, b, c };
		}

		Plane flip()
		{
			return Plane(-a, -b, -c, -d);
		}

		static Plane fromPositionNormal(ivec3 p, ivec3 nor)
		{
			// Cramer's rule
			return Plane (nor.x, nor.y, nor.z,
				(nor.x * p.x + nor.y * p.y + nor.z * p.z));
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
	};
}