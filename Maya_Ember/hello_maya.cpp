#include "hello_maya.h"
#include <maya/MFnPlugin.h>

// define EXPORT for exporting dll functions
#define EXPORT _declspec(dllexport)
#define BIG_NUM 10000

// Maya Plugin creator function
void* helloMaya::creator()
{
	return new helloMaya;
}
// Plugin doIt function
MStatus helloMaya::doIt(const MArgList& argList)
{
	MStatus status;
	MGlobal::displayInfo("Reticuleana");

	MSelectionList selectionList;
	MGlobal::getActiveSelectionList(selectionList);

	ember::EMBER ember = ember::EMBER();
	// For finding the bound
	float minX = 100000000, minY = 100000000, minZ = 100000000, maxX = -100000000, maxY = -100000000, maxZ = -100000000;

	// Iterating through all the selected mesh objects
	for (int i = 0; i < selectionList.length(); i++)
	{
		MDagPath dagPath;
		MObject meshObject;

		selectionList.getDagPath(i, dagPath, meshObject); // Get the first selected object
		MFnMesh mesh(dagPath); // Create an MFnMesh object from the selected object
		MItMeshPolygon polygonIt(dagPath);

		std::vector<std::vector<ember::ivec3>> vertices;
		std::vector<ember::ivec3> normals;

		MString meshName(dagPath.partialPathName());
		MGlobal::displayInfo(meshName);

		// Iterate through polygons and add their vertices and normals
		for (; !polygonIt.isDone(); polygonIt.next())
		{
			MPointArray pointArray;
			MVector normal;
	
			// Get the normal and verts of the current face
			polygonIt.getPoints(pointArray, MSpace::kObject, &status);
			polygonIt.getNormal(normal, MSpace::kObject);
			ember::ivec3 emberNormal;
			emberNormal.x = normal.x;
			emberNormal.y = normal.y;
			emberNormal.z = normal.z;
			normals.push_back(emberNormal);
			//printIvec3(emberNormal);

			// Each triangles have 3 vertices
			std::vector<ember::ivec3 > emberVerts;
		
			for (int k = 0; k < polygonIt.polygonVertexCount(); k++)
			{
				MPoint point = pointArray[k];
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

				// Scale the vert pos to a big enough number
				ember::ivec3 emberVert;
				emberVert.x = point.x * BIG_NUM;
				emberVert.y = point.y * BIG_NUM;
				emberVert.z = point.z * BIG_NUM;
				emberVerts.push_back(emberVert);
				//ember::printIvec3(emberVert);
			}
			vertices.push_back(emberVerts);
		}

		// Load the data into an ember
		ember.ReadMeshData(vertices, normals);
	}

		ember::AABB bound;
		int offset = 100;
		bound.max.x = maxX * BIG_NUM + offset;
		bound.max.y = maxY * BIG_NUM + offset;
		bound.max.z = maxZ * BIG_NUM + offset;
		bound.min.x = minX * BIG_NUM - offset;
		bound.min.y = minY * BIG_NUM - offset;
		bound.min.z = minZ * BIG_NUM - offset;
		ember.SetInitBound(bound);
		// LEO::TODO::currently this only works for the original mesh. If it has transformation it will break
		drawBoundingBox(bound);
		
		// The algorithm starts here
		ember.BuildBSPTree();

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