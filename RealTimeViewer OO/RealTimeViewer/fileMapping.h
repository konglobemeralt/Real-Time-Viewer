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
	

	// SHARED MEOMRY
	HANDLE smCircle;
	HANDLE smMess;


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

		
	

	
};

#endif