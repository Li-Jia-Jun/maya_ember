#pragma once
#include <vector>
#include <queue>
#include "boost/multiprecision/cpp_int.hpp"
#include "boost/multiprecision/cpp_bin_float.hpp"

namespace ember
{
	using namespace boost::multiprecision;

	struct ivec3
	{
		int256_t x, y, z;

		int256_t operator[](int i)
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

		ivec3 operator*(int256_t i)
		{
			return ivec3{ x * i, y * i, z * i };
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
			int256_t i = a.y * b.z - a.z * b.y;
			int256_t j = a.z * b.x - a.x * b.z;
			int256_t k = a.x * b.y - a.y * b.x;
			return ivec3{i, j, k};
		}

		static int256_t dot(const ivec3& a, const ivec3& b)
		{
			return a.x * b.x + a.y * b.y + a.z * b.z;
		}
	};

	struct Point
	{
		// Homogenerous coordinate (obtained from intersect())
		int256_t x1, x2, x3, x4;

		Point(int256_t xx1, int256_t xx2, int256_t xx3, int256_t xx4)
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
			return ivec3{ int256_t(x1 / x4), int256_t(x2 / x4), int256_t(x3 / x4) };
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
		int256_t a, b, c, d;

		Plane(int256_t aa, int256_t bb, int256_t cc, int256_t dd)
		{
			//if (aa % 100 == 0 && bb % 100 == 0 && cc % 100 == 0 && dd % 100 == 0)
			//{
			//	aa = aa / 100;
			//	bb = bb / 100;
			//	cc = cc / 100;
			//	dd = dd / 100;
			//}
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

		static Plane fromPointNormal(Point p, ivec3 nor)
		{
			return Plane(nor.x, nor.y, nor.z,
				(nor.x * p.x1 + nor.y * p.x2, nor.z * p.x3));
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
	private:
		Segment();

	public:
		Segment(Line line, Plane b1, Plane b2);
		Line line;
		Plane bound1, bound2; // Start from bound1 and end at bound2
		Point p1, p2;
	};

	struct Polygon
	{
	private:
		Polygon();

	public:
		Polygon(int meshId, Plane support, std::vector<Plane> bounds);

		int meshId;
		Plane support;
		std::vector<Plane> bounds;

		std::vector<Point> points;		// Cache for reuse
		AABB aabb;						// Cache for reuse
	};
}