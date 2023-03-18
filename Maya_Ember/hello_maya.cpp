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