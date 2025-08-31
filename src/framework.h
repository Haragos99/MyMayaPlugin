#pragma once

#define WIN32_LEAN_AND_MEAN             // Exclude rarely-used stuff from Windows headers
// Windows Header Files
#include <windows.h>
#include <vector>

#include <maya/MGlobal.h>
#include <maya/MPxCommand.h>
#include <maya/MFnMesh.h>
#include <maya/MPointArray.h>
#include <maya/MIntArray.h>
#include <maya/MMatrix.h>
#include <maya/MStatus.h>
#include <maya/MObject.h>
#include <maya/MSelectionList.h>
#include <maya/MDagPath.h>
#include <maya/MItMeshVertex.h>
#include <maya/MItMeshPolygon.h>
#include <maya/MItMeshEdge.h>
#include <maya/MDagPath.h>
#include <maya/MPoint.h>
#include <maya/MItSelectionList.h>
#include <maya/MCallbackIdArray.h>
#include <maya/MEventMessage.h>
#include <maya/MDagMessage.h>
#include <maya/MNodeMessage.h>
#include <maya/MPlug.h>
#include <Eigen/Core>
