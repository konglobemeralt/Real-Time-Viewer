
#include <iostream>
#include <fstream>
#include <vector>
#include <string>
#include <DirectXMath.h>
#include <windows.h>


enum Type
{
	TMeshCreate,
	TMeshUpdate,
	TAddedVertex,
	TVertexUpdate,
	TNormalUpdate,
	TUVUpdate,
	TtransformUpdate,
	TCameraUpdate,
	TLightCreate,
	TLightUpdate,
	TMeshDestroyed,
	TMaterialUpdate,
	TMaterialChanged,
	TAmount
};


class SharedMem
{

public:

	SharedMem();
	~SharedMem();

	char* openSM(float size);
	char* closeSM();

	HANDLE fmCB;
	HANDLE fmMain;

	struct CircBuffer
	{
		unsigned int freeMem;
		unsigned int head;
		unsigned int tail;
	}*cb;
	unsigned int cbSize;

	struct MSGHeader
	{
		unsigned int type;
		unsigned int byteSize;
	}msgHeader;
	unsigned int msgHeaderSize;

	size_t memSize;
	void* buffer;

	// MESHES
	std::vector<DirectX::XMFLOAT3> pos;
	std::vector<DirectX::XMFLOAT2> uv;
	std::vector<DirectX::XMFLOAT3> normal;
	std::vector<DirectX::XMFLOAT3> vertices;

	// Camera
	unsigned camDataSize;
};

