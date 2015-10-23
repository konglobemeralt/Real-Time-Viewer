// UD1414_Plugin.cpp : Defines the exported functions for the DLL application.

//Js. To add:
/*
- Free memory check to compare to BUF_SIZE in order to know if new information is sent to realtimeviewer
- We also need that in RTV.

- upvector, view matrices etc from camera

- ??????

- profit

*/

#include "maya_includes.h"
#include <iostream>
#include <fstream>
#include <DirectXMath.h>
#include <Windows.h>
#include <vector>


using namespace DirectX;
using namespace std;

void createdNodeCallback(MObject &node, void* clientData);
void createdTransformCallback(MObject &node, void* clientData);
void renamedNodeCallback(MObject &node, const MString &str, void* clientData);

void createdLightCallback(MObject &node, void* clientData);
void lightAttrChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);

void meshAttributeChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);
void transformChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData);

void destroyedNodeCallback(MObject& object, MDGModifier& modifier, void* clientData);

void timerCB(float elapsedTime, float lastTime, void* clientData);


MStringArray nodeNames;

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
void getLightInfo(MFnLight&);



void cameraChange(MFnTransform& transform, MFnCamera& camera);
void lightChange(MFnTransform& transform, MPlug &plug);


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


HANDLE hMapFile;
void* pBuf;
HANDLE controlFile;
void* controlBuf;

int usedSpace = 0;


#define BUF_SIZE 1024*1024*100
TCHAR globName[] = TEXT("Global\\testMap");
TCHAR globName2[] = TEXT("Global\\controlFileMap");
//TCHAR globName[] = TEXT("testMap");
//TCHAR globName2[] = TEXT("controlFileMap");


// called when the plugin is loaded
EXPORT MStatus initializePlugin(MObject obj)
{
	// most functions will use this variable to indicate for errors
	MStatus res = MS::kSuccess;


	MFnPlugin myPlugin(obj, "Maya plugin", "1.0", "Any", &res);
	if (MFAIL(res))  {
		CHECK_MSTATUS(res);
	}


	hMapFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,        //use paging file
		NULL,                        //default security
		PAGE_READWRITE,                //read/write access
		0,                            //high order DWORD maximum size
		BUF_SIZE,                        //low order DWORD maximum size
		globName);                    //name of mapping object

	//check if errors are happening
	if (hMapFile == NULL)
	{
		MGlobal::displayInfo("hmapfile Could not create file mapping object.");
	}

	//calling api to create a view to ALL the memory of the file map (memory being specified in command/bash)
	pBuf = MapViewOfFile(hMapFile, FILE_MAP_ALL_ACCESS, 0, 0, BUF_SIZE);

	//error check again
	if (pBuf == NULL)
	{
		MGlobal::displayInfo("pbuf Could not create map view of file.");
		CloseHandle(hMapFile);
	}

	//File mapping


	controlFile = CreateFileMapping(
		INVALID_HANDLE_VALUE,        //use paging file
		NULL,                        //default security
		PAGE_READWRITE,                //read/write access
		0,                            //high order DWORD maximum size
		sizeof(Headtail),                        //low order DWORD maximum size
		globName2);                    //name of mapping object

	//check if errors are happening
	if (controlFile == NULL)
	{
		MGlobal::displayInfo("controlfile Could not create file mapping object.");

	}

	//calling api to create a view to ALL the memory of the file map (memory being specified in command/bash)
	controlBuf = MapViewOfFile(controlFile, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(Headtail));
	//error check again
	if (controlBuf == NULL)
	{
		MGlobal::displayInfo("controlbuf Could not create map view of file.");
		CloseHandle(controlFile);

	}

	unsigned int zero = 0;
	unsigned int mem = BUF_SIZE;
	unsigned int space = mem;


	std::memcpy((char*)controlBuf, &zero, sizeof(unsigned int));
	std::memcpy((char*)controlBuf + (sizeof(unsigned int)* 1), &zero, sizeof(unsigned int));
	std::memcpy((char*)controlBuf + (sizeof(unsigned int)* 2), &zero, sizeof(unsigned int));
	std::memcpy((char*)controlBuf + ((sizeof(unsigned int)* 3)), &space, sizeof(unsigned int));
	std::memcpy((char*)controlBuf + ((sizeof(unsigned int)* 4)), &mem, sizeof(unsigned int));


	CbIds.append(MDGMessage::addNodeAddedCallback(createdNodeCallback, "mesh", &res));
	CbIds.append(MDGMessage::addNodeAddedCallback(createdTransformCallback, "transform", &res));
	CbIds.append(MDGMessage::addNodeAddedCallback(createdLightCallback, "light", &res));
	
	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChangedCallback, &res));

	


	MGlobal::displayInfo("Maya plugin loaded!!");
	MGlobal::displayInfo("HEJ!!");

	goThroughScene();

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


	UnmapViewOfFile((char*)pBuf);
	UnmapViewOfFile((char*)controlBuf);
	CloseHandle(hMapFile);
	CloseHandle(controlFile);



	MGlobal::displayInfo("Maya plugin unloaded!!");

	return MS::kSuccess;
}

void createdNodeCallback(MObject &node, void* clientData)
{
	//append new callbacks as objects are created
	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, meshAttributeChangedCallback));
	CbIds.append(MNodeMessage::addNodeAboutToDeleteCallback(node, destroyedNodeCallback));
	

}

void meshAttributeChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)

{

	std::string plugName(plug.name().asChar());

	MString thePartialName = plug.partialName();
	MGlobal::displayInfo(MString(thePartialName));




	if (plugName.find("doubleSided") != std::string::npos && MNodeMessage::AttributeMessage::kAttributeSet)
	{
		MStatus res;
		MFnMesh meshNode(plug.node(), &res);

		usedSpace = 0;

		unsigned int *headP = (unsigned int*)controlBuf;
		unsigned int *tailP = headP + 1;
		unsigned int *readerAmount = headP + 2;
		unsigned int *freeMem = headP + 3;
		unsigned int *memSize = headP + 4;

		//Dummy check for success!
		MGlobal::displayInfo("Mesh: " + meshNode.fullPathName() + " created!");
		getMeshInfo(meshNode);
	}

	// Vertex has changed
	if(strstr(plug.partialName().asChar(), "pt[") )
	{
		
		MStatus res;
		MFnMesh meshNode(plug.node(), &res);
		getVertexChangeInfo(meshNode);
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

void renamedNode(MObject &node, const MString &str, void* clientData)
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
		unsigned int *headP = (unsigned int*)controlBuf;
		unsigned int *tailP = headP + 1;
		unsigned int *readerAmount = headP + 2;
		unsigned int *freeMem = headP + 3;
		unsigned int *memSize = headP + 4;

		MFnTransform transform(plug.node());

		if (transform.isParentOf(camera.object()))
		{
			cameraChange(transform, camera);
		}

		else if (strstr(name.asChar(), "Light"))
		{
			lightChange(transform, plug.parent());
			tMessage.messageSize = 3;
		}


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


				DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));



				tMessage.messageType = 2;
			
				std::memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));

				std::memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4), &tMessage.matrixData, sizeof(XMFLOAT4X4));

				
				int meshCount = nodeNames.length();
				int meshID = -1;

				MFnMesh nameNode(transform.child(0));
				for (size_t i = 0; i < meshCount; i++)
				{
					if (nodeNames[i] == nameNode.name())
					{
						meshID = i;

					}
					else
					{
						meshID = 0;
					}
						
				}


				//Give the mesh an ID
				tMessage.numMeshes = meshID;

				//Send ID
				std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &tMessage.numMeshes, sizeof(int));




				*headP += 100000;



				if (*headP > *memSize)
				{
					*headP = 0;
				}


			

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
			getLightInfo(light);
			CbIds.append(MNodeMessage::addAttributeChangedCallback(light.object(), lightAttrChangedCallback));
			
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


	

	
}


void getMeshInfo(MFnMesh &meshNode)
{
	MStatus res;

	//Add to namelist
	nodeNames.append(meshNode.name());

	MGlobal::executeCommand("select "+ meshNode.name(), false, true);
	MGlobal::executeCommand("polyTriangulate", false, true);
	
	MItMeshPolygon polyIt(meshNode.object());

	
	//Data
	MPointArray verts;
	
	MFloatArray us;
	MFloatArray vs;
	MFloatVectorArray normal;
	
	int uvIndex;
	//Data end

	///dataSave

	vector<XMFLOAT4> verticies;
	vector<XMFLOAT2> UV;
	vector<XMFLOAT3> normals;

	
	meshNode.getPoints(verts);
	meshNode.getUVs(us, vs);
	meshNode.getNormals(normal);


	while (!polyIt.isDone())
	{
		int triangleCount = 0;
		for (int i = 0; i < 3; i++)
		{
			//Vert
			verticies.push_back(XMFLOAT4(verts[polyIt.vertexIndex(i)].x, verts[polyIt.vertexIndex(i)].y, verts[polyIt.vertexIndex(i)].z, 1.0f));

			//UV
			polyIt.getUVIndex(i, uvIndex, 0);
			UV.push_back(XMFLOAT2(us[uvIndex], 1 - vs[uvIndex]));

			//Normals
			normals.push_back(XMFLOAT3(normal[polyIt.normalIndex(i)].x, normal[polyIt.normalIndex(i)].y, normal[polyIt.normalIndex(i)].z));

			}

		polyIt.next();
	}


	MGlobal::executeCommand("undo", false, true);
	MGlobal::executeCommand("undo", false, true);




	usedSpace = 0;

	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;

	
	

		//MGlobal::displayInfo(MString("Number of verts: ") + vertList.length());


	


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
			else
			{
				meshID = 0;
			}
		}


		//Give the mesh an ID
		tMessage.numMeshes = meshID;

		tMessage.numVerts = verticies.size();

		tMessage.messageSize = 100000;
		tMessage.padding = 0;


		std::memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
		std::memcpy((char*)pBuf + usedSpace + sizeof(int), &tMessage.messageSize, sizeof(int));
		std::memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int), &tMessage.padding, sizeof(int));


		std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &tMessage.numMeshes, sizeof(int));
		std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int), &tMessage.numVerts, sizeof(int));

		for (int i = 0; i < verticies.size(); i++)
		{
			std::memcpy((char*)pBuf + usedSpace +sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int), &tMessage.vert[i].pos, sizeof(XMFLOAT4));
			std::memcpy((char*)pBuf + usedSpace +sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &tMessage.vert[i].uv, sizeof(XMFLOAT2));
			std::memcpy((char*)pBuf + usedSpace +sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4)+sizeof(XMFLOAT2), &tMessage.vert[i].norms, sizeof(XMFLOAT3));

		}


		//Mesh transformation
		MFnTransform meshTransform(meshNode.parent(0));
		MVector translation = meshTransform.getTranslation(MSpace::kObject);
		
		//mesh rotation
		double rotation[4];
		meshTransform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

		//mesh scale
		//For later

		//Build matrix with xmvectors

		XMVECTOR translationVector = XMVectorSet(translation.x, translation.y, translation.z, 1.0f);
		XMVECTOR rotationVector = XMVectorSet(rotation[0], rotation[1], rotation[2], rotation[3]);
		//Temp scale 1 1 1 
		XMVECTOR scaleVector = XMVectorSet(1.0f, 1.0f, 1.0f, 0.0f);
		XMVECTOR zeroVector = XMVectorSet(0.0f, 0.0f, 0.0f, 0.0f);
		
		
		DirectX::XMStoreFloat4x4(&tMessage.matrixData, XMMatrixAffineTransformation(scaleVector, zeroVector, rotationVector, translationVector));


		
		std::memcpy((char*)pBuf + usedSpace +sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &tMessage.matrixData, sizeof(XMFLOAT4X4));

		////memcpy((char*)pBuf + usedSpace + sizeof(CameraData) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(VertexData)+sizeof(MatrixData), &tMessage.camData, sizeof(CameraData));


		*headP += 100000;



			if (*headP > *memSize)
			{
				*headP = 0;
			}



		

		delete[] vertData;

	}





void cameraChange(MFnTransform& transform, MFnCamera& camera)
{

	MPoint eye = camera.eyePoint(MSpace::kWorld);
	MVector viewDirection = camera.viewDirection(MSpace::kWorld);
	MVector upDirection = camera.upDirection(MSpace::kWorld);
	
	viewDirection += eye;

			
	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;

	message tMessage;

	tMessage.messageType = 1;


	XMFLOAT4X4 viewMatrix;
	DirectX::XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
	XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
	XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
	XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));


	std::memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));

	std::memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4), &viewMatrix, sizeof(XMFLOAT4X4));
	


			*headP += 100000;

			if (*headP > *memSize)
			{
				*headP = 0;
			}



			MGlobal::displayInfo(MString("Camera Change!!!! "));



}


void createdLightCallback(MObject &node, void* clientData)
{
	MStatus res;

	CbIds.append(MNodeMessage::addAttributeChangedCallback(node, lightAttrChangedCallback, &res));
	//tMessage.messageSize = 3; ???

	//Do things
	MFnLight lightNode(node);
	getLightInfo(lightNode);

}

void lightAttrChangedCallback(MNodeMessage::AttributeMessage msg, MPlug &plug, MPlug &plug2, void *clientData)
{

	//	std::string plugName(plug.name().asChar());

	//		MString api = plug.node().apiTypeStr();

	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;

	if (*headP != *tailP)
	{
		if (msg& MNodeMessage::kAttributeSet)
		{


			message tMessage;
			tMessage.messageType = 3;


			MGlobal::displayInfo(MString("A light ahoy!!"));
			MFnLight light(plug.node(0));

			MFnTransform lightTransform(light.parent(0));

			//MVector translation = lightTransform.getTranslation(MSpace::kObject);
			MVector translation = lightTransform.translation(MSpace::kWorld);
			//double rotation[4];
			//lightTransform.getRotationQuaternion(rotation[0], rotation[1], rotation[2], rotation[3]);

			//MColor color = light.color();
			MColor colore = light.activeColor();
			XMFLOAT4 asColorVec;
			asColorVec.x = colore.r;
			asColorVec.y = colore.g;
			asColorVec.z = colore.b;
			asColorVec.w = colore.a;



			XMFLOAT4 colorVec;
			colorVec.x = light.color().r;
			colorVec.y = light.color().g;
			colorVec.z = light.color().b;
			colorVec.w = light.color().a;



			memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
			//
			memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &translation, sizeof(XMFLOAT4));
			memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &colorVec, sizeof(XMFLOAT4));

			*headP += 100000;

			if (*headP > *memSize)
			{
				*headP = 0;
			}


		}
		tMessage.messageSize = 3;

	}
	tMessage.messageSize = 3;

}


void lightChange(MFnTransform& transform, MPlug &plug)
{

	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;


	if (*headP != *tailP)
	{
		message tMessage;

		tMessage.messageType = 3;


		//XMFLOAT4X4 viewMatrix;
		//XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
		//	XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
		//	XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
		//	XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));


		//MFnTransform lightTransform(light.parent(0));

		//MFnTransform lightTransform(plug.node());
		//
		//MFnLight light;

		MFnLight light(plug.node(0));

		//MFnTransform lightTransform(light.parent(0));

		MFnTransform transfo(plug.node());
		//MVector translation = lightTransform.getTranslation(MSpace::kObject);
		MVector translation = transform.translation(MSpace::kWorld);

		//MColor color = light.color();

		XMFLOAT4 colorVec;
		colorVec.x = light.color().r;
		colorVec.y = light.color().g;
		colorVec.z = light.color().b;
		colorVec.w = light.color().a;

		//MColor color = light.color();
		MColor colore = light.activeColor();
		XMFLOAT4 asColorVec;
		asColorVec.x = colore.r;
		asColorVec.y = colore.g;
		asColorVec.z = colore.b;
		asColorVec.w = colore.a;



		memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
		//
		memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &translation, sizeof(XMFLOAT4));
		memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &colorVec, sizeof(XMFLOAT4));




		//memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &viewMatrix, sizeof(XMFLOAT4X4));



		*headP += 100000;

		if (*headP > *memSize)
		{
			*headP = 0;
		}


		MGlobal::displayInfo(MString("Light has changed!!"));
	}
	tMessage.messageSize = 3;

}

void getLightInfo(MFnLight& lightNode)
{
	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;


	if (*headP != *tailP)
	{
		message tMessage;

		tMessage.messageType = 3;


		//XMFLOAT4X4 viewMatrix;
		//XMStoreFloat4x4(&viewMatrix, (DirectX::XMMatrixLookAtRH(
		//	XMVectorSet(eye.x, eye.y, eye.z, 1.0f),
		//	XMVectorSet(viewDirection.x, viewDirection.y, viewDirection.z, 0.0f),
		//	XMVectorSet(upDirection.x, upDirection.y, upDirection.z, 0.0f))));


		//MFnTransform lightTransform(light.parent(0));

		//MFnTransform lightTransform(plug.node());
		//
		//MFnLight light;

		//replaced with light Directly
		//MFnLight light(plug.node(0));



		//MFnTransform lightTransform(light.parent(0));
		MObject lightTransformNode = lightNode.parent(0);
		
		MFnTransform transform(lightTransformNode);
		//MVector translation = lightTransform.getTranslation(MSpace::kObject);
		MVector translation = transform.translation(MSpace::kWorld);

		//MColor color = light.color();

		XMFLOAT4 colorVec;
		colorVec.x = lightNode.color().r;
		colorVec.y = lightNode.color().g;
		colorVec.z = lightNode.color().b;
		colorVec.w = lightNode.color().a;

		//MColor color = light.color();
		MColor colore = lightNode.activeColor();
		XMFLOAT4 asColorVec;
		asColorVec.x = colore.r;
		asColorVec.y = colore.g;
		asColorVec.z = colore.b;
		asColorVec.w = colore.a;



		memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
		//
		memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &translation, sizeof(XMFLOAT4));
		memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &colorVec, sizeof(XMFLOAT4));




		//memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &viewMatrix, sizeof(XMFLOAT4X4));



		*headP += 100000;

		if (*headP > *memSize)
		{
			*headP = 0;
		}


		MGlobal::displayInfo(MString("Light has changed!!"));
	}
	tMessage.messageSize = 3;


}

void destroyedNodeCallback(MObject& object, MDGModifier& modifier, void* clientData)
{
	MFnMesh mesh(object);

	unsigned int *headP = (unsigned int*)controlBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;

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

	std::memcpy((char*)pBuf + usedSpace, &messageType, sizeof(int));
	std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &destroyMesh, sizeof(int));
	


	//Copy index, use to destroy mesh of index in RTV
}


void getVertexChangeInfo(MFnMesh &meshNode)
{
	MStatus res;

	
	//MGlobal::executeCommand("select " + meshNode.name(), false, true);
	//MGlobal::executeCommand("polyTriangulate", false, true);
	//
	//MItMeshPolygon polyIt(meshNode.object());
	//
	//
	////Data
	//MPointArray verts;
	//
	//MFloatArray us;
	//MFloatArray vs;
	//MFloatVectorArray normal;
	//
	//int uvIndex;
	////Data end
	//
	/////dataSave
	//
	//vector<XMFLOAT4> verticies;
	//vector<XMFLOAT2> UV;
	//vector<XMFLOAT3> normals;
	//
	//
	//meshNode.getPoints(verts);
	//meshNode.getUVs(us, vs);
	//meshNode.getNormals(normal);
	//
	//
	//while (!polyIt.isDone())
	//{
	//	int triangleCount = 0;
	//	for (int i = 0; i < 3; i++)
	//	{
	//		//Vert
	//		verticies.push_back(XMFLOAT4(verts[polyIt.vertexIndex(i)].x, verts[polyIt.vertexIndex(i)].y, verts[polyIt.vertexIndex(i)].z, 1.0f));
	//
	//		//UV
	//		polyIt.getUVIndex(i, uvIndex, 0);
	//		UV.push_back(XMFLOAT2(us[uvIndex], 1 - vs[uvIndex]));
	//
	//		//Normals
	//		normals.push_back(XMFLOAT3(normal[polyIt.normalIndex(i)].x, normal[polyIt.normalIndex(i)].y, normal[polyIt.normalIndex(i)].z));
	//
	//	}
	//
	//	polyIt.next();
	//}
	//
	//
	//MGlobal::executeCommand("undo", false, true);
	//MGlobal::executeCommand("undo", false, true);
	//
	//
	//
	//
	//usedSpace = 0;
	//
	//unsigned int *headP = (unsigned int*)controlBuf;
	//unsigned int *tailP = headP + 1;
	//unsigned int *readerAmount = headP + 2;
	//unsigned int *freeMem = headP + 3;
	//unsigned int *memSize = headP + 4;
	//
	//
	//
	//
	////MGlobal::displayInfo(MString("Number of verts: ") + vertList.length());
	//
	//
	//
	//
	//
	//message tMessage;
	//
	//tMessage.vert.resize(verticies.size());
	//
	//for (int i = 0; i < verticies.size(); i++)
	//{
	//	tMessage.vert[i].norms = XMFLOAT3(normals.at(i).x, normals.at(i).y, normals.at(i).z);
	//	tMessage.vert[i].pos = XMFLOAT4(verticies.at(i).x, verticies.at(i).y, verticies.at(i).z, verticies.at(i).w);
	//	tMessage.vert[i].uv = XMFLOAT2(UV.at(i).x, UV.at(i).y);
	//}
	//
	//tMessage.messageType = 0;
	//
	//
	//
	//
	//
	//unsigned int meshCount = nodeNames.length();
	//int meshID;
	//
	////MFnMesh nameNode(meshNode.child(0));
	//
	//
	//
	//
	//std::memcpy((char*)pBuf + usedSpace, &tMessage.messageType, sizeof(int));
	//std::memcpy((char*)pBuf + usedSpace + sizeof(int), &tMessage.messageSize, sizeof(int));
	//std::memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int), &tMessage.padding, sizeof(int));
	//
	//
	//std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4), &tMessage.numMeshes, sizeof(int));
	//std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4X4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int), &tMessage.numVerts, sizeof(int));
	//
	//for (int i = 0; i < verticies.size(); i++)
	//{
	//	std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int), &tMessage.vert[i].pos, sizeof(XMFLOAT4));
	//	std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &tMessage.vert[i].uv, sizeof(XMFLOAT2));
	//	std::memcpy((char*)pBuf + usedSpace + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(XMFLOAT4X4)+(sizeof(VertexData)*i) + sizeof(int)+sizeof(int)+sizeof(XMFLOAT4X4)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4)+sizeof(XMFLOAT2), &tMessage.vert[i].norms, sizeof(XMFLOAT3));
	//
	//}
	//
	//
	//
	//tMessage.numVerts = verticies.size();
	//
	//tMessage.messageSize = 100000;
	//tMessage.padding = 0;
	//for (size_t i = 0; i < meshCount; i++)
	//	{
	//		if (nodeNames[i] == meshNode.name())
	//		{
	//			meshID = i;
	//		}
	//		else
	//		{
	//			meshID = 0;
	//		}
	//	}
	//
	////Give the mesh an ID
	//tMessage.numMeshes = meshID;
	////Mesh transformation
	//MFnTransform meshTransform(meshNode.parent(0));
	//MVector translation = meshTransform.getTranslation(MSpace::kObject);
	//
	//
	//
	//
	//*headP += 100000;
	//
	//
	//
	//if (*headP > *memSize)
	//{
	//	*headP = 0;
	//}
	//
	//
	//
	//
	//
	//delete[] vertData;

}

