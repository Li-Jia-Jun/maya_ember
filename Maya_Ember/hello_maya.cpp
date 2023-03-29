#include "hello_maya.h"
#include <maya/MFnPlugin.h>

// define EXPORT for exporting dll functions
#define EXPORT _declspec(dllexport)

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

	for (int i = 0; i < selectionList.length(); i++)
	{
		MDagPath dagPath;
		MObject meshObject;

		selectionList.getDagPath(i, dagPath, meshObject); // Get the first selected object
		MFnMesh mesh(dagPath); // Create an MFnMesh object from the selected object

		float minX = 100000000, minY = 100000000, minZ = 100000000, maxX = -100000000, maxY = -100000000, maxZ = -100000000;
		MString meshName(dagPath.partialPathName());
		MGlobal::displayInfo(meshName);

		int numVertices = mesh.numVertices();
		for (int j = 0; j < numVertices; j++) {
			MPoint point;
			mesh.getPoint(j, point); // Get the position of the ith vertex
			char buffer[128];
			sprintf_s(buffer, " Vertex %d: (%f, %f, %f)", j, point.x, point.y, point.z);
			MGlobal::displayInfo(buffer);

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

		}
		char buffer[128];
		sprintf_s(buffer, " AABB (%f, %f, %f), (%f, %f, %f)", minX, minY, minZ, maxX, maxY, maxZ);
		MGlobal::displayInfo(buffer);

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