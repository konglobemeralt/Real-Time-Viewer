#ifndef FILEMAPPING_H
#define FILEMAPPING_H

#include <tchar.h>
#include <windows.h>
#include <DirectXMath.h>
#include <vector>


class fileMapping
{
public:

	//HeadTail
	fileMapping();
	fileMapping(const fileMapping&);
	~fileMapping();

	//Message not to date, dont use it
	struct VertexData
	{
		int id;
		DirectX::XMFLOAT4 pos;
		DirectX::XMFLOAT2 uv;
		DirectX::XMFLOAT3 norms;
	};
	VertexData *vertData;

	struct MatrixData
	{
		DirectX::XMMATRIX position;
		DirectX::XMMATRIX rotation;
		DirectX::XMMATRIX scale;
	};
	MatrixData matrices;

	struct CameraData
	{
		DirectX::XMFLOAT3 position;
		DirectX::XMFLOAT3 rotation;
		DirectX::XMFLOAT3 viewDirection;
	}; CameraData cameraData;

	struct message
	{
		//Header
		int messageType;
		int messageSize;
		int padding;
		//actual message
		//meshes

		//camera
		CameraData camData;

		//mesh transforms
		DirectX::XMFLOAT4X4 matrixData;

		//mesh
		int numMeshes;
		int numVerts;
		std::vector<VertexData> vert;

	};




	struct HeadTail
	{
		unsigned int *m_head;
		unsigned int *m_tail;
		unsigned int *m_reader;
		unsigned int *m_freeMem;
		unsigned int *m_memSize;
	};




	bool openFileMap();
	bool closeFileMap();
	void* returnPbuf();
	void* returnControlbuf();

	void getControlBufferContent(int&, int&, int&, int&, int&);


private:
	HeadTail headTail_;

	HANDLE hMapFile_;




	void* pBuf_;
	void* controlBuf_;

	//readWriteControlls
	HANDLE bufferController_;


};


#endif