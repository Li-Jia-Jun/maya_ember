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

	ember::EMBER ember;

	// For finding the bound of two meshes
	float minX01 = 100000000, minY01 = 100000000, minZ01 = 100000000, maxX01 = -100000000, maxY01 = -100000000, maxZ01 = -100000000;
	float minX02 = 100000000, minY02 = 100000000, minZ02 = 100000000, maxX02 = -100000000, maxY02 = -100000000, maxZ02 = -100000000;

	BigFloat bf(BIG_NUM_STR);

	if (selectionList.length() < 2)
	{
		MGlobal::displayInfo("Error: one mesh cannot perform boolean operations");
		return status;
	}

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
			BigInt x = ember::bigFloatToBigInt(normal.x * bf);
			BigInt y = ember::bigFloatToBigInt(normal.y * bf);
			BigInt z = ember::bigFloatToBigInt(normal.z * bf);
			ember::ivec3 emberNormal{ x, y, z };
			normals.push_back(emberNormal);

			std::vector<ember::ivec3 > polyVerts;
		
			for (int k = 0; k < polygonIt.polygonVertexCount(); k++)
			{
				MPoint point = pointArray[k];
				// Keep track of 
				if (i == 0)
				{
					if (point.x < minX01)
					{
						minX01 = point.x;
					}
					if (point.y < minY01)
					{
						minY01 = point.y;
					}
					if (point.z < minZ01)
					{
						minZ01 = point.z;
					}
					if (point.x > maxX01)
					{
						maxX01 = point.x;
					}
					if (point.y > maxY01)
					{
						maxY01 = point.y;
					}
					if (point.z > maxZ01)
					{
						maxZ01 = point.z;
					}
				}
				if (i == 1)
				{
					if (point.x < minX02)
					{
						minX02 = point.x;
					}
					if (point.y < minY02)
					{
						minY02 = point.y;
					}
					if (point.z < minZ02)
					{
						minZ02 = point.z;
					}
					if (point.x > maxX02)
					{
						maxX02 = point.x;
					}
					if (point.y > maxY02)
					{
						maxY02 = point.y;
					}
					if (point.z > maxZ02)
					{
						maxZ02 = point.z;
					}
				}			

				// Scale the vert pos to a big enough number
				BigInt px = ember::bigFloatToBigInt(point.x * bf);
				BigInt py = ember::bigFloatToBigInt(point.y * bf);
				BigInt pz = ember::bigFloatToBigInt(point.z * bf);
				ember::ivec3 vert{px, py, pz};
				polyVerts.push_back(vert);
			}
			vertices.push_back(polyVerts);
		}

		// Load the data into an ember
		ember.ReadMeshData(vertices, normals);
	}

	ember::AABB bound, bound01, bound02;
	bound01.max.x = ember::bigFloatToBigInt(BigFloat(maxX01) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound01.max.y = ember::bigFloatToBigInt(BigFloat(maxY01) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound01.max.z = ember::bigFloatToBigInt(BigFloat(maxZ01) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound01.min.x = ember::bigFloatToBigInt(BigFloat(minX01) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));
	bound01.min.y = ember::bigFloatToBigInt(BigFloat(minY01) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));
	bound01.min.z = ember::bigFloatToBigInt(BigFloat(minZ01) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));

	bound02.max.x = ember::bigFloatToBigInt(BigFloat(maxX02) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound02.max.y = ember::bigFloatToBigInt(BigFloat(maxY02) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound02.max.z = ember::bigFloatToBigInt(BigFloat(maxZ02) * BigFloat(BIG_NUM_STR) + BigFloat(AABB_OFFSET));
	bound02.min.x = ember::bigFloatToBigInt(BigFloat(minX02) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));
	bound02.min.y = ember::bigFloatToBigInt(BigFloat(minY02) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));
	bound02.min.z = ember::bigFloatToBigInt(BigFloat(minZ02) * BigFloat(BIG_NUM_STR) - BigFloat(AABB_OFFSET));

	bound.max.x = bound01.max.x >= bound02.max.x ? bound01.max.x : bound02.max.x;
	bound.max.y = bound01.max.y >= bound02.max.y ? bound01.max.y : bound02.max.y;
	bound.max.z = bound01.max.z >= bound02.max.z ? bound01.max.z : bound02.max.z;
	bound.min.x = bound01.min.x <= bound02.min.x ? bound01.min.x : bound02.min.x;
	bound.min.y = bound01.min.y <= bound02.min.y ? bound01.min.y : bound02.min.y;
	bound.min.z = bound01.min.z <= bound02.min.z ? bound01.min.z : bound02.min.z;

	ember.SetInitBounds(bound, bound01, bound02);

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