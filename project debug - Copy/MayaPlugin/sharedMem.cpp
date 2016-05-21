#include "sharedMem.h"

SharedMemory::SharedMemory() {}

SharedMemory::~SharedMemory() {}

char* SharedMemory::OpenMemory(float size)
{
	//SE_CREATE_GLOBAL_NAME;
	size *= 1024 * 1024;
	memSize = size;
	// Circular buffer data
	smCircle = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		sizeof(CircBuffer),
		L"Global/CircularBuffer3");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return "CircularBuffer allready exist\n";

	if (smCircle == NULL)
		return "Could not open file mapping object! -> CircularBuffer";

	cb = (CircBuffer*)MapViewOfFile(smCircle, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (cb == NULL)
	{
		CloseHandle(cb);
		return "Could not map view of file!";
	}

	if (GetLastError() != ERROR_ALREADY_EXISTS)
	{
		cb->head = 0;
		cb->tail = 0;
		cb->freeMem = size;
	}

	// Main data
	smMess = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		(DWORD)0,
		size,
		L"Global/MainData3");
	if (GetLastError() == ERROR_ALREADY_EXISTS)
		return "MainData allready exist\n";

	if (smMess == NULL)
		return "Could not open file mapping object! -> MainData";

	buffer = MapViewOfFile(smMess, FILE_MAP_ALL_ACCESS, 0, 0, 0);
	if (buffer == NULL)
	{
		CloseHandle(buffer);
		return "Could not map view of file!";
	}
	return "Shared memory open success!";
}

char* SharedMemory::CloseMemory()
{
	char* status = NULL;

	if (UnmapViewOfFile(cb) == 0)
		status = "Failed unmap CircBuffer!";
	if (CloseHandle(smCircle) == 0)
		status = "Failed close fmCB!";
	if (UnmapViewOfFile(buffer) == 0)
		status = "Failed unmap buffer!";
	if (CloseHandle(smMess) == 0)
		status = "Failed unmap fmMain!";
	if (status == NULL)
		status = "Closed all maps!";

	return status;
}