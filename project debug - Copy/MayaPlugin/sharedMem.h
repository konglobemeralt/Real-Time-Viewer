#ifndef SHAREDMEMORY_H
#define SHAREDMEMORY_H

#include <vector>
#include <string>
#include <iostream>
#include <windows.h>
#include <DirectXMath.h>
#include <fstream>

using namespace std;
using namespace DirectX;

class SharedMemory
{
public:

	SharedMemory();
	~SharedMemory();

	char* OpenMemory(float size);
	char* CloseMemory();


	HANDLE smCircle;
	HANDLE smMess;

	struct CircBuffer
	{
		unsigned int freeMem;
		unsigned int head;
		unsigned int tail;
	}*cb;
	unsigned int cbSize;


	size_t memSize;
	void* buffer;


};

#endif