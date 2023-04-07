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
	// <<<your code goes here>>>

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

		// Note: here we assume the mesh is a pure triangle mesh
		// Iterate through triangulated polygons and add their vertices and normals
		for (; !polygonIt.isDone(); polygonIt.next())
		{
			MPointArray pointArray;
			MIntArray triangleVerts;
			MVector normal;
			polygonIt.getTriangle(0, pointArray, triangleVerts, MSpace::kObject);

			// Get the normal of the current vertex
			polygonIt.getNormal(normal, MSpace::kObject);
			ember::ivec3 emberNormal;
			emberNormal.x = normal.x;
			emberNormal.y = normal.y;
			emberNormal.z = normal.z;
			normals.push_back(emberNormal);

			// Each triangles have 3 vertices
			std::vector<ember::ivec3 > emberVerts;
			for (int k = 0; k < 3; k++)
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

				//char buffer[128];
				//sprintf_s(buffer, "Point: (%f, %f, %f)", point.x, point.y, point.z);
				//MGlobal::displayInfo(buffer);

				//ember::printIvec3(emberVert);
			}
			vertices.push_back(emberVerts);
		}

		// Load the data into an ember
		ember.ReadMeshData(vertices, normals);


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

		//}
		//for (int i = 0; i < normals.size(); i++)
		//{
		//	char buffer[128];
		//	sprintf_s(buffer, " Normal %i: (%i, %i, %i)", i, normals[i].x, normals[i].y, normals[i].z);
		//	MGlobal::displayInfo(buffer);
		//}
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

		//char buffer[128];
		//sprintf_s(buffer, " bound: max: (%i, %i, %i), min: (%i, %i, %i)", bound.max.x, bound.max.y, bound.max.z, bound.min.x, bound.min.y, bound.min.z);
		//MGlobal::displayInfo(buffer);
		
		// The algorithm starts here
		//ember.BuildBSPTree();


		// Try drawing a simple box with some given verts
		
		// It seems that the vert must be counter clock wise
		//MPointArray vertices;
		//vertices.append(MPoint(-0.5, 0.5, 0.5));
		//vertices.append(MPoint(0.5, 0.5, 0.5));
		//vertices.append(MPoint(0.5, -0.5, 0.5));
		//vertices.append(MPoint(-0.5, -0.5, 0.5));
		//vertices.append(MPoint(-0.5, 0.5, -0.5));
		//vertices.append(MPoint(0.5, 0.5, -0.5));
		//vertices.append(MPoint(0.5, -0.5, -0.5));
		//vertices.append(MPoint(-0.5, -0.5, -0.5));

		//// num of verts for each face
		//MIntArray vertCount;
		//for (int i = 0; i < 6; i++)
		//{
		//	vertCount.append(4);
		//}

		//int faceConnectsArray[] = { 3, 2, 1, 0, 2, 6, 5, 1, 6, 7, 4, 5, 7, 3, 0, 4, 0, 1, 5, 4, 7, 6, 2, 3};
		//MIntArray vertList(faceConnectsArray, 24);

		//MFnMesh meshFn;

		//MIntArray faceList;
		//for (int i = 0; i < 6; i++)
		//{
		//	faceList.append(i);
		//}

		//MFnTransform transformFn;
		//MObject transformObj = transformFn.create();
		//transformFn.setName("BooleanaResult");

		//
		//MObject meshObj = meshFn.create(
		//	vertices.length(),	// num of verts
		//	vertCount.length(),	// num of polygons
		//	vertices,	// vert pos array
		//	vertCount,	// polygon count array
		//	vertList,	// polygon connects
		//	transformObj	//parent object
		//);
		//meshFn.setName("ResultShape");
		//MGlobal::executeCommand("sets -add initialShadingGroup ResultShape;");


		//MFnDependencyNode meshNode(meshObj);
		//MPlug overrideEnabledPlug = meshNode.findPlug("overrideEnabled", true, &status);
		//overrideEnabledPlug.setValue(1);
		//MPlug overrideShadingPlug = meshNode.findPlug("overrideShading", true, &status);
		//overrideShadingPlug.setValue(0);


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