#include "util.h"

#include <string>

#include <maya/MGlobal.h>
#include <maya/MString.h>

using namespace ember;


long long int ember::multiply(Point x, Plane s)
{
	// Not sure if this works
	return x.x1 * s.a + x.x2 * s.b + x.x3 * s.c + x.x4 * s.d;
}

int ember::sign(long long int value)
{
	if (value > 0) return 1;
	else if (value < 0) return -1;
	else return 0;
}

long long int ember::determiant3x3(
	long long int a, long long int b, long long int c,
	long long int d, long long int e, long long int f,
	long long int g, long long int h, long long int i)
{
	long long int t1 = a * (e * i - f * h);
	long long int t2 = b * (d * i - f * g);
	long long int t3 = c * (d * h - e * g);
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
		long long int c = classify(point, polygon->bounds[i]);
		if (c >= 0)	// Points on polygon edge are also considered 'exterior'
		{
			return false;
		}
	}
	return true;
}

int ember::getCloestAxis(ivec3 dir)
{
	long long int x = abs(dir.x);
	long long int y = abs(dir.y);
	long long int z = abs(dir.z);

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
	Plane xy( 0, 0, 1, -pos.z );
	Plane yz( 1, 0, 0, -pos.x );
	Plane zx( 0, 1, 0, -pos.y );
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
		bound1 = Plane(-bound1.a, -bound1.b, -bound1.c, -bound1.d ); 
	}
	if (ivec3::dot(bound2.getNormal(), dir) < 0)
	{
		bound2 = Plane( -bound2.a, -bound2.b, -bound2.c, -bound2.d );
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

	//Point point = getPolygonPoint(polygon, 0);
	//char buffer[128];
	//sprintf_s(buffer, "Compute polygon com, point 0 pos = %i %i %i ", point.getPosition().x, point.getPosition().y, point.getPosition().z);
	//MString debug(buffer);
	//MGlobal::displayInfo(buffer);

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

		Point p1 = intersect(polygon->support, edgePlane, bound1);
		Point p2 = intersect(polygon->support, edgePlane, bound2);
		long long int c1 = classify(p1, splitPlane);
		long long int c2 = classify(p2, splitPlane);

		printStr("bound point 1 2:");
		printPoint(p1);
		printPoint(p2);
		//printPlane(bound1);
		//printPlane(bound2);

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
	long long int c1 = classify(x, segment.bound1);
	long long int c2 = classify(x, segment.bound2);
	if (c1 >= 0 || c2 >= 0) // Points right on the segment end are considered 'exterior'
	{
		x.x4 = 0; 
	}

	return x;
}

Point ember::intersect(Plane p, Plane q, Plane r)
{
	long long int x1 = determiant3x3(
		p.d, p.b, p.c,
		q.d, q.b, q.c,
		r.d, r.b, r.c);

	long long int x2 = determiant3x3(
		p.a, p.d, p.c,
		q.a, q.d, q.c,
		r.a, r.d, r.c);

	long long int x3 = determiant3x3(
		p.a, p.b, p.d,
		q.a, q.b, q.d,
		r.a, r.b, r.d);

	long long int x4 = determiant3x3(
		p.a, p.b, p.c,
		q.a, q.b, q.c,
		r.a, r.b, r.c);

	return Point(x1, x2, x3, x4);
}

long long int ember::classify(Point x, Plane s)
{
	// Return 1 means x on the positive side of s
	// Return -1 means x on the negative side of s
	// Return 0 means x on s	
	return sign(multiply(x, s)) * sign(x.x4);
}

Polygon* ember::fromPositionNormal(std::vector<ivec3> posVec, ivec3 normal, int meshId)
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
		ivec3 edgeDir = p2 - p1;

		// If the vertex order follows the right hand rule
		// the calculated bound plane normal will orient outside
		ivec3 nor = ivec3::cross(support.getNormal(), edgeDir);
		bounds.push_back(Plane::fromPositionNormal(p1, nor));
	}

	return new Polygon{ meshId, support, bounds };
}

void ember::printStr(const char* str)
{
	char buffer[128];
	sprintf_s(buffer, str);
	MGlobal::displayInfo(buffer);
}

void ember::printNum(long long int n)
{
	char buffer[32];
	sprintf_s(buffer, "num: %lld", n);
	MGlobal::displayInfo(buffer);
}

void ember::printIvec3(ember::ivec3 v)
{
	char buffer[128];
	sprintf_s(buffer, "ivec3: %lld %lld %lld", v.x, v.y, v.z);
	MGlobal::displayInfo(buffer);
}

void ember::printPoint(ember::Point p)
{
	char buffer[128];
	sprintf_s(buffer, "x1: %lld x2: %lld x3: %lld x4: %lld", p.x1, p.x2, p.x3, p.x4);
	MGlobal::displayInfo(buffer);
}

void ember::printPlane(ember::Plane p)
{
	char buffer[128];
	sprintf_s(buffer, "a: %lld b: %lld c: %lld d: %lld", p.a, p.b, p.c, p.d);
	MGlobal::displayInfo(buffer);
}

void ember::printPolygon(ember::Polygon p)
{
	char buffer[128];
	sprintf_s(buffer, "support plane: ");
	MGlobal::displayInfo(buffer);
	ember::printPlane(p.support);

	sprintf_s(buffer, "bounds: ");
	MGlobal::displayInfo(buffer);
	for (int i = 0; i < p.bounds.size(); i++)
	{
		ember::printPlane(p.bounds[i]);
	}
}

void ember::drawPolygon(Polygon* p)
{
	int numVerts = p->bounds.size();
	//printNum(numVerts);

	MPointArray vertices;
	MIntArray vertCount;
	MIntArray vertList;
	vertCount.append(numVerts);
	//printNum(vertCount[0]);

	for (int i = 0; i < numVerts; i++)
	{
		Point vert = getPolygonPoint(p, i);
		//printPoint(vert);

		if (vert.x4 == 0)
		{
			printStr("detect x4 == 0, change it to 1");
			vert.x4 = 1;
		}

		ivec3 vertPos = vert.getPosition();
		//printIvec3(vertPos);
		vertices.append(MPoint((float)vertPos.x / BIG_NUM, (float)vertPos.y / BIG_NUM, (float)vertPos.z / BIG_NUM));
		vertList.append(i);
		//printNum(i);
	}

	MFnTransform transformFn;
	MFnMesh meshFn;

	MObject transformObj = transformFn.create();
	transformFn.setName("Polygon0");

	MObject meshObj = meshFn.create(
		numVerts,	// num of verts
		1,	// num of polygons
		vertices,	// vert pos array
		vertCount,	// polygon count array
		vertList,	// polygon connects
		transformObj	//parent object
	);
	MString shapeName = transformFn.name() + "Shape";
	
	meshFn.setName(shapeName);
	MGlobal::executeCommand("sets -add initialShadingGroup " + shapeName + ";");
}

void ember::drawBoundingBox(AABB boundingBox)
{
	static int count = 0;

	float maxX = (float)boundingBox.max.x / BIG_NUM;
	float maxY = (float)boundingBox.max.y / BIG_NUM;
	float maxZ = (float)boundingBox.max.z / BIG_NUM;
	float minX = (float)boundingBox.min.x / BIG_NUM;
	float minY = (float)boundingBox.min.y / BIG_NUM;
	float minZ = (float)boundingBox.min.z / BIG_NUM;

	MPointArray vertices;
	vertices.append(MPoint(minX, maxY, maxZ));
	vertices.append(MPoint(maxX, maxY, maxZ));
	vertices.append(MPoint(maxX, minY, maxZ));
	vertices.append(MPoint(minX, minY, maxZ));
	vertices.append(MPoint(minX, maxY, minZ));
	vertices.append(MPoint(maxX, maxY, minZ));
	vertices.append(MPoint(maxX, minY, minZ));
	vertices.append(MPoint(minX, minY, minZ));

	// num of verts for each face
	MIntArray vertCount;
	for (int i = 0; i < 6; i++)
	{
		vertCount.append(4);
	}

	int faceConnectsArray[] = { 3, 2, 1, 0, 2, 6, 5, 1, 6, 7, 4, 5, 7, 3, 0, 4, 0, 1, 5, 4, 7, 6, 2, 3 };
	MIntArray vertList(faceConnectsArray, 24);

	MFnMesh meshFn;

	MFnTransform transformFn;
	MObject transformObj = transformFn.create();
	std::string transformName = "BoundingBox";
	transformName.append(std::to_string(count));
	transformFn.setName(transformName.c_str());


	MObject meshObj = meshFn.create(
		vertices.length(),	// num of verts
		vertCount.length(),	// num of polygons
		vertices,	// vert pos array
		vertCount,	// polygon count array
		vertList,	// polygon connects
		transformObj	//parent object
	);

	std::string meshName = "BoxShape";
	meshName.append(std::to_string(count));
	meshFn.setName(meshName.c_str());
	std::string commandStr = "sets -add initialShadingGroup ";
	commandStr.append(meshName);
	commandStr.append(";");
	MGlobal::executeCommand(commandStr.c_str());

	MFnDependencyNode meshNode(meshObj);
	MPlug overrideEnabledPlug = meshNode.findPlug("overrideEnabled", true);
	overrideEnabledPlug.setValue(1);
	MPlug overrideShadingPlug = meshNode.findPlug("overrideShading", true);
	overrideShadingPlug.setValue(0);

	count++;
}