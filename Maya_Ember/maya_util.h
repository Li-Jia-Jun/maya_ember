#pragma once
#include <maya/MGlobal.h>
#include <maya/MString.h>
#include "geometry.h"
void printIvec3(ember::ivec3 v)
{
	char buffer[128];
	sprintf_s(buffer, "ivec3: %i %i %i", v.x, v.y, v.z);
	MString debug(buffer);
	MGlobal::displayInfo(buffer);
}

void printPoint(ember::Point p)
{
	char buffer[128];
	sprintf_s(buffer, "x1: %i x2: %i x3: %i x4: %i", p.x1, p.x2, p.x3, p.x4);
	MString debug(buffer);
	MGlobal::displayInfo(buffer);
}

void printPlane(ember::Plane p)
{
	char buffer[128];
	sprintf_s(buffer, "a: %i b: %i c: %i d: %i", p.a, p.b, p.c, p.d);
	MString debug(buffer);
	MGlobal::displayInfo(buffer);
}

void printPolygon(ember::Polygon p)
{
	char buffer[128];
	sprintf_s(buffer, "support plane: ");
	MString debug(buffer);
	MGlobal::displayInfo(buffer);
	ember::printPlane(p.support);

	sprintf_s(buffer, "bounds: ");
	MGlobal::displayInfo(buffer);
	for (int i = 0; i < p.bounds.size(); i++)
	{
		ember::printPlane(p.bounds[i]);
	}
}
