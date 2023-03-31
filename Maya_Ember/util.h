#pragma once

#include "geometry.h"
#include <utility>

namespace ember
{
	// ======== Configure =============
	
#define LEAF_POLYGON_COUNT (25)

#define AABB_ADJUST (10)

	// ==================================


	// ======== Basic Math =============

	int multiply(Point x, Plane s);

	int sign(int value);

	int determiant3x3(
		int a, int b, int c,
		int d, int e, int f,
		int g, int h, int i);

	bool isDirectionEqual(ivec3 dir1, ivec3 dir2);

	//bool collinear(ivec3 a, ivec3 b, ivec3 c);


	// ======== Mesh Operation =============


	Point getPointfromPosition(ivec3 pos);

	/// <summary>
	/// Create a segment along the given axis direction
	/// </summary>
	Segment getAxisSegmentFromPositions(ivec3 pos1, ivec3 pos2, int axis);

	/// <summary>
	/// Split the polygon with a plane and create two new polygons
	/// - the bounds of new polygons are in order
	/// - the original polygon does not change
	/// </summary>
	std::pair<Polygon*, Polygon*> splitPolygon(Polygon* polygon, Plane splitPlane);

	/// <summary>
	/// Find exclusive intersection between segmeng and polygon
	/// The result can be a point or no intersection (indicated by point.x4 == 0)
	///  - exclusive means points just on the edges or end points are not considered valid
	/// </summary>
	Point intersectSegmentPolygon(Polygon* polygon, Segment segment);

	Point intersect(Plane p, Plane q, Plane r);

	int classify(Point x, Plane s);
}

