#include "fileMapping.h"

#define BUFFER_SIZE 1024*1024*100

TCHAR globName[] = TEXT("Global\\testMap");
TCHAR globName2[] = TEXT("Global\\controlFileMap");

fileMapping::fileMapping()
{
	pBuf_ = 0;
	hMapFile_ = 0;


	//controll buffer
	headTail_.m_head = 0;
	headTail_.m_tail = 0;
	headTail_.m_reader = 0;
	headTail_.m_freeMem = 0;
	headTail_.m_memSize = 0;

}

fileMapping::~fileMapping()
{

}

bool fileMapping::openFileMap()
{

	hMapFile_ = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		BUFFER_SIZE,
		globName);


	if (hMapFile_ == NULL)
	{
		//	MessageBox(hWnd, L"Could not open the filemap!", L"Filler Text", MB_OK);
		return false;
	}


	pBuf_ = (void*)MapViewOfFile(hMapFile_, FILE_MAP_ALL_ACCESS, 0, 0, (BUFFER_SIZE));


	if (pBuf_ == NULL)
	{
		//	MessageBox(hWnd, L"Could not create view of the filemap!", L"Filler Text", MB_OK);
		return false;
		CloseHandle(hMapFile_);
		return false;
	}


	bufferController_ = CreateFileMapping(
		INVALID_HANDLE_VALUE,
		NULL,
		PAGE_READWRITE,
		0,
		sizeof(HeadTail),
		globName2);


	if (bufferController_ == NULL)
	{
		//	MessageBox(hWnd, L"Could not open the controller filemap!", L"Filler Text", MB_OK);
		return false;
	}


	controlBuf_ = MapViewOfFile(bufferController_, FILE_MAP_ALL_ACCESS, 0, 0, sizeof(HeadTail));

	if (controlBuf_ == NULL)
	{
		//	MessageBox(hWnd, L"Could not open the view of the controller filemap!", L"Filler Text", MB_OK);
		CloseHandle(bufferController_);
		return 1;
	}


	//Control Buffer
	headTail_.m_head = (unsigned int*)controlBuf_;
	headTail_.m_tail = headTail_.m_head + 1;
	headTail_.m_reader = headTail_.m_head + 2;
	headTail_.m_freeMem = headTail_.m_head + 3;
	headTail_.m_memSize = headTail_.m_head + 4;



	return true;
}

bool fileMapping::closeFileMap()
{
	CloseHandle(hMapFile_);
	return true;
}

void* fileMapping::returnPbuf()
{
	return pBuf_;
}

void* fileMapping::returnControlbuf()
{
	return controlBuf_;
}


void fileMapping::getControlBufferContent(int &head, int &tail, int &reader, int &freeSpace, int &memSize)
{

	head = *headTail_.m_head;
	tail = *headTail_.m_tail;
	reader = *headTail_.m_reader;
	freeSpace = *headTail_.m_freeMem;
	memSize = *headTail_.m_memSize;

}