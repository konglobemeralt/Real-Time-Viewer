

#include "maya_includes.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <vector>
#include "sharedMem.h"

using namespace DirectX;
using namespace std;

void createdNodeCallback(MObject &node, void* clientData);
void createdTransformCallback(MObject &node, void* clientData);
void renamedNodeCallback(MObject &node, const MString &str, void* clientData);

//void createdLightCallback(MObject &node, void* clientData);
//void lightAttrChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);

void meshAttributeChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);
void transformChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);

void destroyedNodeCallback(MObject& object, MDGModifier& modifier, void* clientData);

void timerCB(float elapsedTime, float lastTime, void* clientData);


//materials
void matChanged(MFnMesh& mesh);
void shaderChangedCallback(MObject &node, void* clientData);
void shaderAttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);

MStringArray nodeNames;
MStringArray materialNames;

enum typeOfMessage
{
	meshMessage,
	camMessage,
	transformMessage,
	lightMessage,
};


void goThroughScene();


void getCameraInfo(MFnCamera&);
void getMeshInfo(MFnMesh&);
void getVertexChangeInfo(MFnMesh&);
void getExtrudeChangeInfo(MPlug&);
//void getLightInfo(MFnLight&);

//void getMaterialInfo(MFnMesh&);

void cameraChange(MFnTransform& transform, MFnCamera& camera);
//void lightChange(MFnTransform& transform, MPlug &plug);


float period = 0.1;
float timeNext = 0;

MObject node;

MCallbackIdArray CbIds;

struct VertexData
{
	XMFLOAT4 pos;
	XMFLOAT2 uv;
	XMFLOAT3 norms;
};
VertexData *vertData;

struct CameraData
{
	XMFLOAT4X4 cameraViewMatrix;

};
CameraData cameraData;

struct LightData
{
	XMFLOAT4 position;
	XMFLOAT4 color;
};

struct MaterialData
{
	XMFLOAT4 color;
	XMFLOAT4 specular;
	float reflectivity;
	float specRolloff;
};

struct message
{
	//Header
	int messageType;
	int messageSize;
	int padding;
	//actual message
	//meshes


	//lights
	LightData lightData;

	//camera
	CameraData camData;

	//mesh transforms
	XMFLOAT4X4 matrixData;

	MaterialData matData;
	char textureAddress[500];

	//mesh
	int numMeshes;
	int numVerts;
	std::vector<VertexData> vert;
};

message tMessage;



struct ObjectHeader
{
	int vertexAmount;
};



//File mapping

struct Headtail
{
	unsigned int head;
	unsigned int tail;
	unsigned int reader;
	unsigned int freeMem;
	unsigned int memSize;
};



SharedMemory sm;
unsigned int localHead;
unsigned int slotSize;
MStringArray meshNames;
MStringArray meshMatNames;

// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;


	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res)) {
		CHECK_MSTATUS(res);
	}


	
	localHead = 0;
	slotSize = 256;
	sm.cbSize = 20;
	sm.msgHeaderSize = 8;

	MString memoryString;
	
	memoryString = sm.OpenMemory(100.0f);
	if (memoryString != "Shared memory open success!")
	{
		MGlobal::displayInfo(memoryString);
		sm.CloseMemory();
		return MStatus::kFailure;
	}
	else
	{
		MGlobal::displayInfo(memoryString);
	}


	goThroughScene();

	

	CbIds.append(MDGMessage::addNodeAddedCallback(createdNodeCallback, "mesh", &res));
	CbIds.append(MDGMessage::addNodeAddedCallback(createdTransformCallback, "transform", &res));
	//CbIds.append(MDGMessage::addNodeAddedCallback(createdLightCallback, "light", &res));

	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChangedCallback, &res));

	//trying to figure out how to react to changes also
	CbIds.append(MDGMessage::addNodeAddedCallback(shaderChangedCallback, "lambert", &res));
	CbIds.append(MDGMessage::addNodeAddedCallback(shaderChangedCallback, "blinn", &res));
	CbIds.append(MDGMessage::addNodeAddedCallback(shaderChangedCallback, "phong", &res));


	MGlobal::displayInfo("Maya plugin loaded!!");
	//MGlobal::displayInfo("HEJ!!");



	// if res == kSuccess then the plugin has been loaded,
	// otherwise is has not.
	return res;
}


EXPORT MStatus uninitializePlugin(MObject obj)
{
	// simply initialize the Function set with the MObject that represents
	// our plugin
	MFnPlugin plugin(obj);

	// if any resources have been allocated, release and free here before
	// returning...

	if (CbIds.length() != 1)
	{
		MMessage::removeCallbacks(CbIds);
	}


	sm.CloseMemory();

	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}

void createdNodeCallback(MObject &node, void* clientData)
{
	//append new callbacks as objects are created
	CbIds.append(MNodeMessage::addNodeAboutToDeleteCallback(node, destroyedNodeCallback));
	
	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChangedCallback));
	
	CbIds.append(MNodeMessage::addNameChangedCallback(node, renamedNodeCallback));

}

void meshAttributeChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)

{

	std::string plugName(plug.name().asChar());



	if (strstr(plug.name().asChar(), "doubleSided") != NULL && MNodeMessage::AttributeMessage::kAttributeSet)
	{

		MStatus res;
		MFnMesh meshNode(plug.node(), &res);


		bool exist = false;

		for (size_t i = 0; i < nodeNames.length(); i++)
		{
			if (nodeNames[i] == meshNode.name())
			{
				exist = true;
			}

		}

		if (!exist)
		{
			nodeNames.append(meshNode.name());
		}



		

		//Dummy check for success!
		//MGlobal::displayInfo("Mesh: " + meshNode.fullPathName() + " created!");
		getMeshInfo(meshNode);
	}



	// Vertex has changed
	else if (strstr(plug.partialName().asChar(), "pt["))
	{

		MStatus res;
		MFnMesh meshNode(plug.node(), &res);
		getVertexChangeInfo(meshNode);
	}

	// uv has changed
	else if (strstr(plug.partialName().asChar(), "pv"))
	{

		MStatus res;
		MFnMesh meshNode(plug.node(), &res);
		getVertexChangeInfo(meshNode);
	}

	else if (strstr(plug2.name().asChar(), "polyExtrude") && strstr(plug2.name().asChar(), "manipMatrix"))
	{
		MStatus res;
		MGlobal::displayInfo(MString("Extruude"));
		//MFnMesh meshNode(plug.node(), &res);
		getExtrudeChangeInfo(plug);

	}


	else if (strstr(plug.partialName().asChar(), "iog"))
	{
		MFnMesh mesh(plug.node());
		matChanged(mesh);
	}

	

}
void timerCB(float elapsedTime, float lastTime, void* clientData)
{
	period += elapsedTime;

	if ((int)period % 5 == 0)
	{
		timeNext += elapsedTime;
		if (timeNext >= 1.0f)
		{
			int minute, seconds;
			MString time = MString() + (int)period;
			minute = (int)period / 60;
			seconds = (int)period % 60;

			MString strMin = MString() + minute;
			MString strSec = MString() + seconds;

			//printing in minutes and seconds passed
			MGlobal::displayInfo("Time passed: " + strMin + ":" + strSec);

		}
	}
}

void renamedNodeCallback(MObject &node, const MString &str, void* clientData)
{


	if (strcmp(str.asChar(), "#"))
	{

		if (node.hasFn(MFn::kMesh) == true)
		{
			MFnDagNode mesh(node);
			MString name = mesh.name();
			name = mesh.fullPathName();
			MGlobal::displayInfo("nodefn Changed name from " + str + " to " + name);

		}
	}

}



void createdTransformCallback(MObject &node, void* clientData)
{
	MStatus res;

	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, transformChangedCallback, &res));
	CbIds.append(MNodeMessage::addNodeAboutToDeleteCallback(node, destroyedNodeCallback));


}


void transformChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)
{
	//=?? Camera
	M3dView view = M3dView::active3dView();
	MDagPath camPath;
	view.getCamera(camPath);
	MFnCamera camera(camPath);

	MString name = plug.name();

	if (msg & MNodeMessage::kAttributeSet)
	{



		MFnTransform transform(plug.node());



		if (transform.isParentOf(camera.object()))
		{
			cameraChange(transform, camera);
		}

		//else if (strstr(name.asChar(), "Light"))
		//{
		//	lightChange(transform, plug.parent());
		//}


		else
		{




			MGlobal::displayInfo(MString("Mesh Transformed!!! "));

			MFnMesh meshNode(plug.node());

			//mesh rotation
			double rotation[4];
			transform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

			double scale[3];
			transform.getScale(scale);

			MVector translation = transform.getTranslation(MSpace::kPostTransform);
			//Build matrix with xmvectors
			MGlobal::displayInfo(MString() + "Color: " + translation.x + " " + translation.y + " " + translation.z + " ");


			XMVECTOR translationVector = XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
			XMVECTOR rotationVector = XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]);
			XMVECTOR scaleVector = XMVectorSet(scale[0], scale[1], scale[2], 0.0f);
			XMVECTOR zeroVector = XMVectorSet(0, 0, 0, 0.0f);

			//Put in xmFloat4x4
			DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));



			tMessage.messageType = 2;

			


			int meshCount = nodeNames.length();
			int meshID = -1;

			MFnMesh nameNode(transform.child(0));
			for (size_t i = 0; i < meshCount; i++)
			{
				if (nodeNames[i] == nameNode.name())
				{
					meshID = i;

				}


			}

			MGlobal::displayInfo(MString("ID = " + meshID));


			//Give the mesh an ID
			tMessage.numMeshes = meshID;
			
			int lastFreeMem = 0;
			
			do
			{

				if (sm.cb->freeMem > 250)
				{
					// Sets head to 0 if there are no place to write
					if (sm.cb->head > sm.memSize - 250)
					{
						// The place over will set a number to indicate that the head was moved to 0
						unsigned int type = -1;
						memcpy((char*)sm.buffer + sm.cb->head, &type, sizeof(int));
						lastFreeMem = sm.memSize - sm.cb->head;
						sm.cb->head = 0;

					}
					localHead = sm.cb->head;

					//Send ID
					std::memcpy((char*)sm.buffer + localHead + sizeof(XMFLOAT4X4) + sizeof(int), &tMessage.numMeshes, sizeof(int));
					//end Message Type
					std::memcpy((char*)sm.buffer + localHead, &tMessage.messageType, sizeof(int));
					//send Matrix
					std::memcpy((char*)sm.buffer + localHead + sizeof(int), &tMessage.matrixData, sizeof(XMFLOAT4X4));

					//////////////
					sm.cb->freeMem -= (250 + lastFreeMem);
					sm.cb->head += 250;
					break;


				}
			} while ((sm.cb->freeMem >  250));

			}

		}



	}


//}


void goThroughScene()
{
	MItDag itMeshes(MItDag::kDepthFirst, MFn::kMesh);
	while (!itMeshes.isDone())
	{
		MFnMesh mesh(itMeshes.item());
		getMeshInfo(mesh);
		CbIds.append(MNodeMessage::addAttributeChangedCallback(mesh.object(), meshAttributeChangedCallback));
		CbIds.append(MNodeMessage::addNodeAboutToDeleteCallback(mesh.object(), destroyedNodeCallback));
		itMeshes.next();
	}


	MItDag lightIt(MItDag::kDepthFirst, MFn::kLight);
	while (!lightIt.isDone())
	{
		MFnLight light(lightIt.item());
		//getLightInfo(light);
//		CbIds.append(MNodeMessage::addAttributeChangedCallback(light.object(), lightAttrChangedCallback));

		lightIt.next();
	}

	//Tranforms for all objects cameras
	MItDag itTransform(MItDag::kDepthFirst, MFn::kTransform);
	while (!itTransform.isDone())
	{
		MFnTransform transform(itTransform.item());
		CbIds.append(MNodeMessage::addAttributeChangedCallback(transform.object(), transformChangedCallback));

		itTransform.next();
	}


	MItDependencyNodes itLambert(MFn::kLambert);
	while (!itLambert.isDone())
	{
		MFnLambertShader lambertShader(itLambert.item());


		CbIds.append(MNodeMessage::addAttributeChangedCallback(lambertShader.object(), shaderAttrChanged));
		materialNames.append(lambertShader.name());
		itLambert.next();
	}

	MItDependencyNodes itBlinn(MFn::kBlinn);
	while (!itBlinn.isDone())
	{
		MFnBlinnShader blinnShader(itBlinn.item());

		CbIds.append(MNodeMessage::addAttributeChangedCallback(blinnShader.object(), shaderAttrChanged));
		itBlinn.next();
	}

	MItDependencyNodes itPhong(MFn::kPhong);
	while (!itPhong.isDone())
	{
		MFnPhongShader phongShader(itPhong.item());
		CbIds.append(MNodeMessage::addAttributeChangedCallback(phongShader.object(), shaderAttrChanged));
		itPhong.next();
	}






}


void getMeshInfo(MFnMesh &meshNode)
{
	MStatus res;



	MItMeshPolygon polyIt(meshNode.object());


	//Data
	MPointArray vert;//POINT
	MIntArray vertex; //VERTEX LIST
	float2 uv;
	MVector normal;



	///dataSave

	vector<XMFLOAT4> verticies;
	vector<XMFLOAT2> UV;
	vector<XMFLOAT3> normals;



	while (!polyIt.isDone())
	{
		polyIt.getTriangles(vert, vertex);

		for (size_t i = 0; i < vert.length(); i++)
		{
			polyIt.getUVAtPoint(vert[i], uv);
			polyIt.getNormal(normal);

			verticies.push_back(XMFLOAT4(vert[i].x, vert[i].y, vert[i].z, 1.0f));
			UV.push_back(XMFLOAT2(uv[0], 1 - uv[1]));
			normals.push_back(XMFLOAT3(normal.x, normal.y, normal.z));
		}
		polyIt.next();
	}
	


		message tMessage;

		tMessage.vert.resize(verticies.size());

		for (int i = 0; i < verticies.size(); i++)
		{
			tMessage.vert[i].norms = XMFLOAT3(normals.at(i).x, normals.at(i).y, normals.at(i).z);
			tMessage.vert[i].pos = XMFLOAT4(verticies.at(i).x, verticies.at(i).y, verticies.at(i).z, verticies.at(i).w);
			tMessage.vert[i].uv = XMFLOAT2(UV.at(i).x, UV.at(i).y);
		}

		tMessage.messageType = 0;





		unsigned int meshCount = nodeNames.length();
		int meshID;

		//MFnMesh nameNode(meshNode.child(0));

		for (size_t i = 0; i < meshCount; i++)
		{
			if (nodeNames[i] == meshNode.name())
			{
				meshID = i;
			}

		}


		//Give the mesh an ID
		tMessage.numMeshes = meshID;

		tMessage.numVerts = verticies.size();

		tMessage.messageSize = 10000;
		tMessage.padding = 0;
		
		int lastFreeMem = 0;

		do
		{
			if (sm.cb->freeMem > 250)
			{
				// Sets head to 0 if there are no place to write
				if (sm.cb->head > sm.memSize - 250)
				{
					// The place over will set a number to indicate that the head was moved to 0
					unsigned int type = -1;
					memcpy((char*)sm.buffer  + sm.cb->head, &type, sizeof(int));
					
					lastFreeMem = sm.memSize - sm.cb->head;
					sm.cb->head = 0;

				}
				localHead = sm.cb->head;


				std::memcpy((char*)sm.buffer + localHead, &tMessage.messageType, sizeof(int));
				std::memcpy((char*)sm.buffer + localHead + sizeof(int), &tMessage.messageSize, sizeof(int));



				std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int), &tMessage.numMeshes, sizeof(int));
				std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int), &tMessage.numVerts, sizeof(int));

				for (int i = 0; i < verticies.size(); i++)
				{
					std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i), &tMessage.vert[i].pos, sizeof(XMFLOAT4));
					std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4), &tMessage.vert[i].uv, sizeof(XMFLOAT2));
					std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4) + sizeof(XMFLOAT2), &tMessage.vert[i].norms, sizeof(XMFLOAT3));

				}


				//Mesh transformation
				MFnTransform meshTransform(meshNode.parent(0));
				MVector translation = meshTransform.getTranslation(MSpace::kObject);

				//mesh rotation
				double rotation[4];
				meshTransform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

				//mesh scale
				double scale[4];
				meshTransform.getScale(scale);
				//Build matrix with xmvectors

				XMVECTOR translationVector = XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
				XMVECTOR rotationVector = XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]);
				XMVECTOR scaleVector = XMVectorSet(scale[0], scale[1], scale[2], scale[3]);
				XMVECTOR zeroVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);


				DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));



				std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int), &tMessage.matrixData, sizeof(XMFLOAT4X4));

				////memcpy((char*)pBuf + usedSpace + sizeof(CameraData) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(VertexData)+sizeof(MatrixData), &tMessage.camData, sizeof(CameraData));

				//GET MATERIAL
				int instanceNumber = 0;
				MObjectArray shaders;
				MIntArray indices;

				MaterialData matD;
				//BlinnData blinnD;
				//PhongData phongD;

				meshNode.getConnectedShaders(instanceNumber, shaders, indices);

				for (int i = 0; i < shaders.length(); i++)
				{
					MPlugArray connections;
					MFnDependencyNode shaderGroup(shaders[i]);
					MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
					shaderPlug.connectedTo(connections, true, false);
					for (int u = 0; u < connections.length(); u++)
					{
						if (connections[u].node().hasFn(MFn::kLambert))
						{
							MPlugArray plugs;
							MFnLambertShader lambertShader(connections[u].node());
							lambertShader.findPlug("color").connectedTo(plugs, true, false);

							matD.color.x = lambertShader.color().r;
							matD.color.y = lambertShader.color().g;
							matD.color.z = lambertShader.color().b;
							matD.color.w = lambertShader.color().a;

							matD.specular.x = -1;
							matD.specular.y = -1;
							matD.specular.z = -1;
							matD.specular.w = -1;

							matD.reflectivity = 0;
							matD.specRolloff = 0;

							MGlobal::displayInfo(MString("LAMBERT HERE!!"));





							/*	memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4), &matD.color, sizeof(XMFLOAT4));
							memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
							memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &matD.reflectivity, sizeof(float));
							memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float), &matD.specRolloff, sizeof(float));
							*/
							//1
							//materialNames.append(lambertShader.name());

						}
						else if (connections[u].node().hasFn(MFn::kPhong))
						{
							MPlugArray plugs;
							MFnPhongShader phongShader(connections[u].node());
							phongShader.findPlug("color").connectedTo(plugs, true, false);

							matD.color.x = phongShader.color().r;
							matD.color.y = phongShader.color().g;
							matD.color.z = phongShader.color().b;
							matD.color.w = phongShader.color().a;

							matD.specular.x = phongShader.specularColor().r;
							matD.specular.y = phongShader.specularColor().g;
							matD.specular.z = phongShader.specularColor().b;
							matD.specular.w = phongShader.specularColor().a;

							matD.reflectivity = phongShader.reflectivity();
							matD.specRolloff = 0;

							//matD.cosine = phongShader.cosPower();
							//Controls the size of shiny highlights on the surface. The valid range is 2 to infinity.
							//The slider range is 2 (broad highlight, not very shiny surface) to 100 (small highlight, very shiny surface),
							//though you can type in a higher value. The default value is 20. 






							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4), &matD.color, sizeof(XMFLOAT4));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &matD.reflectivity, sizeof(float));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float), &matD.specRolloff, sizeof(float));
							materialNames.append(phongShader.name());


							MGlobal::displayInfo(MString("Phong!!"));
						}

						else if (connections[u].node().hasFn(MFn::kBlinn))
						{
							MPlugArray plugs;
							MFnBlinnShader blinnShader(connections[u].node());
							blinnShader.findPlug("color").connectedTo(plugs, true, false);

							matD.color.x = blinnShader.color().r;
							matD.color.y = blinnShader.color().g;
							matD.color.z = blinnShader.color().b;
							matD.color.w = blinnShader.color().a;

							matD.specular.x = blinnShader.specularColor().r;
							matD.specular.y = blinnShader.specularColor().g;
							matD.specular.z = blinnShader.specularColor().b;
							matD.specular.w = blinnShader.specularColor().a;

							matD.specRolloff = blinnShader.specularRollOff();
							matD.reflectivity = blinnShader.reflectivity();
							//matD.eccentricity = blinnShader.eccentricity();

							MGlobal::displayInfo(MString("Blinn!!"));


							materialNames.append(blinnShader.name());

							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4), &matD.color, sizeof(XMFLOAT4));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &matD.reflectivity, sizeof(float));
							//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float), &matD.specRolloff, sizeof(float));

						}

					}
					/// Move header
					
				}
					sm.cb->freeMem -= (250 + lastFreeMem);
					sm.cb->head += 250;
					localHead += 250;
					break;

			}

		} while ((sm.cb->freeMem > 250));

	

}





void cameraChange(MFnTransform& transform, MFnCamera& camera)
{

	MPoint eye = camera.eyePoint(MSpace::kWorld);
	MVector viewDirection = camera.viewDirection(MSpace::kWorld);
	MVector upDirection = camera.upDirection(MSpace::kWorld);

	viewDirection += eye;


	message tMessage;

	tMessage.messageType = 1;


	XMFLOAT4X4 viewMatrix;
	DirectX::XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
		XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
		XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
		XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));

	int lastFreeMem = 0;

	do{

		if (sm.cb->freeMem > 250)
		{
			// Sets head to 0 if there are no place to write
			if (sm.cb->head > sm.memSize - 250)
			{
				// The place over will set a number to indicate that the head was moved to 0
				unsigned int type = -1;
				memcpy((char*)sm.buffer + sm.cb->head, &type, sizeof(int));
				lastFreeMem = sm.memSize - sm.cb->head;
				sm.cb->head = 0;

			}
			localHead;

			std::memcpy((char*)sm.buffer + localHead, &tMessage.messageType, sizeof(int));

			std::memcpy((char*)sm.buffer + localHead + sizeof(int), &viewMatrix, sizeof(XMFLOAT4X4));

			/// Move header
			sm.cb->freeMem -= (250 + lastFreeMem);
			sm.cb->head += 250;
			localHead += 250;
			break;
		}

	}while ((sm.cb->freeMem > 250));
	
	


	MGlobal::displayInfo(MString("Camera Change!!!! "));



}

//
//void createdLightCallback(MObject &node, void* clientData)
//{
//	MStatus res;
//
//	//CbIds.append(MNodeMessage::addAttributeChangedCallback(node, lightAttrChangedCallback, &res));
//	//tMessage.messageSize = 3; ???
//
//	//Do things
//	MFnLight lightNode(node);
//	//getLightInfo(lightNode);
//
//}
//
//void lightAttrChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)
//{
//
//	//	std::string plugName(plug.name().asChar());
//
//	//		MString api = plug.node().apiTypeStr();
//
//
//
//	if (msg& MNodeMessage::kAttributeSet)
//	{
//
//
//		message tMessage;
//		tMessage.messageType = 3;
//
//
//		MGlobal::displayInfo(MString("A light ahoy!!"));
//		MFnLight light(plug.node(0));
//
//		MFnTransform lightTransform(light.parent(0));
//
//		//MVector translation = lightTransform.getTranslation(MSpace::kObject);
//		MVector translation = lightTransform.translation(MSpace::kWorld);
//		//double rotation[4];
//		//lightTransform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);
//
//		//MColor color = light.color();
//		MColor colore = light.activeColor();
//		XMFLOAT4 asColorVec;
//		asColorVec.x = colore.r;
//		asColorVec.y = colore.g;
//		asColorVec.z = colore.b;
//		asColorVec.w = colore.a;
//
//
//
//		XMFLOAT4 colorVec;
//		colorVec.x = light.color().r;
//		colorVec.y = light.color().g;
//		colorVec.z = light.color().b;
//		colorVec.w = light.color().a;
//
//
//
//
//	}
//}

//void lightChange(MFnTransform& transform, MPlug &plug)
//{
//
//	unsigned int *headP = (unsigned int*)controlBuf;
//	unsigned int *tailP = headP + 1;
//	unsigned int *readerAmount = headP + 2;
//	unsigned int *freeMem = headP + 3;
//	unsigned int *memSize = headP + 4;
//
//
//
//	message tMessage;
//
//	tMessage.messageType = 3;
//
//
//	//XMFLOAT4X4 viewMatrix;
//	//XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
//	//	XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
//	//	XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
//	//	XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));
//
//
//	//MFnTransform lightTransform(light.parent(0));
//
//	//MFnTransform lightTransform(plug.node());
//	//
//	//MFnLight light;
//
//	MFnLight light(plug.node(0));
//
//	//MFnTransform lightTransform(light.parent(0));
//
//	MFnTransform transfo(plug.node());
//	//MVector translation = lightTransform.getTranslation(MSpace::kObject);
//	MVector translation = transform.translation(MSpace::kWorld);
//
//	//MColor color = light.color();
//
//	/*	XMFLOAT4 colorVec;
//	colorVec.x = light.color().r;
//	colorVec.y = light.color().g;
//	colorVec.z = light.color().b;
//	colorVec.w = light.color().a;*/
//
//
//
//
//	//colorVec.x = 0;
//	//colorVec.y = 1;
//	//colorVec.z = 0;
//	//colorVec.w = 1;
//
//
//
//	memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
//	//
//	memcpy((char*)pBuf + usedSpace + sizeof(int) + sizeof(int) + sizeof(int), &translation, sizeof(XMFLOAT4));
//	//	memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &colorVec, sizeof(XMFLOAT4));
//
//
//
//
//	//memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &viewMatrix, sizeof(XMFLOAT4X4));
//
//
//	*headP += 10000;
//
//
//
//	if (*headP >= *memSize)
//	{
//		*headP = 0;
//	}
//
//	MGlobal::displayInfo(MString("Light has changed!!"));
//
//
//}

//void getLightInfo(MFnLight& lightNode)
//{
//	unsigned int *headP = (unsigned int*)controlBuf;
//	unsigned int *tailP = headP + 1;
//	unsigned int *readerAmount = headP + 2;
//	unsigned int *freeMem = headP + 3;
//	unsigned int *memSize = headP + 4;
//
//
//
//	message tMessage;
//
//	tMessage.messageType = 3;
//
//
//	//XMFLOAT4X4 viewMatrix;
//	//XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
//	//	XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
//	//	XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
//	//	XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));
//
//
//	//MFnTransform lightTransform(light.parent(0));
//
//	//MFnTransform lightTransform(plug.node());
//	//
//	//MFnLight light;
//
//	//replaced with light Directly
//	//MFnLight light(plug.node(0));
//
//
//
//	//MFnTransform lightTransform(light.parent(0));
//	MObject lightTransformNode = lightNode.parent(0);
//
//	MFnTransform transform(lightTransformNode);
//	//MVector translation = lightTransform.getTranslation(MSpace::kObject);
//	MVector translation = transform.translation(MSpace::kWorld);
//
//	//MColor color = light.color();
//
//	XMFLOAT4 colorVec;
//	colorVec.x = lightNode.color().r;
//	colorVec.y = lightNode.color().g;
//	colorVec.z = lightNode.color().b;
//	colorVec.w = lightNode.color().a;
//
//
//
//	//colorVec.x = 1;
//	//colorVec.y = 0;
//	//colorVec.z = 0;
//	//colorVec.w = 1;
//
//	memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
//	//
//	memcpy((char*)pBuf + usedSpace + sizeof(int) + sizeof(int) + sizeof(int), &translation, sizeof(XMFLOAT4));
//	memcpy((char*)pBuf + usedSpace + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4), &colorVec, sizeof(XMFLOAT4));
//
//
//
//
//	//memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &viewMatrix, sizeof(XMFLOAT4X4));
//
//
//
//	*headP += 10000;
//
//
//
//	if (*headP >= *memSize)
//	{
//		*headP = 0;
//	}
//
//
//	MGlobal::displayInfo(MString("Light created!!"));
//
//	tMessage.messageSize = 3;
//
//
//}

void destroyedNodeCallback(MObject& object, MDGModifier& modifier, void* clientData)
{
	MFnMesh mesh(object);

	

	int messageType = 5;

	unsigned int meshCount = nodeNames.length();
	int destroyMesh;
	for (size_t i = 0; i < meshCount; i++)
	{
		if (nodeNames[i] == mesh.name())
		{
			destroyMesh = i;
		}
	}
	MGlobal::displayInfo(MString(mesh.name() + " has changed been destroyed!!"));
	MGlobal::displayInfo(MString("ID = " + destroyMesh));
	//destroyMesh = -1;


		std::memcpy((char*)sm.buffer + localHead, &messageType, sizeof(int));
		//std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &destroyMesh, sizeof(int));
		std::memcpy((char*)sm.buffer + localHead + sizeof(int), &destroyMesh, sizeof(int));



	}
	//Copy index, use to destroy mesh of index in RTV



void getVertexChangeInfo(MFnMesh &meshNode)
{
	MStatus res;


	MItMeshPolygon polyIt(meshNode.object());


	//Data
	MPointArray vert;
	MIntArray vertex;
	float2 uv;
	MVector normal;



	///dataSave

	vector<XMFLOAT4> verticies;
	vector<XMFLOAT2> UV;
	vector<XMFLOAT3> normals;



	while (!polyIt.isDone())
	{
		polyIt.getTriangles(vert, vertex);

		for (size_t i = 0; i < vert.length(); i++)
		{

			polyIt.getUVAtPoint(vert[i], uv);
			polyIt.getNormal(normal);

			verticies.push_back(XMFLOAT4(vert[i].x, vert[i].y, vert[i].z, 1.0f));
			UV.push_back(XMFLOAT2(uv[0], 1 - uv[1]));
			normals.push_back(XMFLOAT3(normal.x, normal.y, normal.z));
		}
		polyIt.next();
	}



	//MGlobal::displayInfo(MString("Number of verts: ") + vertList.length());




	message tMessage;

	tMessage.vert.resize(verticies.size());

	for (int i = 0; i < verticies.size(); i++)
	{
		tMessage.vert[i].norms = XMFLOAT3(normals.at(i).x, normals.at(i).y, normals.at(i).z);
		tMessage.vert[i].pos = XMFLOAT4(verticies.at(i).x, verticies.at(i).y, verticies.at(i).z, verticies.at(i).w);
		tMessage.vert[i].uv = XMFLOAT2(UV.at(i).x, UV.at(i).y);
	}

	tMessage.messageType = 6;



	unsigned int meshCount = nodeNames.length();
	int meshID;

	//MFnMesh nameNode(meshNode.child(0));

	for (size_t i = 0; i < meshCount; i++)
	{
		if (nodeNames[i] == meshNode.name())
		{
			meshID = i;
		}

	}
	MGlobal::displayInfo(MString("ID = " + meshID));

	//Give the mesh an ID
	tMessage.numMeshes = meshID;

	tMessage.numVerts = verticies.size();

	tMessage.messageSize = 10000;
	tMessage.padding = 0;
	

		std::memcpy((char*)sm.buffer + localHead, &tMessage.messageType, sizeof(int));
		std::memcpy((char*)sm.buffer + localHead + sizeof(int), &tMessage.messageSize, sizeof(int));



		std::memcpy((char*)sm.buffer + localHead+ sizeof(int) + sizeof(int), &tMessage.numMeshes, sizeof(int));
		std::memcpy((char*)sm.buffer + localHead+ sizeof(int) + sizeof(int) + sizeof(int), &tMessage.numVerts, sizeof(int));

		for (int i = 0; i < verticies.size(); i++)
		{
			std::memcpy((char*)sm.buffer  + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i), &tMessage.vert[i].pos, sizeof(XMFLOAT4));
			std::memcpy((char*)sm.buffer  + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4), &tMessage.vert[i].uv, sizeof(XMFLOAT2));
			std::memcpy((char*)sm.buffer  + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4) + sizeof(XMFLOAT2), &tMessage.vert[i].norms, sizeof(XMFLOAT3));

		}

		//mesh rotation

		MFnTransform transform(meshNode.parent(0));

		double rotation[4];
		transform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

		double scale[3];
		transform.getScale(scale);

		MVector translation = transform.getTranslation(MSpace::kPostTransform);
		//Build matrix with xmvectors


		XMVECTOR translationVector = XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
		XMVECTOR rotationVector = XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]);
		XMVECTOR scaleVector = XMVectorSet(scale[0], scale[1], scale[2], 0.0f);
		XMVECTOR zeroVector = XMVectorSet(0, 0, 0, 0.0f);


		DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));






		std::memcpy((char*)sm.buffer + localHead + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int), &tMessage.matrixData, sizeof(XMFLOAT4X4));




		////memcpy((char*)pBuf + usedSpace + sizeof(CameraData) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(VertexData)+sizeof(MatrixData), &tMessage.camData, sizeof(CameraData));


		


	delete[] vertData;

}



void getExtrudeChangeInfo(MPlug& plug)
{
	MStatus res;

	MFnMesh meshNode(plug.node());

	MItMeshPolygon polyIt(meshNode.object());


	//Data
	MPointArray vert;
	MIntArray vertex;
	float2 uv;
	MVector normal;

	///dataSave
	vector<XMFLOAT4> verticies;
	vector<XMFLOAT2> UV;
	vector<XMFLOAT3> normals;




	while (!polyIt.isDone())
	{
		polyIt.getTriangles(vert, vertex);

		for (size_t i = 0; i < vert.length(); i++)
		{

			polyIt.getUVAtPoint(vert[i], uv);
			polyIt.getNormal(normal);

			verticies.push_back(XMFLOAT4(vert[i].x, vert[i].y, vert[i].z, 1.0f));
			UV.push_back(XMFLOAT2(uv[0], 1 - uv[1]));
			normals.push_back(XMFLOAT3(normal.x, normal.y, normal.z));
		}
		polyIt.next();
	}

	

	message tMessage;

	tMessage.vert.resize(verticies.size());

	for (int i = 0; i < verticies.size(); i++)
	{
		tMessage.vert[i].norms = XMFLOAT3(normals.at(i).x, normals.at(i).y, normals.at(i).z);
		tMessage.vert[i].pos = XMFLOAT4(verticies.at(i).x, verticies.at(i).y, verticies.at(i).z, verticies.at(i).w);
		tMessage.vert[i].uv = XMFLOAT2(UV.at(i).x, UV.at(i).y);
	}

	tMessage.messageType = 7;



	unsigned int meshCount = nodeNames.length();
	int meshID = meshCount;



	for (size_t i = 0; i < meshCount; i++)
	{
		if (nodeNames[i] == meshNode.name())
		{
			meshID = i;
		}

	}
	//MGlobal::displayInfo(MString("ID = " + meshID));

	//Give the mesh an ID
	tMessage.numMeshes = meshID;

	tMessage.numVerts = verticies.size();

	tMessage.messageSize = 10000;
	tMessage.padding = 0;

	
		std::memcpy((char*)sm.buffer, &tMessage.messageType, sizeof(int));
		std::memcpy((char*)sm.buffer + sizeof(int), &tMessage.messageSize, sizeof(int));



		std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int), &tMessage.numMeshes, sizeof(int));
		std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int), &tMessage.numVerts, sizeof(int));

		for (int i = 0; i < verticies.size(); i++)
		{
			std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i), &tMessage.vert[i].pos, sizeof(XMFLOAT4));
			std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4), &tMessage.vert[i].uv, sizeof(XMFLOAT2));
			std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4X4) + (sizeof(VertexData)*i) + sizeof(XMFLOAT4) + sizeof(XMFLOAT2), &tMessage.vert[i].norms, sizeof(XMFLOAT3));

		}

		//mesh rotation

		MFnTransform transform(meshNode.parent(0));

		double rotation[4];
		transform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

		double scale[3];
		transform.getScale(scale);

		MVector translation = transform.getTranslation(MSpace::kPostTransform);
		//Build matrix with xmvectors


		XMVECTOR translationVector = XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
		XMVECTOR rotationVector = XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]);
		XMVECTOR scaleVector = XMVectorSet(scale[0], scale[1], scale[2], 0.0f);
		XMVECTOR zeroVector = XMVectorSet(0, 0, 0, 0.0f);


		DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));

		std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(int), &tMessage.matrixData, sizeof(XMFLOAT4X4));

		////memcpy((char*)pBuf + usedSpace + sizeof(CameraData) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(VertexData)+sizeof(MatrixData), &tMessage.camData, sizeof(CameraData));


		

	delete[] vertData;

}


void shaderChangedCallback(MObject &node, void* clientData)
{
	

	MStatus res;

	MGlobal::displayInfo(MString("Created?"));


	MFnDependencyNode Shadername(node);

	
	
	MFnLambertShader lambertShader;
	lambertShader.setObject(node);
	MGlobal::displayInfo("Ayy, lamao");
	MGlobal::displayInfo(lambertShader.name());
	materialNames.append(lambertShader.name());
	


	tMessage.messageType = 9;
	tMessage.messageSize = materialNames.length();

		std::memcpy((char*)sm.buffer, &tMessage.messageType, sizeof(int));
		//tempMatID
		std::memcpy((char*)sm.buffer + sizeof(int), &tMessage.messageSize, sizeof(int));
		//hasTexture




CbIds.append(MNodeMessage::addAttributeChangedCallback(node, shaderAttrChanged));
}

////void getMaterialInfo(MFnMesh& mesh)
//{
//
//	//RUNS WHEN CONNECTING MATERIAL TO MESH
//	int instanceNumber = 0;
//	MObjectArray shaders;
//	MIntArray indices;
//
//	MaterialData matD;
//	//BlinnData blinnD;
//	//PhongData phongD;
//
//	MStatus status;
//
//	const MString TEXTURE_NAME("fileTextureName");
//
//
//	mesh.getConnectedShaders(instanceNumber, shaders, indices);
//
//	for (int i = 0; i < shaders.length(); i++)
//	{
//		MPlugArray connections;
//		MFnDependencyNode shaderGroup(shaders[i]);
//		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
//		shaderPlug.connectedTo(connections, true, false);
//		for (int u = 0; u < connections.length(); u++)
//		{
//			if (connections[u].node().hasFn(MFn::kLambert))
//			{
//				MPlugArray plugs;
//				MFnLambertShader lambertShader(connections[u].node());
//				lambertShader.findPlug("color").connectedTo(plugs, true, false);
//
//				matD.color.x = lambertShader.color().r;
//				matD.color.y = lambertShader.color().g;
//				matD.color.z = lambertShader.color().b;
//				matD.color.w = lambertShader.color().a;
//
//				matD.specular.x = -1;
//				matD.specular.y = -1;
//				matD.specular.z = -1;
//				matD.specular.w = -1;
//
//				matD.reflectivity = 0;
//				matD.specRolloff = 0;
//
//				MGlobal::displayInfo(MString("LAMBERT!! THERE"));
//
//
//				tMessage.messageType = 4;
//
//				int materialID = -1;
//				for (size_t i = 0; i < materialNames.length(); i++)
//				{
//					if (materialNames[i] == lambertShader.name())
//					{
//						materialID = i;
//
//					}
//
//				//	if (materialID = -1)
//				//	{
//				//		tMessage.messageType = 8;
//				//
//				//	}
//
//
//
//				}
//
//
//				int meshID = -1;
//				for (size_t i = 0; i < nodeNames.length(); i++)
//				{
//					if (materialNames[i] == mesh.name())
//					{
//						meshID = i;
//
//					}
//
//		
//				}
//
//				//Give the material an ID
//				tMessage.numMeshes = materialID;
//				//Send ID
//				std::memcpy((char*)pBuf + sizeof(int), &tMessage.numMeshes, sizeof(int));
//
//
//				memcpy((char*)pBuf +sizeof(int)+ sizeof(int)+sizeof(int), &matD.color, sizeof(XMFLOAT4));
//				memcpy((char*)pBuf +sizeof(int)+ sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
//				//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &matD.reflectivity, sizeof(float));
//				
//				//TEMP MESH ID TRANSFER
//				memcpy((char*)pBuf + sizeof(int)+sizeof(int), &meshID, sizeof(int));
//
//				//memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float), &matD.specRolloff, sizeof(float));
//				//message type
//				std::memcpy((char*)pBuf, &tMessage.messageType, sizeof(int));
//
//
//			}
//			else if (connections[u].node().hasFn(MFn::kPhong))
//			{
//				MPlugArray plugs;
//				MFnPhongShader phongShader(connections[u].node());
//				phongShader.findPlug("color").connectedTo(plugs, true, false);
//
//				matD.color.x = phongShader.color().r;
//				matD.color.y = phongShader.color().g;
//				matD.color.z = phongShader.color().b;
//				matD.color.w = phongShader.color().a;
//
//				matD.specular.x = phongShader.specularColor().r;
//				matD.specular.y = phongShader.specularColor().g;
//				matD.specular.z = phongShader.specularColor().b;
//				matD.specular.w = phongShader.specularColor().a;
//
//				matD.reflectivity = phongShader.reflectivity();
//				matD.specRolloff = 0;
//
//				//matD.cosine = phongShader.cosPower();
//				//Controls the size of shiny highlights on the surface. The valid range is 2 to infinity.
//				//The slider range is 2 (broad highlight, not very shiny surface) to 100 (small highlight, very shiny surface),
//				//though you can type in a higher value. The default value is 20. 
//
//				tMessage.messageType = 4;
//
//
//				int materialID = -1;
//				for (size_t i = 0; i < nodeNames.length(); i++)
//				{
//					if (materialNames[i] == phongShader.name())
//					{
//						materialID = i;
//
//					}
//					if (materialID = -1)
//					{
//						tMessage.messageType = 8;
//
//					}
//
//
//				}
//
//				
//
//				MGlobal::displayInfo(MString("Phong!!"));
//			}
//
//			else if (connections[u].node().hasFn(MFn::kBlinn))
//			{
//				MPlugArray plugs;
//				MFnBlinnShader blinnShader(connections[u].node());
//				blinnShader.findPlug("color").connectedTo(plugs, true, false);
//
//				matD.color.x = blinnShader.color().r;
//				matD.color.y = blinnShader.color().g;
//				matD.color.z = blinnShader.color().b;
//				matD.color.w = blinnShader.color().a;
//
//				matD.specular.x = blinnShader.specularColor().r;
//				matD.specular.y = blinnShader.specularColor().g;
//				matD.specular.z = blinnShader.specularColor().b;
//				matD.specular.w = blinnShader.specularColor().a;
//
//				matD.specRolloff = blinnShader.specularRollOff();
//				matD.reflectivity = blinnShader.reflectivity();
//				//matD.eccentricity = blinnShader.eccentricity();
//
//				MGlobal::displayInfo(MString("Blinn!!"));
//
//				tMessage.messageType = 4;
//
//				int materialID = -1;
//				for (size_t i = 0; i < nodeNames.length(); i++)
//				{
//					if (materialNames[i] == blinnShader.name())
//					{
//						materialID = i;
//
//					}
//					if (materialID = -1)
//					{
//						tMessage.messageType = 8;
//
//					}
//
//
//				}
//
//				//Give the mesh an ID
//				tMessage.numMeshes = materialID;
//
//				//Send ID
//				std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), &tMessage.numMeshes, sizeof(int));
//
//
//
//				memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4), &matD.color, sizeof(XMFLOAT4));
//				memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
//				memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &matD.reflectivity, sizeof(float));
//				memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float), &matD.specRolloff, sizeof(float));
//			
//				std::memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
//			}
//
//		
//		//look for all the file textures that are upstream from the shader node
//		MItDependencyGraph shaderTextureIter(connections[u].node(), MFn::kFileTexture, MItDependencyGraph::kUpstream,
//			MItDependencyGraph::kBreadthFirst, MItDependencyGraph::kNodeLevel,
//			&status);
//		if (status != MS::kFailure)
//		{
//			shaderTextureIter.disablePruningOnFilter(); //don't prune the iter path for nodes that don't match the filter
//
//			if (!shaderTextureIter.isDone())//did we find any file texture nodes?
//			{
//				MObject textureNode; //current texture node being processed
//
//				for (; !shaderTextureIter.isDone(); shaderTextureIter.next())
//				{
//					textureNode = shaderTextureIter.thisNode(&status);
//					if (status != MS::kFailure)
//					{
//						//attach a node function set so we can get attribute info
//						MFnDependencyNode textureNodeFn(textureNode, &status);
//						if (status != MS::kFailure)
//						{
//							//MPlug attribPlug;
//							//
//							//attribPlug = textureNodeFn.findPlug(MString("fileTextureName"), &status);
//							//int howmany = attribPlug.numChildren();
//							//MString name = attribPlug.name();
//
//
//							MPlug ftnPlug = textureNodeFn.findPlug("ftn");
//							MString fileName;
//							ftnPlug.getValue(fileName);
//							//attribPlug = textureNodeFn.findPlug(MString("imageName"), &status);
//							MGlobal::displayInfo(MString("Texture name is: " + fileName));
//							MGlobal::displayInfo(MString("it somethn"));
//							
//							const char* charname = fileName.asChar();
//
//							int addressMax = sizeof(tMessage.textureAddress);
//
//							if (fileName.length() < addressMax)
//							{
//								for (int i = 0; i < addressMax; i++)
//								{
//									tMessage.textureAddress[i] = charname[i];
//								}
//
//							}
//							else
//								MGlobal::displayInfo(MString("Address contains way many chars."));
//
//							
//							//memcpy((char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &matD.specular, sizeof(XMFLOAT4));
//							memcpy((char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &tMessage.textureAddress, (sizeof(char)* 500));
//
//							
//
//							//if (status != MS::kFailure)
//							//{
//							//	MGlobal::displayInfo(MString("Texture name is: " + textureNodeFn.name()));
//							//	MGlobal::displayInfo(MString("nice"));
//							//
//							//
//							//	//cout << "Texture name: " << textureNodeFn.name().asChar() << endl;
//							//
//							//}//end if, got file name plug
//
//						}//end if, made fn set
//
//					}//end if, made texture object
//
//				}//end for, texture iter
//
//			}//end if, have textures
//
//		}//end if, made texture iter
//
//}
//	
//
//
//		/*if (connections.length() > 0)
//		{
//			MObject src = connections[0];
//
//			if (src.hasFn(MFn::kFileTexture))
//			{
//
//				MFnDependencyNode fnFile(src);
//				MPlug ftnPlug = fnFile.findPlug(TEXTURE_NAME, &status);
//				if (status = MS::kSuccess)
//				{
//					MString fileTextureName;
//					ftnPlug.getValue(fileTextureName);
//				}
//			}
//		}*/
//
//
//	}
//
//
//}

void shaderAttrChanged(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)
{

	MGlobal::displayInfo(MString("Created222 ?"));
	

	MaterialData matD;



	if ((msg & MNodeMessage::kAttributeSet) || msg & MNodeMessage::kConnectionMade)
	{
		//meshNames.append(mesh.name());
		MFnLambertShader lambertShader;
		MFnBlinnShader blinnShader;
		MFnPhongShader phongShader;
		MColor color;
		int whatMaterial = 0;

		// Find the material and then color
		if (plug.node().hasFn(MFn::kLambert))
		{
			lambertShader.setObject(plug.node());
			//MGlobal::displayInfo(lambertShader.name());
			//MGlobal::displayInfo("Swag");
			color = lambertShader.color();
			whatMaterial = 1;
			//materialNames.append(lambertShader.name());

//	int matID = -1;
//	for (size_t i = 0; i < materialNames.length(); i++)
//	{
//		if (nodeNames[i] == lambertShader.name())
//		{
//			matID = i;
//		}
//
//	}
//
//	if (matID == -1)
//	{
//		//2
//		materialNames.append(lambertShader.name());
//
//	}


		}
		else if (plug.node().hasFn(MFn::kBlinn))
		{
			blinnShader.setObject(plug.node());
			MGlobal::displayInfo(blinnShader.name());
			color = blinnShader.color();
			whatMaterial = 2;
		}
		else if (plug.node().hasFn(MFn::kPhong))
		{
			phongShader.setObject(plug.node());
			MGlobal::displayInfo(phongShader.name());
			color = phongShader.color();
			whatMaterial = 3;
		}

		//Texture:
		MFnDependencyNode matNamer(plug.node());
		MObjectArray Files;
		MString filename;
		int pathSize;
		int texExist = 0;
		MPlugArray textureConnect;
		MPlug texturePlug;

		if (whatMaterial == 1)
		{
			texturePlug = lambertShader.findPlug("color");
		}
		else if (whatMaterial == 2)
		{
			texturePlug = blinnShader.findPlug("color");
		}
		else if (whatMaterial == 3)
		{
			texturePlug = phongShader.findPlug("color");
		}

		MGlobal::displayInfo(texturePlug.name());
		texturePlug.connectedTo(textureConnect, true, false);

		if (textureConnect.length() != 0)
		{
			MGlobal::displayInfo(textureConnect[0].name());

			MFnDependencyNode fn(textureConnect[0].node());
			MGlobal::displayInfo(fn.name());

			MPlug ftn = fn.findPlug("ftn");

			ftn.getValue(filename);

			MGlobal::displayInfo(filename);

			pathSize = filename.numChars();

			if (pathSize > 0)
				texExist = 1;
		}

		for (int i = 0; i < materialNames.length(); i++)
		{
			//MGlobal::displayInfo("Test:");
			//MGlobal::displayInfo(materialNames[i]);
			//MGlobal::displayInfo("Check::");
			//MGlobal::displayInfo(matNamer.name());
			if (materialNames[i] == matNamer.name())
			{

		
					tMessage.messageType = 4;
					tMessage.messageSize = i;
					MGlobal::displayInfo("Coosen:");
					MGlobal::displayInfo(matNamer.name());
					std::memcpy((char*)sm.buffer, &tMessage.messageType, sizeof(int));
					//tempMatID
					std::memcpy((char*)sm.buffer + sizeof(int), &tMessage.messageSize, sizeof(int));
					//hasTexture
					std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int), &texExist, sizeof(int));

					if (texExist == 1)
					{
						std::string toString = filename.asChar();
						const char* toChar = toString.c_str();


						std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), toChar, (sizeof(char) * 500));


					}

					matD.color.x = color.r;
					matD.color.y = color.g;
					matD.color.z = color.b;
					matD.color.w = color.a;




					std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int) + sizeof(int), &matD.color, sizeof(DirectX::XMFLOAT4));




				}




			}
		}

		

}

void matChanged(MFnMesh& mesh)
{

	
	// MATERIAL:
	unsigned int instanceNumber = 0;
	MObjectArray shaders;
	MPlugArray test;
	MIntArray indices;
	MPlugArray connections;
	MColor color;
	int whatMaterial = 0;
	MFnLambertShader lambertShader;
	MFnBlinnShader blinnShader;
	MFnPhongShader phongShader;
	MString MatName;

	int localMesh = nodeNames.length();
	for (int i = 0; i < nodeNames.length(); i++)
	{
		if (mesh.name() == nodeNames[i])
		{
			localMesh = i;
		}
	}

	// Find the shadingReasourceGroup
	mesh.getConnectedShaders(instanceNumber, shaders, indices);
	if (shaders.length() != 0)
	{
		MFnDependencyNode shaderGroup(shaders[0]);
		MGlobal::displayInfo(shaderGroup.name());
		MPlug shaderPlug = shaderGroup.findPlug("surfaceShader");
		MGlobal::displayInfo(shaderPlug.name());
		shaderPlug.connectedTo(connections, true, false);

		// Find the material and then color
		if (connections[0].node().hasFn(MFn::kLambert))
		{
			lambertShader.setObject(connections[0].node());
			MGlobal::displayInfo(lambertShader.name());
			color = lambertShader.color();
			whatMaterial = 1;
			MatName = lambertShader.name();

		}
		else if (connections[0].node().hasFn(MFn::kBlinn))
		{
			blinnShader.setObject(connections[0].node());
			MGlobal::displayInfo(blinnShader.name());
			color = blinnShader.color();
			whatMaterial = 2;
			MatName = blinnShader.name();
		}
		else if (connections[0].node().hasFn(MFn::kPhong))
		{
			phongShader.setObject(connections[0].node());
			MGlobal::displayInfo(phongShader.name());
			color = phongShader.color();
			whatMaterial = 3;
			MatName = phongShader.name();
		}

		//Texture:
		MObjectArray Files;
		MString filename;
		int pathSize;
		int texExist = 0;
		MPlugArray textureConnect;
		MPlug texturePlug;

		if (whatMaterial == 1)
		{
			texturePlug = lambertShader.findPlug("color");

		}
		else if (whatMaterial == 2)
		{
			texturePlug = blinnShader.findPlug("color");

		}
		else if (whatMaterial == 3)
		{
			texturePlug = phongShader.findPlug("color");

		}

		MGlobal::displayInfo(texturePlug.name());
		texturePlug.connectedTo(textureConnect, true, false);

		if (textureConnect.length() != 0)
		{
			MGlobal::displayInfo(textureConnect[0].name());

			MFnDependencyNode fn(textureConnect[0].node());
			MGlobal::displayInfo(fn.name());

			MPlug ftn = fn.findPlug("ftn");

			ftn.getValue(filename);

			MGlobal::displayInfo(filename);

			pathSize = filename.numChars();

			if (pathSize > 0)
				texExist = 1;

		}


		

			if (texExist == 1)
			{

				//tMessage.messageType = 4;

				//std::memcpy((char*)sm.buffer, &tMessage.messageType, sizeof(int));
				////hasTexture
				//std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int), &texExist, sizeof(int));

				//std::string toString = filename.asChar();
				//const char* toChar = toString.c_str();

				//std::memcpy((char*)sm.buffer + +sizeof(int) + sizeof(int) + sizeof(int) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), toChar, (sizeof(char) * 500));



			}





		}

		int meshID = -1;
		for (size_t i = 0; i < nodeNames.length(); i++)
		{
			if (nodeNames[i] == mesh.name())
			{
				meshID = i;
			}
		}

		//HERERE, probably doesnt fint mat
		int matID = -1;
		for (size_t i = 0; i < materialNames.length(); i++)
		{
			if (materialNames[i] == MatName)
			{
				matID = i;
			}


		}


		MGlobal::displayInfo(MString("ID = " + meshID));

		//Give the mesh an ID
		tMessage.numMeshes = matID;
		tMessage.messageType = 8;
		tMessage.messageSize = meshID;

		tMessage.padding = 0;


		/*std::memcpy((char*)sm.buffer, &tMessage.messageType, sizeof(int));
		std::memcpy((char*)sm.buffer + sizeof(int), &tMessage.messageSize, sizeof(int));



		std::memcpy((char*)sm.buffer + sizeof(int) + sizeof(int), &tMessage.numMeshes, sizeof(int));*/


		//	// Send data to shared memory
		//	do
		//	{
		//		if (sm.cb->freeMem >= slotSize/* && sm.cb->head < sm.memSize - sm.cb->freeMem*/)
		//		{
		//			// Sets head to 0 if there are no place to write
		//			if (sm.cb->head == sm.memSize)
		//				sm.cb->head = 0;
		//
		//			localHead = sm.cb->head;
		//
		//			// Message header
		//			sm.msgHeader.type = TMaterialUpdate;
		//			if (texExist == 0)
		//			{
		//				sm.msgHeader.byteSize = sm.msgHeaderSize + sizeof(MColor)+sizeof(int)+sizeof(int);
		//				sm.msgHeader.byteSize += slotSize - sm.msgHeader.byteSize;
		//			}
		//			else
		//			{
		//				sm.msgHeader.byteSize = sm.msgHeaderSize + sizeof(MColor)+sizeof(int)+sizeof(int)+pathSize + sizeof(int);
		//				sm.msgHeader.byteSize += slotSize - sm.msgHeader.byteSize;
		//			}
		//			memcpy((char*)sm.buffer + localHead, &sm.msgHeader, sm.msgHeaderSize);
		//			localHead += sm.msgHeaderSize;
		//
		//			// Mesh ID
		//			memcpy((char*)sm.buffer + localHead, &localMesh, sizeof(int));
		//			localHead += sizeof(int);
		//
		//			// Material data
		//			memcpy((char*)sm.buffer + localHead, &color, sizeof(MColor));
		//			localHead += sizeof(MColor);
		//
		//			//Bool for Texture
		//			XMINT4 ha = XMINT4(texExist, 0, 0, 0);
		//			memcpy((char*)sm.buffer + localHead, &ha.x, sizeof(int));
		//			localHead += sizeof(int);
		//
		//			if (texExist == 1)
		//			{
		//				//Texture Size
		//				memcpy((char*)sm.buffer + localHead, &pathSize, sizeof(int));
		//				localHead += sizeof(int);
		//
		//				//Texture data
		//				memcpy((char*)sm.buffer + localHead, filename.asChar(), pathSize);
		//				localHead += pathSize;
		//			}
		//
		//			// Move header
		//			sm.cb->freeMem -= slotSize;
		//			sm.cb->head += slotSize;
		//			break;
		//		}
		//	} while (sm.cb->freeMem >!slotSize);


		
}

