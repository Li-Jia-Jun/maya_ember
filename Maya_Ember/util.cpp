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

bool ember::isDirectionEqual(ivec3 dir1, ivec3 dir2)
{
	ivec3 crossProduct = ivec3::cross(dir1, dir2);
	return crossProduct.x == 0 && crossProduct.y == 0 && crossProduct.z == 0;
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


bool ember::isPlaneEqual(Plane p1, Plane p2)
{
	ivec3 nor1 = p1.getNormal();
	ivec3 nor2 = p2.getNormal();

	if (!isDirectionEqual(nor1, nor2))
		return false;

	if (p1.d == 0)
		return true;

	if (p1.a != 0)
		return p2.a / p1.a == p2.d / p1.d;
	else if (p1.b != 0)
		return p2.b / p1.b == p2.d / p1.d;
	else if (p1.c != 0)
		return p2.c / p1.c == p2.d / p1.d;
	else
		return false;
}

bool ember::isPointInPolygon(Polygon* polygon, Point point)
{
	for (int i = 0; i < polygon->bounds.size(); i++)
	{
		int c = classify(point, polygon->bounds[i]);
		if (c >= 0)	// Points on polygon edge are also considered 'exterior'
		{
			return false;
		}
	}
	return true;
}

int ember::getCloestAxis(ivec3 dir)
{
	int x = abs(dir.x);
	int y = abs(dir.y);
	int z = abs(dir.z);

	if (x > y && x > z)
	{
		return 0;
	}
	else if (y > x && y > z)
	{
		return 1;
	}
	else
	{
		return 2;
	}
}

Line ember::getAxisLine(ivec3 pos, int axis)
{
	Line line;
	switch (axis)
	{
	case 0:
		line.p1 = Plane::fromPositionNormal(pos, ivec3{ 0, 1, 0 });
		line.p2 = Plane::fromPositionNormal(pos, ivec3{ 0, 0, 1 });
		break;
	case 1:
		line.p1 = Plane::fromPositionNormal(pos, ivec3{ 1, 0, 0 });
		line.p2 = Plane::fromPositionNormal(pos, ivec3{ 0, 0, 1 });
		break;
	case 2:
		line.p1 = Plane::fromPositionNormal(pos, ivec3{ 1, 0, 0 });
		line.p2 = Plane::fromPositionNormal(pos, ivec3{ 0, 1, 0 });
		break;
	default:
		break;
	}
	return line;
}

Point ember::getPointfromPosition(ivec3 pos)
{
	// Build point as intersection of three axis planes
	Plane xy{ 0, 0, 1, -pos.z };
	Plane yz{ 1, 0, 0, -pos.x };
	Plane zx{ 0, 1, 0, -pos.y };
	return intersect(xy, yz, zx);
}

Point ember::getPolygonPoint(Polygon* polygon, int index)
{
	int pointCount = polygon->bounds.size();
	Plane plane1 = polygon->bounds[index];
	Plane plane2 = polygon->bounds[(index+1) % pointCount];
	return intersect(plane1, plane2, polygon->support);
}

Segment ember::getPolygonSegment(Polygon* polygon, int index)
{
	int edgeCount = polygon->bounds.size();
	Plane plane1 = polygon->bounds[(index - 1 + edgeCount) % edgeCount];
	Plane plane2 = polygon->bounds[index];
	Plane plane3 = polygon->bounds[(index + 1) % edgeCount];

	// If the convex polygon can guarantee bounding plane orient outward,
	// the new segment bounding plane will also orient outward
	return Segment{ Line{polygon->support, plane2}, plane1, plane3 };
}

Segment ember::getSegmentfromPlanes(Plane plane1, Plane plane2, Plane bound1, Plane bound2)
{
	Point st = intersect(plane1, plane2, bound1);
	Point ed = intersect(plane1, plane2, bound2);
	ivec3 dir = ed.getPosition() - st.getPosition();

	// Make sure both bounds orient outside
	if (ivec3::dot(bound1.getNormal(), dir) > 0)
	{
		bound1 = Plane{-bound1.a, -bound1.b, -bound1.c, -bound1.d }; 
	}
	if (ivec3::dot(bound2.getNormal(), dir) < 0)
	{
		bound2 = Plane{ -bound2.a, -bound2.b, -bound2.c, -bound2.d };
	}

	return Segment{ Line{plane1, plane2}, bound1, bound2 };
}

ivec3 ember::getRoundedPolygonCOM(Polygon* polygon)
{
	ivec3 accumulate{ 0, 0, 0 };

	int pointCount = polygon->bounds.size();
	for (int i = 0; i < pointCount; i++)
	{
		Point point = getPolygonPoint(polygon, i);
		accumulate = accumulate + point.getPosition();
	}

	return ivec3{ accumulate.x / pointCount, accumulate.y / pointCount, accumulate.z / pointCount };
}

Segment ember::getAxisSegmentFromPositions(ivec3 stPos, ivec3 edPos, int axis)
{
	Line line;
	Segment segment;

	switch (axis)
	{
	case 0:	// X axis (Line planes are XY, XZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 0, 1, 0 });

		// Make sure all bound planes orient outside
		if (edPos.x > stPos.x)
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ -1, 0, 0 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ 1, 0, 0 });
		}
		else
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ 1, 0, 0 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ -1, 0, 0 });
		}
		break;
	case 1:	// Y axis (Line planes are XY, YZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 1, 0, 0 });

		if (edPos.y > stPos.y)
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, -1, 0 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, 1, 0 });
		}
		else
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 1, 0 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, -1, 0 });
		}
		break;
	case 2:	// Z axis (Line planes are XZ, YZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 1, 0 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 1, 0, 0 });

		if (edPos.z > stPos.z)
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, -1 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, 0, 1 });
		}
		else
		{
			segment.bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, 1 });
			segment.bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, 0, -1 });
		}
		break;
	default:
		break;
	}

	segment.line = line;

	return segment;
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

				// Since split plane orients to right side, we need to flip it
				// to make sure all bound planes orient outside
				Plane flippedSplitPlane = Plane{-splitPlane.a, -splitPlane.b, -splitPlane.c, -splitPlane.d};
				rightEdgePlanes.push_back(flippedSplitPlane);
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

Point ember::intersectLinePolygon(Polygon* polygon, Line line)
{
	Point x = intersect(polygon->support, line.p1, line.p2);

	// If intersection is not unique
	if (x.x4 == 0)
	{
		return x;
	}

	// If the intersection point is not inside polygon
	if (!isPointInPolygon(polygon, x))
	{
		x.x4 = 0;
	}

	return x;
}

Point ember::intersectSegmentPolygon(Polygon* polygon, Segment segment)
{
	Point x = intersectLinePolygon(polygon, segment.line);

	// If line intersection is invalid 
	if (x.x4 == 0)
	{
		return x;
	}

	// If intersection point is not within segment
	int c1 = classify(x, segment.bound1);
	int c2 = classify(x, segment.bound2);
	if (c1 >= 0 || c2 >= 0) // Points right on the segment end are considered 'exterior'
	{
		x.x4 = 0; 
	}

	return x;
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
