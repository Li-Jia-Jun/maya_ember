#pragma once
#include "geometry.h"
#include <utility>

#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MString.h>
#include <maya/MFnTransform.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MPlug.h>


namespace ember
{
	// ======== Configure =============
	
#define LEAF_POLYGON_COUNT (6)

#define GLOBAL_BSP_NODE_COUNT (7)

#define AABB_OFFSET "100000000000"

#define POSITION_CLOSE "1000"

#define POINT_AABB ivec3{BigInt("100000000000"), BigInt("100000000000"), BigInt("100000000000")}

#define BIG_NUM_STR "10000000000000"


	// ==================================


	// ======== Basic Math =============

	BigInt multiply(Point x, Plane s);

	int sign(BigInt value);

	BigInt determiant3x3(
		BigInt a, BigInt b, BigInt c,
		BigInt d, BigInt e, BigInt f,
		BigInt g, BigInt h, BigInt i);

	bool isDirectionEqual(ivec3 dir1, ivec3 dir2);

	bool isPointEqual(Point p1, Point p2);

	BigInt bigFloatToBigInt(BigFloat f);


	// ======== Mesh Operation =============

	bool isPlaneEqual(Plane p1, Plane p2);

	bool isPointInPolygon(Polygon* polygon, Point point);

	bool isAABBIntersect(AABB& a, AABB& b);

	bool isSegmentInBound(Segment& segment, AABB& bound);

	int getCloestAxis(ivec3 dir);

	Line getAxisLine(ivec3 pos, int axis);

	Point getPointfromPosition(ivec3 pos);

	AABB getSegmentAABB(Segment& segment);

	Segment getPolygonSegment(Polygon* polygon, int index);

	/// <summary>
	/// Get polygon center of mass and rounded to integer coordinate
	/// </summary>
	ivec3 getRoundedPolygonCOM(Polygon* polygon);

	/// <summary>
	/// Create a segment along the given axis direction
	/// </summary>
	Segment getAxisSegmentFromPositions(ivec3 stPos, ivec3 edPos, int axis);

	std::vector<int> TraceSegment(std::vector<Polygon*> polygons, Segment segment, std::vector<int> WNV, int selfMark = -1);

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

	Polygon* fromPositionNormal(std::vector<ivec3>, ivec3, int);

	void printStr(const char*);
	void printVector(std::vector<int> vec);
	void printNum(BigInt);
	void printIvec3(ivec3);
	void printPoint(Point);
	void printPlane(Plane);
	void printSegment(ember::Segment);
	void printPolygon(Polygon*);
	void drawPolygon(Polygon*);
	void drawPolygons(std::vector<Polygon*> p);
	void drawBoundingBox(AABB);
	void drawPosition(ivec3);
	void drawSegment(Segment s);
}


