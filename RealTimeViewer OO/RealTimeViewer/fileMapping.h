#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <d3d11.h>
#include <d3dcompiler.h>
#include <DirectXMath.h>
#include <vector>
#include <string>
#include <iostream>
#include <windows.h>

#include <fstream>

using namespace DirectX;
using namespace std;

#pragma comment (lib, "d3d11.lib")
#pragma comment (lib, "d3dcompiler.lib")

using namespace DirectX;

class SharedMemory
{
public:

	SharedMemory();
	~SharedMemory();

	void OpenMemory(float size);
	int ReadMSGHeader();
	void ReadMemory(unsigned int type);

	// SHARED MEOMRY
	HANDLE fmCB;
	HANDLE fmMain;
	unsigned int slotSize;
	unsigned int localTail;

	struct CircBuffer
	{
		unsigned int freeMem;
		unsigned int head;
		unsigned int tail;
	}*cb;
	unsigned int localFreeMem;

	size_t memSize;
	void* buffer;

	// MESSAGE HEADER
	struct MSGHeader
	{
		unsigned int type;
		unsigned int byteSize;
	}msgHeader;

	// MESH
	struct meshTexture
	{
		XMINT4 textureExist;
		XMFLOAT4 materialColor;
	};
	struct MeshData
	{
		//VertexData* vertexData;
		XMFLOAT3* pos;
		XMFLOAT2* uv;
		XMFLOAT3* normal;
		unsigned int vertexCount;
		XMFLOAT4X4* transform;
		ID3D11Buffer* meshesBuffer[3];
		ID3D11Buffer* transformBuffer;

		//Material:
		ID3D11Buffer* colorBuffer;
		meshTexture meshTex;
		ID3D11ShaderResourceView* meshTextures;

		//Texture:
		unsigned int textureSize;
		char* texturePath;
	};
	vector<MeshData> meshes;
	unsigned int localMesh;
	unsigned int localVertex;
	XMFLOAT3 vtxChanged;
	unsigned int meshSize;

	// TEXTURES
	//vector<ID3D11ShaderResourceView*> meshTextures;

	// TRANSFORM
	vector<string> tranformNames;
	vector<XMFLOAT4X4> transforms;
	vector<ID3D11Buffer*> transformBuffers;

	// CAMERA
	XMFLOAT4X4* view;
	XMFLOAT4X4* projection;
	XMFLOAT4X4 projectionTemp;
	ID3D11Buffer* viewMatrix;
	ID3D11Buffer* projectionMatrix;
	D3D11_MAPPED_SUBRESOURCE mapSub;

	struct CameraData
	{
		double pos[3];
		double view[3];
		double up[3];
	}*cameraData;
	XMFLOAT4X4* testViewMatrix;

	//LIGHT
	struct LightData
	{
		XMFLOAT4 pos;
		XMFLOAT4 color;
	};
	struct Lights
	{
		LightData* lightData;
		ID3D11Buffer* lightBuffer;
	}light;
	int localLight;
};

#endif