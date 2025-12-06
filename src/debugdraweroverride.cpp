#include "debugdraweroverride.h"
#include "deltamushnode.h"

MTypeId MyLocator::id(0x001226C1);

MObject MyLocator::locatorMsgAttr;


MStatus MyLocator::initialize()
{
    MFnTypedAttribute tAttr2;
    locatorMsgAttr = tAttr2.create("deformerMessage", "dmsg", MFnData::kString);
    tAttr2.setStorable(false);
    tAttr2.setWritable(true);
    tAttr2.setReadable(true);

    addAttribute(locatorMsgAttr);

    return MS::kSuccess;
}

MStatus MyLocator::connectionMade(const MPlug& plug, const MPlug& otherPlug, bool asSrc)
{
    auto node = otherPlug.node();
    MFnDependencyNode fn(node);

    const DeltaMushNode* deltamushNode = dynamic_cast<DeltaMushNode*>(fn.userNode());
    if (!deltamushNode) {
        MGlobal::displayInfo("Error: Cannot cast node to DeltaMushNode" );
        return MS::kFailure;
    }
	deltamush = deltamushNode->m_deltamush;
    MGlobal::displayInfo("Connection Was successful: " + fn.name());
    return MS::kSuccess;
}

MUserData* MyLocatorDrawOverride::prepareForDraw(
    const MDagPath& objPath,
    const MDagPath& cameraPath,
    const MHWRender::MFrameContext& frameContext,
    MUserData* oldData)
{
    MObject drawNode = objPath.node();
    MFnDependencyNode depNodeFn(drawNode);
    MGlobal::displayInfo("Preparing for draw: " + depNodeFn.name());
    auto userDataObj = depNodeFn.userNode();
    MyLocator* locatorNode = dynamic_cast<MyLocator*>(userDataObj);
    MeshData* data = dynamic_cast<MeshData*>(oldData);
    if (locatorNode && locatorNode->deltamush)
    {
        // Allocate only if needed. Maya will delete it automatically.
        data = locatorNode->deltamush->createMeshData();
    }

    return data;
}

// TODO: Refactor for other solution to use MUserData
void MyLocatorDrawOverride::addUIDrawables(
    const MDagPath& path,
    MHWRender::MUIDrawManager& drawManager,
    const MHWRender::MFrameContext& frameContext,
    const MUserData* data) 
{
   const MeshData* meshdata = dynamic_cast<const MeshData*>(data);
    if(meshdata)
    { 
		const MMatrix& localToWorld = meshdata->localToWorldMatrix;
        drawManager.beginDrawable();
        MGlobal::displayInfo("SSSSSIt worked");

        for (auto v : meshdata->m_smooth)
        {
            //drawManager.setPointSize(8.0f);
            //drawManager.setColor(MColor(0.0f, 1.0f, 1.0f));
            //drawManager.point(v);
        }
        
        /*
		* Smooth mesh draw
        * 
        for (auto f : meshdata->faceesIDX)
        {
            drawManager.setColor(MColor(1.0f, 1.0f, 0.0f));
            auto faceVerts = f.second;
            MPointArray face;
            for (int i : faceVerts)
            {
                face.append(meshdata->m_smooth[i]);
            }

            drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, face);

        }
        */

        // Draw red point
        drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
        drawManager.point(MPoint(0.0, 0.0, 0.0));

        if (meshdata->factor < 1)
        {
            for (int i = 0; i < meshdata->m_vertices.length(); ++i)
            {
                drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
                MPoint delta = meshdata->deltas[i];
                MPoint start = meshdata->m_matrcesC[i] * delta;
                delta[0] *= meshdata->factor;
                delta[1] *= meshdata->factor;
                delta[2] *= meshdata->factor;
                MPoint end = meshdata->m_matrcesC[i] * delta;
                drawManager.line(start * localToWorld, end * localToWorld);
            }
        }
	    int d = meshdata->collisonData.intersected.size();
        for (auto& b : meshdata->collisonData.intersected)
        {
            drawBoundingBox(drawManager, b, MColor(1.0f, 0.0f, 1.0f), 3.0f);
        }

        for(auto& b : meshdata->collisonData.mesh)
        {
            drawBoundingBox(drawManager, b, MColor(0.0f, 0.0f, 1.0f), 1.0f);
	    }

        for (auto& b : meshdata->collisonData.smoothmesh)
        {
            drawBoundingBox(drawManager, b, MColor(0.0f, 1.0f, 0.0f), 1.0f);
        }


        for (auto v : meshdata->collisonData.collidedVertecesIdx)
        {
		    drawManager.setPointSize(8.0f);
            drawManager.setColor(MColor(0.0f, 0.0f, 1.0f));
		    MPoint p = meshdata->m_vertices[v] * localToWorld;
		    drawManager.point(p);
        }

        for (auto v : meshdata->collisonData.collidedAllVertecesIdx)
        {
            drawManager.setPointSize(8.0f);
            drawManager.setColor(MColor(1.0f, 0.0f, 1.0f));
            MPoint p = meshdata->m_vertices[v] * localToWorld;
            drawManager.point(p);
        }

        for (auto f : meshdata->collisonData.filteredFacesIdx)
        {
            drawManager.setColor(MColor(1.0f, 0.75f, 0.0f));
		    auto faceVerts = meshdata->faceesIDX.at(f);
		    MPointArray face;
            for (int i : faceVerts)
            {
			    face.append(meshdata->m_vertices[i] * localToWorld);
            }

		    drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, face);

        }

        for (auto f : meshdata->collisonData.collidedFacesIdx)
        {
            drawManager.setColor(MColor(1.0f, 0.0f, 0.0f));
            auto faceVerts = meshdata->faceesIDX.at(f);
            MPointArray face;
            for (int i : faceVerts)
            {
                face.append(meshdata->m_vertices[i] * localToWorld);
            }

            drawManager.mesh(MHWRender::MUIDrawManager::kTriangles, face);

        }

        for (auto e : meshdata->collisonData.collidedEdgesIdx)
        {
            drawManager.setLineWidth(2.0f);
            drawManager.setColor(MColor(0.0f, 1.0f, 0.0f));
		    auto edgeVerts = meshdata->edgesIDX.at(e);
            MPoint p1 = meshdata->m_vertices[edgeVerts.first] * localToWorld;
            MPoint p2 = meshdata->m_vertices[edgeVerts.second] * localToWorld;
		    drawManager.line(p1, p2);
        }

        for (auto& p : meshdata->collisonData.collidedPoints)
        {
            drawManager.setPointSize(10.0f);
            drawManager.setColor(MColor(0.0f, 0.0f, 1.0f));
            drawManager.point(p * localToWorld);
        }
    }
    // Draw green line
    drawManager.setColor(MColor(0.0f, 1.0f, 0.0f));
    drawManager.line(MPoint(0.0, 0.0, 0.0), MPoint(1.0, 1.0, 0.0));

    drawManager.endDrawable();
}




// Draws a bounding box (outline) for the given box in world space
void MyLocatorDrawOverride::drawBoundingBox(MHWRender::MUIDrawManager& drawManager,
    const MBoundingBox& box,
    const MColor& color,
    float lineWidth)
{
    MPoint minP = box.min();
    MPoint maxP = box.max();

    MPoint corners[8] = {
        MPoint(minP.x, minP.y, minP.z),
        MPoint(maxP.x, minP.y, minP.z),
        MPoint(maxP.x, maxP.y, minP.z),
        MPoint(minP.x, maxP.y, minP.z),
        MPoint(minP.x, minP.y, maxP.z),
        MPoint(maxP.x, minP.y, maxP.z),
        MPoint(maxP.x, maxP.y, maxP.z),
        MPoint(minP.x, maxP.y, maxP.z)
    };

    drawManager.setColor(color);
    drawManager.setLineWidth(lineWidth);

    // bottom face
    drawManager.line(corners[0], corners[1]);
    drawManager.line(corners[1], corners[2]);
    drawManager.line(corners[2], corners[3]);
    drawManager.line(corners[3], corners[0]);

    // top face
    drawManager.line(corners[4], corners[5]);
    drawManager.line(corners[5], corners[6]);
    drawManager.line(corners[6], corners[7]);
    drawManager.line(corners[7], corners[4]);

    // vertical edges
    drawManager.line(corners[0], corners[4]);
    drawManager.line(corners[1], corners[5]);
    drawManager.line(corners[2], corners[6]);
    drawManager.line(corners[3], corners[7]);

}
