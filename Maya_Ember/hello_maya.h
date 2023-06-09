#ifndef HELLOMAYA_H
#define HELLOMAYA_H
#include <maya/MArgList.h>
#include <maya/MObject.h>
#include <maya/MGlobal.h>
#include <maya/MFnPlugin.h>
#include <maya/MPointArray.h>
#include <maya/MFnMesh.h>
#include <maya/MPoint.h>
#include <maya/MDagPath.h>
#include <maya/MString.h>
#include <maya/MPxCommand.h>
#include <maya/MSelectionList.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MIntArray.h>
#include <maya/MVectorArray.h>
#include <maya/MFnMeshData.h>
#include <maya/MFnDependencyNode.h>
#include <maya/MFnTransform.h>
#include <maya/MPlug.h>
#include <maya/MFnAttribute.h>
#include <maya/M3dView.h>
#include <maya/MPxContext.h>
#include <maya/MArgList.h>
#include <maya/MPxCommand.h>
#include <maya/MSyntax.h>
#include <maya/MArgParser.h>
#include "ember.h"
#include <vector>



// custom Maya command
class helloMaya : public MPxCommand
{
public:
	helloMaya() {};
	virtual MStatus doIt(const MArgList& args);
	static void* creator();

};
#endif