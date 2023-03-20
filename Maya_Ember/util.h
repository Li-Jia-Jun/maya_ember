#pragma once

#include "geometry.h"

namespace ember
{

	// ======== Basic Math =============

	int multiply(Point x, Plane s);

	int sign(int value);

	int determiant3x3(
		int a, int b, int c,
		int d, int e, int f,
		int g, int h, int i);

	bool collinear(ivec3 a, ivec3 b, ivec3 c);

	// ==================================



	// ======== Mesh Operation =============

	Plane getPlaneFromPositions(ivec3 p1, ivec3 p2, ivec3 p3);

	Polygon getPlaneBasedPolygon(std::vector<ivec3> posVec, ivec3 normal, int meshId);

	Point intersect(Plane p, Plane q, Plane r);

	int classify(Point x, Plane s);

	// ================================== 
}

