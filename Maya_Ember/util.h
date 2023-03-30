#pragma once

#include "geometry.h"

namespace ember
{
	// ======== Configure =============
	
#define LEAF_POLYGON_COUNT (25)

	// ==================================


	// ======== Basic Math =============

	int multiply(Point x, Plane s);

	int sign(int value);

	int determiant3x3(
		int a, int b, int c,
		int d, int e, int f,
		int g, int h, int i);

	//bool collinear(ivec3 a, ivec3 b, ivec3 c);

	// ==================================


	// ======== Mesh Operation =============
	
	Point intersect(Plane p, Plane q, Plane r);

	Point getPointFromVertexPos(ivec3 pos);

	Segment getAxisSegmentFromPositions(ivec3 pos1, ivec3 pos2, int axis);

	Polygon getPlaneBasedPolygon(std::vector<ivec3> posVec, ivec3 normal, int meshId);

	int classify(Point x, Plane s);

	// ================================== 
}

