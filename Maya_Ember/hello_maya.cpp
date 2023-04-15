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

	MSyntax syntax;
	status = syntax.addFlag("-u", "-union", MSyntax::kString);
	status = syntax.addFlag("-i", "-intersection", MSyntax::kString);
	status = syntax.addFlag("-s", "subtraction", MSyntax::kString);


	MArgParser argParser(syntax, argList);
	unsigned int num_of_flag_used = argParser.numberOfFlagsUsed();
	if (num_of_flag_used > 1)
	{
		MGlobal::displayInfo("Error: multiple Boolean operations are selected");
		return status;
	}


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
			polygonIt.getPoints(pointArray, MSpace::kWorld, &status);
			polygonIt.getNormal(normal, MSpace::kWorld);
			ember::ivec3 emberNormal{ normal.x * BIG_NUM, normal.y * BIG_NUM, normal.z * BIG_NUM };
			normals.push_back(emberNormal);

			std::vector<ember::ivec3 > polyVerts;
		
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
				ember::ivec3 vert;
				vert.x = point.x * BIG_NUM;
				vert.y = point.y * BIG_NUM;
				vert.z = point.z * BIG_NUM;
				polyVerts.push_back(vert);
			}
			vertices.push_back(polyVerts);
		}

		// Load the data into an ember
		ember.ReadMeshData(vertices, normals);
	}

		ember::AABB bound;
		int offset = 1000;
		bound.max.x = maxX * BIG_NUM + offset;
		bound.max.y = maxY * BIG_NUM + offset;
		bound.max.z = maxZ * BIG_NUM + offset;
		bound.min.x = minX * BIG_NUM - offset;
		bound.min.y = minY * BIG_NUM - offset;
		bound.min.z = minZ * BIG_NUM - offset;
		ember.SetInitBound(bound);

		// The algorithm starts here
		ember.BuildBSPTree();

		if (argParser.isFlagSet("-u"))
		{
			MGlobal::displayInfo("Reticuleana: Union");
		}
		else if (argParser.isFlagSet("-i"))
		{
			MGlobal::displayInfo("Reticuleana: Intersection");
		}
		else if (argParser.isFlagSet("-s"))
		{
			MGlobal::displayInfo("Reticuleana: Subtraction");
		}
		else
		{
			MGlobal::displayInfo("Error: no Boolean operation is selected");
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

	// Auto register Mel menu script
	char buffer[2048];
	sprintf_s(buffer, 2048, "source \"%s/EmberUI.mel\";", plugin.loadPath().asChar());
	MGlobal::executeCommand(buffer, true);

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