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

//bool ember::collinear(ivec3 a, ivec3 b, ivec3 c)
//{
//	// Heron's formula
//
//	ivec3 ea = a - b;
//	ivec3 eb = b - c;
//	ivec3 ec = c - a;
//
//	float la = ea.length();
//	float lb = eb.length();
//	float lc = ec.length();
//
//	float s = (la + lb + lc) * 0.5f;
//
//	if ((abs(s - la) < 0.0001) || (abs(s - lb) < 0.0001) || (abs(s - lc) < 0.0001))
//		return true;
//	else
//		return true;
//}


Point ember::getPointFromVertexPos(ivec3 pos)
{
	// Build point as intersection of three axis planes

	Plane xy{ 0, 0, 1, -pos.z };
	Plane yz{ 1, 0, 0, -pos.x };
	Plane zx{ 0, 1, 0, -pos.y };

	return intersect(xy, yz, zx);
}

Segment ember::getAxisSegmentFromPositions(ivec3 pos1, ivec3 pos2, int axis)
{
	Line line;
	Segment segment;

	switch (axis)
	{
	case 0:	// X axis (Line planes are XY, XZ)
		line.p1 = Plane::fromPositionNormal(pos1, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(pos1, ivec3{ 0, 1, 0 });
		segment.bound1 = Plane::fromPositionNormal(pos1, ivec3{ 1, 0, 0 });
		segment.bound2 = Plane::fromPositionNormal(pos2, ivec3{ 1, 0, 0 });
		break;
	case 1:	// Y axis (Line planes are XY, YZ)
		line.p1 = Plane::fromPositionNormal(pos1, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(pos1, ivec3{ 1, 0, 0 });
		segment.bound1 = Plane::fromPositionNormal(pos1, ivec3{ 0, 1, 0 });
		segment.bound2 = Plane::fromPositionNormal(pos2, ivec3{ 0, 1, 0 });
		break;
	case 2:	// Z axis (Line planes are XZ, YZ)
		line.p1 = Plane::fromPositionNormal(pos1, ivec3{ 0, 1, 0 });
		line.p2 = Plane::fromPositionNormal(pos1, ivec3{ 1, 0, 0 });
		segment.bound1 = Plane::fromPositionNormal(pos1, ivec3{ 0, 0, 1 });
		segment.bound2 = Plane::fromPositionNormal(pos2, ivec3{ 0, 0, 1 });
		break;
	default:
		break;
	}

	segment.line = line;

	return segment;
}

Polygon ember::getPlaneBasedPolygon(std::vector<ivec3> posVec, ivec3 normal, int meshId)
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
		bounds.push_back(Plane::getPlaneFromTriangle(p1, p2, p3));
	}

	return Polygon{ meshId, support, bounds };
}

std::pair<Polygon*, Polygon*> ember::splitPolygon(Polygon* polygon, Plane splitPlane)
{
	std::vector<Plane> leftEdgePlanes;
	std::vector<Plane> rightEdgePlanes;
	bool hasSplit = false;

	// Divide edge by split plane
	int boundSize = polygon->bounds.size();
	for (int j = 0; j < boundSize; j++)
	{
		Plane edgePlane = polygon->bounds[j];
		Plane bound1 = polygon->bounds[(j - 1 + boundSize) % boundSize];
		Plane bound2 = polygon->bounds[(j + 1) % boundSize];

		int c1 = classify(intersect(polygon->support, edgePlane, bound1), splitPlane);
		int c2 = classify(intersect(polygon->support, edgePlane, bound2), splitPlane);

		if (c1 <= 0 && c2 <= 0)
		{
			// Edge to left (no positive values, including all zero)
			leftEdgePlanes.push_back(edgePlane);
		}
		else if (c1 >= 0 && c2 >= 0)
		{
			// Edge to right (all positive values)
			rightEdgePlanes.push_back(edgePlane);
		}
		else
		{
			// Split and add edge to both sides
			leftEdgePlanes.push_back(edgePlane);

			if (!hasSplit)
			{
				leftEdgePlanes.push_back(splitPlane);
				rightEdgePlanes.push_back(splitPlane);
				hasSplit = true;
			}

			rightEdgePlanes.push_back(edgePlane);
		}
	}

	// Collect divided edges to build polygons
	Polygon* leftPolygon = nullptr;
	Polygon* rightPolygon = nullptr;
	if (!leftEdgePlanes.empty())
	{
		leftPolygon = new Polygon();
		leftPolygon->meshId = polygon->meshId;
		leftPolygon->bounds = leftEdgePlanes;
		leftPolygon->support = polygon->support;
	}
	if (!rightEdgePlanes.empty())
	{
		rightPolygon = new Polygon();
		rightPolygon->meshId = polygon->meshId;
		rightPolygon->bounds = rightEdgePlanes;
		rightPolygon->support = polygon->support;
	}

	return std::make_pair(leftPolygon, rightPolygon);
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

