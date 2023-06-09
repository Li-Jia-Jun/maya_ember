#include "util.h"

#include <string>

#include <maya/MGlobal.h>
#include <maya/MString.h>

using namespace ember;


int256_t ember::multiply(Point x, Plane s)
{
	// Flip the s.d here because the representation in paper is different from Cramer's rule
	// In Cramer's rule, we have ax + by + cz = d
	// In paper, we have ax + by + cz + d = 0
	return (x.x1 * s.a + x.x2 * s.b + x.x3 * s.c + x.x4 * (-s.d));
}

int ember::sign(int256_t value)
{
	if (value > int256_t(0)) return 1;
	else if (value < int256_t(0)) return -1;
	else return 0;
}

int256_t ember::determiant3x3(
	int256_t a, int256_t b, int256_t c,
	int256_t d, int256_t e, int256_t f,
	int256_t g, int256_t h, int256_t i)
{
	int256_t t1 = a * (e * i - f * h);
	int256_t t2 = b * (d * i - f * g);
	int256_t t3 = c * (d * h - e * g);
	return t1 - t2 + t3;
}

bool ember::isDirectionEqual(ivec3 dir1, ivec3 dir2)
{
	ivec3 crossProduct = ivec3::cross(dir1, dir2);
	return crossProduct.x == 0 && crossProduct.y == 0 && crossProduct.z == 0;
}

bool ember::isPointEqual(Point p1, Point p2)
{
	int256_t close(POSITION_CLOSE);
	bool b1 = abs(p1.x1 * p2.x4 - p2.x1 * p1.x4) < close;
	bool b2 = abs(p1.x2 * p2.x4 - p2.x2 * p1.x4) < close;
	bool b3 = abs(p1.x3 * p2.x4 - p2.x3 * p1.x4) < close;
	return b1 && b2 && b3;
}


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

bool ember::isAABBIntersect(AABB& a, AABB& b)
{
	return (a.min.x < b.max.x && a.max.x > b.min.x) &&
		(a.min.y < b.max.y && a.max.y > b.min.y) &&
		(a.min.z < b.max.z && a.max.z > b.min.z);
}

bool ember::isPointInPolygon(Polygon* polygon, Point point)
{
	bool outside = false;
	for (int i = 0; i < polygon->bounds.size(); i++)
	{
		int c = classify(point, polygon->bounds[i]);
		if (c > 0)	// Points on polygon edge are considered 'inside'
		{
			outside = true;
			break;
		}
	}

	if (outside) return false;
	return true;
}

int ember::getCloestAxis(ivec3 dir)
{
	int256_t x = abs(dir.x);
	int256_t y = abs(dir.y);
	int256_t z = abs(dir.z);

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

AABB ember::getSegmentAABB(Segment& segment)
{
	ivec3 pos1 = segment.p1.getPosition();
	ivec3 pos2 = segment.p2.getPosition();
	int256_t minX = pos1.x < pos2.x ? pos1.x : pos2.x;
	int256_t minY = pos1.y < pos2.y ? pos1.y : pos2.y;
	int256_t minZ = pos1.z < pos2.z ? pos1.z : pos2.z;
	int256_t maxX = pos1.x > pos2.x ? pos1.x : pos2.x;
	int256_t maxY = pos1.y > pos2.y ? pos1.y : pos2.y;
	int256_t maxZ = pos1.z > pos2.z ? pos1.z : pos2.z;
	int256_t offset(AABB_OFFSET);
	return AABB{ ivec3{minX - offset, minY - offset, minZ - offset}, ivec3{maxX + offset, maxY + offset, maxZ + offset} };
}

Segment ember::getPolygonSegment(Polygon* polygon, int index)
{
	int edgeCount = polygon->bounds.size();
	Plane plane1 = polygon->bounds[(index - 1 + edgeCount) % edgeCount];
	Plane plane2 = polygon->bounds[index];
	Plane plane3 = polygon->bounds[(index + 1) % edgeCount];

	// If the convex polygon can guarantee bounding plane orient outward,
	// the new segment bounding plane will also orient outward
	return Segment( Line{polygon->support, plane2}, plane1, plane3 );
}

ivec3 ember::getRoundedPolygonCOM(Polygon* polygon)
{
	ivec3 accumulate{ 0, 0, 0 };

	int pointCount = polygon->bounds.size();
	for (int i = 0; i < pointCount; i++)
	{
		accumulate = accumulate + polygon->points[i].getPosition();
	}

	return ivec3{ accumulate.x / pointCount, accumulate.y / pointCount, accumulate.z / pointCount };
}

Segment ember::getAxisSegmentFromPositions(ivec3 stPos, ivec3 edPos, int axis)
{
	Line line;
	Plane bound1, bound2;

	switch (axis)
	{
	case 0:	// X axis (Line planes are XY, XZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 0, 1, 0 });
		// Make sure all bound planes orient outside
		bound1 = Plane::fromPositionNormal(stPos, ivec3{ -1, 0, 0 });
		bound2 = Plane::fromPositionNormal(edPos, ivec3{ 1, 0, 0 });
		break;
	case 1:	// Y axis (Line planes are XY, YZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, 1 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 1, 0, 0 });
		bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, -1, 0 });
		bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, 1, 0 });
		break;
	case 2:	// Z axis (Line planes are XZ, YZ)
		line.p1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 1, 0 });
		line.p2 = Plane::fromPositionNormal(stPos, ivec3{ 1, 0, 0 });
		bound1 = Plane::fromPositionNormal(stPos, ivec3{ 0, 0, -1 });
		bound2 = Plane::fromPositionNormal(edPos, ivec3{ 0, 0, 1 });;
	default:
		break;
	}

	return Segment(line, bound1, bound2);
}

std::vector<int> ember::TraceSegment(std::vector<Polygon*> polygons, Segment segment, std::vector<int> WNV, int selfMark)
{	
	AABB segmentAABB = getSegmentAABB(segment);

	std::vector<int> inPointMeshes;
	std::vector<int> outPointMeshes;
	std::vector<Point> inPoints;	// Temp solution to avoid counting twice when intersceting mutual line
	std::vector<Point> outPoints;	// of two polygons

	for (int i = 0; i < polygons.size(); i++)
	{
		if (i == selfMark)
			continue;

		Polygon* polygon = polygons[i];

		// If we can guarantee that input mesh does not have self folding
		if (selfMark >= 0 && polygon->meshId == polygons[selfMark]->meshId)
			continue;

		if (!isAABBIntersect(segmentAABB, polygon->aabb))
			continue;

		Point x = intersectSegmentPolygon(polygon, segment);
		if (x.isValid())
		{
			int c2 = classify(segment.p2, polygon->support);
			if (c2 > 0)
			{
				// Segment is going out
				bool diffPoint = true;
				for (int j = 0; j < outPoints.size(); j++)
				{
					if (isPointEqual(outPoints[j], x) && polygon->meshId == outPointMeshes[j])
					{
						diffPoint = false;
						break;
					}
				}
				if (diffPoint)
				{
					WNV[polygon->meshId] = WNV[polygon->meshId] - 1;
					outPoints.push_back(x);
					outPointMeshes.push_back(polygon->meshId);
				}
			}
			else if (c2 < 0)
			{
				// Segment is going in
				bool diffPoint = true;
				for (int j = 0; j < inPoints.size(); j++)
				{
					if (isPointEqual(inPoints[j], x) && polygon->meshId == inPointMeshes[j])
					{
						diffPoint = false;
						break;
					}
				}
				if (diffPoint)
				{
					WNV[polygon->meshId] = WNV[polygon->meshId] + 1;
					inPoints.push_back(x);
					inPointMeshes.push_back(polygon->meshId);
				}
			}
			else
			{
				// Segment lays on the polygon (impossible under our assumption?)
				printStr("ERROR: trace segment lies on the polygon!!");
			}
		}
	}

	return WNV;
}

std::pair<Polygon*, Polygon*> ember::splitPolygon(Polygon* polygon, Plane splitPlane)
{
	std::vector<Plane> leftEdgePlanes;
	std::vector<Plane> rightEdgePlanes;
	std::vector<int> leftZeroPointEdgeIndex;
	std::vector<int> rightZeroPointEdgeIndex;
	int zeroPointCount = 0;
	int edgeOnSplitCount = 0;
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
		int c1 = classify(p1, splitPlane);
		int c2 = classify(p2, splitPlane);

		// Keep track of zero points on the split plane
		if (c1 == 0)
		{
			leftZeroPointEdgeIndex.push_back(leftEdgePlanes.size());
			rightZeroPointEdgeIndex.push_back(rightEdgePlanes.size());
			zeroPointCount++;
		}
		if (c2 == 0)
		{
			leftZeroPointEdgeIndex.push_back(leftEdgePlanes.size());
			rightZeroPointEdgeIndex.push_back(rightEdgePlanes.size());
			zeroPointCount++;
		}

		// Edge lies on split plane
		if (c1 == 0 && c2 == 0)
		{
			leftEdgePlanes.push_back(edgePlane);
			rightEdgePlanes.push_back(edgePlane);
			edgeOnSplitCount++;
		}
		// Edge to left (no positive values)
		else if (c1 <= 0 && c2 <= 0)
		{	
			leftEdgePlanes.push_back(edgePlane);
		}
		// Edge to right (all positive values)
		else if (c1 >= 0 && c2 >= 0)
		{
			rightEdgePlanes.push_back(edgePlane);
		}
		else
		{
			if (!hasSplit)
			{
				hasSplit = true;

				if (c1 <= 0 && c2 >= 0)
				{
					leftEdgePlanes.push_back(edgePlane);
					leftEdgePlanes.push_back(splitPlane);

					rightEdgePlanes.push_back(splitPlane.flip());
					rightEdgePlanes.push_back(edgePlane);
				}
				else
				{
					leftEdgePlanes.push_back(splitPlane);
					leftEdgePlanes.push_back(edgePlane);

					rightEdgePlanes.push_back(edgePlane);
					rightEdgePlanes.push_back(splitPlane.flip());
				}
			}
			else
			{
				leftEdgePlanes.push_back(edgePlane);
				rightEdgePlanes.push_back(edgePlane);
			}
		}
	}

	// If we have 2 distinct points
	if (zeroPointCount - edgeOnSplitCount * 2 >= 4)
	{
		int leftInsert = leftZeroPointEdgeIndex[0] <= leftZeroPointEdgeIndex[1] ? 
			leftZeroPointEdgeIndex[0] : leftZeroPointEdgeIndex[1];
		int rightInsert = rightZeroPointEdgeIndex[0] >= rightZeroPointEdgeIndex[1] ?
			rightZeroPointEdgeIndex[0] : rightZeroPointEdgeIndex[1];

		leftEdgePlanes.insert(leftEdgePlanes.begin() + leftInsert, splitPlane);
		rightEdgePlanes.insert(rightEdgePlanes.begin() + rightInsert, splitPlane.flip());
	}

	// Collect divided edges to build polygons
	Polygon* leftPolygon = nullptr;
	Polygon* rightPolygon = nullptr;
	if (leftEdgePlanes.size() >= 2)
	{
		leftPolygon = new Polygon(polygon->meshId, polygon->support, leftEdgePlanes);
	}
	if (rightEdgePlanes.size() >= 2)
	{
		rightPolygon = new Polygon(polygon->meshId, polygon->support, rightEdgePlanes);
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
		if (x.x1 == 0 && x.x2 == 0 && x.x3 == 0) // When line parallel to the polygon
		{
			// Check if the segment has intersection with the polygon
			for (int i = 0; i < polygon->bounds.size(); i++)
			{
				Point tmp = intersect(segment.line.p1, segment.line.p2, polygon->bounds[i]);
				if (tmp.isValid())
					return tmp;
			}	
		}
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
	int256_t x1 = determiant3x3(
		p.d, p.b, p.c,
		q.d, q.b, q.c,
		r.d, r.b, r.c);

	int256_t x2 = determiant3x3(
		p.a, p.d, p.c,
		q.a, q.d, q.c,
		r.a, r.d, r.c);

	int256_t x3 = determiant3x3(
		p.a, p.b, p.d,
		q.a, q.b, q.d,
		r.a, r.b, r.d);

	int256_t x4 = determiant3x3(
		p.a, p.b, p.c,
		q.a, q.b, q.c,
		r.a, r.b, r.c);

	return Point(x1, x2, x3, x4);
}

int ember::classify(Point x, Plane s)
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
		ivec3 edgeDir = p1 - p2;

		// If the vertex order follows the right hand rule
		// the calculated bound plane normal will orient outside
		ivec3 nor = ivec3::cross(normal, edgeDir);
		bounds.push_back(Plane::fromPositionNormal(p1, nor));
	}

	return new Polygon{ meshId, support, bounds };
}

void ember::printStr(const char* str)
{
	char buffer[1024];
	sprintf_s(buffer, str);
	MGlobal::displayInfo(buffer);
}

void ember::printVector(std::vector<int> vec)
{
	printStr("vec: ");
	for (int i = 0; i < vec.size(); i++)
	{
		printNum(vec[i]);
	}
}

void ember::printNum(int256_t n)
{
	char buffer[1024];
	sprintf_s(buffer, "num: %s", n.str().c_str());
	MGlobal::displayInfo(buffer);
}

void ember::printIvec3(ember::ivec3 v)
{
	char buffer[1024];
	sprintf_s(buffer, "ivec3: %s %s %s", 
		v.x.str().c_str(), v.y.str().c_str(), v.z.str().c_str());
	MGlobal::displayInfo(buffer);
}

void ember::printPoint(ember::Point p)
{
	char buffer[1024];
	sprintf_s(buffer, "x1: %s x2: %s x3: %s x4: %s", 
		p.x1.str().c_str(), p.x2.str().c_str(), p.x3.str().c_str(), p.x4.str().c_str());
	MGlobal::displayInfo(buffer);
}

void ember::printPlane(ember::Plane p)
{
	char buffer[1024];
	sprintf_s(buffer, "a: %s b: %s c: %s d: %s", 
		p.a.str().c_str(), p.b.str().c_str(), p.c.str().c_str(), p.d.str().c_str());
	MGlobal::displayInfo(buffer);
}

void ember::printSegment(ember::Segment s)
{
	char buffer[1024];
	sprintf_s(buffer, "line: ");
	MGlobal::displayInfo(buffer);
	ember::printPlane(s.line.p1);
	ember::printPlane(s.line.p2);

	sprintf_s(buffer, "bounds: ");
	MGlobal::displayInfo(buffer);
	ember::printPlane(s.bound1);
	ember::printPlane(s.bound2);
}

void ember::printPolygon(ember::Polygon* p)
{
	char buffer[1024];
	sprintf_s(buffer, "support plane: ");
	MGlobal::displayInfo(buffer);
	ember::printPlane(p->support);

	sprintf_s(buffer, "bounds: ");
	MGlobal::displayInfo(buffer);
	for (int i = 0; i < p->bounds.size(); i++)
	{
		ember::printPlane(p->bounds[i]);
	}
}

void ember::drawPolygon(Polygon* p)
{
	if (p == nullptr)
	{
		printStr("FATAL: DRAWING AN EMPTY POLYGON");
		return;
	}
	int numVerts = p->bounds.size();

	MPointArray vertices;
	MIntArray vertCount;
	MIntArray vertList;
	vertCount.append(numVerts);

	for (int i = 0; i < numVerts; i++)
	{
		Point vert = p->points[i];

		if (vert.x4 == 0)
		{
			printStr("detect x4 == 0, change it to 1");
			vert.x4 = 1;
		}

		ivec3 vertPos = vert.getPosition();
		cpp_bin_float_100 x(vertPos.x);
		cpp_bin_float_100 y(vertPos.y);
		cpp_bin_float_100 z(vertPos.z);
		cpp_bin_float_100 div(BIG_NUM_STR);
		vertices.append(MPoint(
			(double)(x / div),
			(double)(y / div),
			(double)(z / div)));
		vertList.append(i);
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

void ember::drawPolygons(std::vector<Polygon*> p)
{
	MPointArray vertices;
	MIntArray vertCount;
	MIntArray vertList;
	int totalVerts = 0;

	for (int i = 0; i < p.size(); i++)
	{
		int numVerts = p[i]->bounds.size();

		vertCount.append(numVerts);

		for (int j = 0; j < numVerts; j++)
		{
			Point vert = p[i]->points[j];

			if (vert.x4 == 0)
			{
				printStr("detect x4 == 0, change it to 1");
				vert.x4 = 1;
			}

			ivec3 vertPos = vert.getPosition();
			cpp_bin_float_100 x(vertPos.x);
			cpp_bin_float_100 y(vertPos.y);
			cpp_bin_float_100 z(vertPos.z);
			cpp_bin_float_100 div(BIG_NUM_STR);
			vertices.append(MPoint(
				(double)(x / div),
				(double)(y / div),
				(double)(z / div)));
			vertList.append(j + totalVerts);
		}
		totalVerts += numVerts;

	}

	MFnTransform transformFn;
	MFnMesh meshFn;

	MObject transformObj = transformFn.create();
	transformFn.setName("Polygon0");

	MObject meshObj = meshFn.create(
		totalVerts, // num of verts
		p.size(), // num of polygons
		vertices, // vert pos array
		vertCount, // polygon count array
		vertList, // polygon connects
		transformObj //parent object
	);
	MString shapeName = transformFn.name() + "Shape";

	meshFn.setName(shapeName);
	MGlobal::executeCommand("sets -add initialShadingGroup " + shapeName + ";");
}

void ember::drawBoundingBox(AABB boundingBox)
{
	static int count = 0;

	cpp_bin_float_100 fmaxX(boundingBox.max.x);
	cpp_bin_float_100 fmaxY(boundingBox.max.y);
	cpp_bin_float_100 fmaxZ(boundingBox.max.z);
	cpp_bin_float_100 fminX(boundingBox.min.x);
	cpp_bin_float_100 fminY(boundingBox.min.y);
	cpp_bin_float_100 fminZ(boundingBox.min.z);
	cpp_bin_float_100 div = cpp_bin_float_100(BIG_NUM_STR);
	double maxX = (double)(fmaxX / div);
	double maxY = (double)(fmaxY / div);
	double maxZ = (double)(fmaxZ / div);
	double minX = (double)(fminX / div);
	double minY = (double)(fminY / div);
	double minZ = (double)(fminZ / div);

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

void ember::drawPosition(ivec3 pos)
{
	// Draw Ref point using AABB
	AABB refPointAABB;
	refPointAABB.min = pos - POINT_AABB;
	refPointAABB.max = pos + POINT_AABB;
	drawBoundingBox(refPointAABB);
}

void ember::drawSegment(Segment s)
{
	// Compute the two points for the segment 
	Point p1 = intersect(s.bound1, s.line.p1, s.line.p2);
	Point p2 = intersect(s.bound2, s.line.p1, s.line.p2);
	ivec3 p1p = p1.getPosition();
	ivec3 p2p = p2.getPosition();
	
	cpp_bin_float_100 fx1(p1p.x);
	cpp_bin_float_100 fy1(p1p.y);
	cpp_bin_float_100 fz1(p1p.z);
	cpp_bin_float_100 fx2(p2p.x);
	cpp_bin_float_100 fy2(p2p.y);
	cpp_bin_float_100 fz2(p2p.z);
	cpp_bin_float_100 f(BIG_NUM_STR);
	char buffer[1024];
	sprintf_s(buffer, 1024, "curve -bezier -d 1 -p %f %f %f -p %f %f %f -k 0 -k 1", 
		(double)(fx1/f), (double)(fy1/f), (double)(fz1/f), 
		(double)(fx2/f), (double)(fy2/f), (double)(fz2/f));
	MGlobal::executeCommand(buffer, true);
	//MGlobal::displayInfo("buffer");
}