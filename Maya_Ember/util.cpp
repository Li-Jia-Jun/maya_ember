#include "util.h"

using namespace ember;


int ember::multiply(Point x, Plane s)
{
	// Not sure if this works
	return x.x1 * s.a + x.x2 * s.b + x.x3 * s.c + x.x4 * s.d;
}

int ember::sign(int value)
{
	if (value > 0) return 1;
	else if (value < 0) return -1;
	else return 0;
}

int ember::determiant3x3(
	int a, int b, int c,
	int d, int e, int f,
	int g, int h, int i)
{
	int t1 = a * (e * i - f * h);
	int t2 = b * (d * i - f * g);
	int t3 = c * (d * h - e * g);
	return t1 - t2 + t3;
}

bool ember::collinear(ivec3 a, ivec3 b, ivec3 c)
{
	// Heron's formula

	ivec3 ea = a - b;
	ivec3 eb = b - c;
	ivec3 ec = c - a;

	float la = ea.length();
	float lb = eb.length();
	float lc = ec.length();

	float s = (la + lb + lc) * 0.5f;

	if ((abs(s - la) < 0.0001) || (abs(s - lb) < 0.0001) || (abs(s - lc) < 0.0001))
		return true;
	else
		return true;
}



Plane ember::getPlaneFromPositions(ivec3 p1, ivec3 p2, ivec3 p3)
{
	ivec3 e1 = p1 - p2;
	ivec3 e2 = p1 - p3;

	ivec3 nor = e1.cross(e2);

	return Plane::fromPositionNormal(p3, nor);
}

Polygon ember::getPlaneBasedPolygon(std::vector<ivec3> posVec, ivec3 normal, int meshId)
{
	// Compute support plane
	ivec3 p0 = posVec[0];
	Plane support = Plane::fromPositionNormal(p0, normal);

	// Compute bound plane for each edge
	// (assume that bound plane is perpendicular to support plane)
	std::vector<Plane> bounds;
	for (int i = 0; i < posVec.size() - 1; i++)
	{
		ivec3 p1 = posVec[i];
		ivec3 p2 = posVec[i + 1];
		ivec3 p3 = p1 + normal; // p3 is outside support plane
		bounds.push_back(getPlaneFromPositions(p1, p2, p3));
	}

	return Polygon{ meshId, support, bounds };
}

Point ember::intersect(Plane p, Plane q, Plane r)
{
	int x1 = determiant3x3(
		p.d, p.b, p.c,
		q.d, q.b, q.c,
		r.d, r.b, r.c);

	int x2 = determiant3x3(
		p.a, p.d, p.c,
		q.a, q.d, q.c,
		r.a, r.d, r.c);

	int x3 = determiant3x3(
		p.a, p.b, p.d,
		q.a, q.b, q.d,
		r.a, r.b, r.d);

	int x4 = determiant3x3(
		p.a, p.b, p.c,
		q.a, q.b, q.c,
		r.a, r.b, r.c);

	return { x1, x2, x3, x4 };
}

int ember::classify(Point x, Plane s)
{
	// Return 1 means x on the positive side of s
	// Return -1 means x on the negative side of s
	// Return 0 means x on s	
	return sign(multiply(x, s)) * sign(x.x4);
}

