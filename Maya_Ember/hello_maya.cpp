#include "hello_maya.h"
#include <maya/MFnPlugin.h>

// define EXPORT for exporting dll functions
#define EXPORT _declspec(dllexport)
#define BIG_NUM 100000

// Maya Plugin creator function
void* helloMaya::creator()
{
	return new helloMaya;
}
// Plugin doIt function
MStatus helloMaya::doIt(const MArgList& argList)
{
	MStatus status;
	MGlobal::displayInfo("Hello World!");
	// <<<your code goes here>>>

	MString nameStr;
	MString idStr;
	for (int i = 0; i < argList.length(); i++)
	{
		if (MString("-name") == argList.asString(i, &status) && 
			MS::kSuccess == status)
		{
			nameStr = argList.asString(++i, &status);
		}
		else if (MString("-id") == argList.asString(i, &status) &&
			MS::kSuccess == status)
		{
			idStr = argList.asString(++i, &status);
		}
		else
		{
			// Do nothing for unidentified parameters
		}
	}

	MString formatStr = "confirmDialog -title \"Hello Maya\" -message \"Name: ^1s \\nId: ^2s\" - button \"OK\" - defaultButton \"OK\"; ";
	MString dialogStr;
	dialogStr.format(formatStr, nameStr, idStr);
	status = MGlobal::executeCommand(dialogStr);

	if (!status)
	{
		status.perror("confirmDialog command failed");
	}	


	MSelectionList selectionList;
	MGlobal::getActiveSelectionList(selectionList);
	
	// Iterating through all the selected mesh objects
	for (int i = 0; i < selectionList.length(); i++)
	{
		MDagPath dagPath;
		MObject meshObject;

		selectionList.getDagPath(i, dagPath, meshObject); // Get the first selected object
		MFnMesh mesh(dagPath); // Create an MFnMesh object from the selected object
		MItMeshPolygon polygonIt(dagPath);

		ember::EMBER ember = ember::EMBER();
		std::vector<std::vector<ember::ivec3>> vertices;
		std::vector<ember::ivec3> normals;

		float minX = 100000000, minY = 100000000, minZ = 100000000, maxX = -100000000, maxY = -100000000, maxZ = -100000000;
		MString meshName(dagPath.partialPathName());
		MGlobal::displayInfo(meshName);

		// Note: here we assume the mesh is a pure triangle mesh
		for (; !polygonIt.isDone(); polygonIt.next())
		{
			MPointArray pointArray;
			MIntArray triangleVerts;
			MVector normal;
			//polygonIt.getTriangles(pointArray, triangleVerts, MSpace::kObject);
			polygonIt.getTriangle(0, pointArray, triangleVerts, MSpace::kObject);
			// Print out the indices of the triangle's vertices
			//char buffer[128];
			// sprintf_s(buffer, " Triangle:  (%i, %i, %i)", triangleVerts[0], triangleVerts[1], triangleVerts[2]);
			// MGlobal::displayInfo(buffer);

			// Get the normal of the current triangle
			polygonIt.getNormal(normal, MSpace::kObject);
			//sprintf_s(buffer, " Normal: (%f, %f, %f)", normal.x, normal.y, normal.z);
			//MGlobal::displayInfo(buffer);
			ember::ivec3 emberNormal;
			emberNormal.x = normal.x;
			emberNormal.y = normal.y;			
			emberNormal.z = normal.z;
			normals.push_back(emberNormal);
			
			std::vector<ember::ivec3 > emberVerts;
			for (int k = 0; k < 3; k++)
			{
				MPoint point = pointArray[k];
				//mesh.getPoint(k, point); // Get the position of the ith vertex

				if (point.x < minX)
				{
					minX = point.x;
				}
				if (point.y < minY)
				{
					minY = point.y;
				}
				if (point.z < minZ)
				{
					minZ = point.z;
				}
				if (point.x > maxX)
				{
					maxX = point.x;
				}
				if (point.y > maxY)
				{
					maxY = point.y;
				}
				if (point.z > maxZ)
				{
					maxZ = point.z;
				}
				// char buffer[128];
				// sprintf_s(buffer, " Point: (%f, %f, %f)", point.x, point.y, point.z);
				// MGlobal::displayInfo(buffer);
				ember::ivec3 emberVert;
				emberVert.x = point.x * BIG_NUM;
				emberVert.y = point.y * BIG_NUM;
				emberVert.z = point.z * BIG_NUM;
				emberVerts.push_back(emberVert);
			}
			vertices.push_back(emberVerts);
		}

		// Load the data into an ember
		ember.ReadMeshData(vertices, normals);
		ember::AABB bound;
		bound.max.x = maxX * BIG_NUM;
		bound.max.y = maxY * BIG_NUM;
		bound.max.z = maxZ * BIG_NUM;
		bound.min.x = minX * BIG_NUM;
		bound.min.y = minY * BIG_NUM;
		bound.min.z = minZ * BIG_NUM;
		ember.SetInitBound(bound);

		/* PRINT DEBUGGING INFO */
		//for (int i = 0; i < vertices.size(); i++)
		//{
		//	char buffer[128];
		//	sprintf_s(buffer, " Vertex %i - 0: (%i, %i, %i)", i, vertices[i][0].x, vertices[i][0].y, vertices[i][0].z);
		//	MGlobal::displayInfo(buffer);
		//	sprintf_s(buffer, " Vertex %i - 1: (%i, %i, %i)", i, vertices[i][1].x, vertices[i][1].y, vertices[i][1].z);
		//	MGlobal::displayInfo(buffer);
		//	sprintf_s(buffer, " Vertex %i - 2: (%i, %i, %i)", i, vertices[i][2].x, vertices[i][2].y, vertices[i][2].z);
		//	MGlobal::displayInfo(buffer);
		//for (int i = 0; i < normals.size(); i++)
		//{
		//	char buffer[128];
		//	sprintf_s(buffer, " Normal %i: (%i, %i, %i)", i, normals[i].x, normals[i].y, normals[i].z);
		//	MGlobal::displayInfo(buffer);
		//}
		//char buffer[128];
		//sprintf_s(buffer, " bound: max: (%i, %i, %i), min: (%i, %i, %i)", bound.max.x, bound.max.y, bound.max.z, bound.min.x, bound.min.y, bound.min.z);
		//MGlobal::displayInfo(buffer);
	}
	return status;
}

// Initialize Maya Plugin upon loading
EXPORT MStatus initializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj, "CIS660", "1.0", "Any");
	status = plugin.registerCommand("helloMaya", helloMaya::creator);
	if (!status)
		status.perror("registerCommand failed");
	return status;
}
// Cleanup Plugin upon unloading
EXPORT MStatus uninitializePlugin(MObject obj)
{
	MStatus status;
	MFnPlugin plugin(obj);
	status = plugin.deregisterCommand("helloMaya");
	if (!status)
		status.perror("deregisterCommand failed");
	return status;
}