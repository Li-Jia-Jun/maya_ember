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

	bool isPlaneEqual(Plane p1, Plane p2);

	bool isPointInPolygon(Polygon* polygon, Point point);

	int getCloestAxis(ivec3 dir);

	Line getAxisLine(ivec3 pos, int axis);

	Point getPointfromPosition(ivec3 pos);

	Point getPolygonPoint(Polygon* polygon, int index);

	Segment getPolygonSegment(Polygon* polygon, int index);

	/// <summary>
	/// Get polygon center of mass and rounded to integer coordinate
	/// </summary>
	ivec3 getRoundedPolygonCOM(Polygon* polygon);

	/// <summary>
	/// Create a segment along the given axis direction
	/// </summary>
	Segment getAxisSegmentFromPositions(ivec3 stPos, ivec3 edPos, int axis);

	/// <summary>
	/// Split the polygon with a plane and create two new polygons
	/// - the bounds of new polygons are in order
	/// - the original polygon does not change
	/// </summary>
	std::pair<Polygon*, Polygon*> splitPolygon(Polygon* polygon, Plane splitPlane);

	/// <summary>
	/// Find exclusive intersection between line and polygon
	/// The result can be a point or no intersection (indicated by point.x4 == 0)
	///  - exclusive means points just on the edges are not considered valid
	/// </summary>
	Point intersectLinePolygon(Polygon* polygon, Line line);

	/// <summary>
	/// Find exclusive intersection between segment and polygon
	///  - points on the end point of the segment are not considered valid
	/// </summary>
	Point intersectSegmentPolygon(Polygon* polygon, Segment segment);
	

	Point intersect(Plane p, Plane q, Plane r);

	int classify(Point x, Plane s);
}

