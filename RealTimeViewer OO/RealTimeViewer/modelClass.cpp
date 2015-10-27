#ifdef _MSC_VER
#define _CRT_SECURE_NO_WARNINGS
#endif


#include <stdint.h>
#include <stdio.h>
#include <wchar.h>
#include "ModelClass.h"
#include "textureClass.h"


//Temp placement
struct ObjectHeader
{
	int vertexAmount;
};

struct CameraData
{
	DirectX::XMFLOAT4X4 camMatrix;
};

ModelClass::ModelClass()
: m_pVertexBuffer(nullptr)
, m_pIndexBuffer(nullptr)
, m_pModel(nullptr)
, m_vertexCount(0)
, m_indexCount(0)
, matID(0)
{}

ModelClass::~ModelClass()
{
}

bool ModelClass::Initialize(ID3D11Device* pDevice, WCHAR* textureFilename, void* cBuf, void* pBuf)
{
	bool result;

	

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(pDevice, cBuf, pBuf);
	if (!result)
	{
		return false;
	}

	//Load the texture for this model.
	result = LoadTexture(pDevice, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;
}

bool ModelClass::UpdateBuffers(ID3D11Device* pDevice, void* cBuf, void* pBuf, WCHAR* textureFilename)
{
	bool result;

	


	//Shutdown();

	// Initialize the vertex and index buffers.
	result = InitializeBuffers(pDevice, cBuf, pBuf);
	if (!result)
	{
		return false;
	}

	return true;

	//Load the texture for this model.
	result = LoadTexture(pDevice, textureFilename);
	if (!result)
	{
		return false;
	}

	return true;


}



void ModelClass::Shutdown()
{
	// Release the model texture.
	ReleaseTexture();

	// Shutdown the vertex and index buffers.
	ShutdownBuffers();

	// Release the model data.
	ReleaseModel();

	return;
}


void ModelClass::ReleaseTexture()
{
	// Release the texture object.
	if (m_pTexture)
	{
		m_pTexture->Shutdown();
		delete m_pTexture;
		m_pTexture = 0;
	}

	return;
}

void ModelClass::Render(ID3D11DeviceContext* pContext)
{
	RenderBuffers(pContext);
}

int ModelClass::GetIndexCount()
{
	return m_indexCount;

}


bool ModelClass::InitializeBuffers(ID3D11Device* device, void* cBuf, void* pBuf)
{



	//Control Buffer
	unsigned int* headPtr = (unsigned int*)cBuf;
	unsigned int* tailPtr = headPtr + 1;
	unsigned int* readPtr = headPtr + 2;
	unsigned int* freeMemPtr = headPtr + 3;
	unsigned int* memSizePtr = headPtr + 4;
		
	



	VertexType* vertices;
	unsigned long* indices;
	D3D11_BUFFER_DESC vertexBufferDesc, indexBufferDesc;
	D3D11_SUBRESOURCE_DATA vertexData, indexData;
	HRESULT result;
	int i;

	
	

	

		// Create the vertex array.

		int usedSpace = 0;
		//Read Header
		int messageType = -1;



		

		

		//ModelID
		memcpy(&m_modelID, (char*)pBuf + usedSpace + (sizeof(int)* 4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));
		//vertCount
		memcpy(&m_vertexCount, (char*)pBuf + usedSpace + (sizeof(int)* 4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));

			//memcpy(&m_vertexCount, (char*)pBuf + (sizeof(int) *4), sizeof(int));


			vertices = new VertexType[m_vertexCount];
			if (!vertices)
			{
				return false;
			}


			m_indexCount = m_vertexCount;
			// Create the index array.
			indices = new unsigned long[m_indexCount];
			if (!indices)
			{
				return false;
			}

			
			struct VertexData
			{
				XMFLOAT4 pos;
				XMFLOAT2 uv;
				XMFLOAT3 norms;
			};

			char* tempBuf = (char*)pBuf;

			memcpy(&m_worldMatrix, (char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + (sizeof(DirectX::XMFLOAT4X4)), sizeof(DirectX::XMFLOAT4X4));

			tempBuf += (sizeof(int)* 5) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float);

			// Load the vertex array and index array with data.
			for (i = 0; i < m_vertexCount; i++)
			{
				indices[i] = i;
				memcpy(&vertices[i].position, (char*)tempBuf  + (sizeof(VertexData)*i), sizeof(XMFLOAT4));
				memcpy(&vertices[i].texture, (char*)tempBuf  + (sizeof(VertexData)*i) + sizeof(XMFLOAT4), sizeof(XMFLOAT2));
				memcpy(&vertices[i].normal, (char*)tempBuf  + (sizeof(VertexData)*i) + sizeof(XMFLOAT4)+sizeof(XMFLOAT2), sizeof(XMFLOAT3));

				//usedSpace += sizeof(XMFLOAT4)+sizeof(XMFLOAT2)+sizeof(XMFLOAT3);

			}


			

			unsigned int tempH = *headPtr;

			if (*tailPtr < *memSizePtr) // read == *readerAmount) &&
			{
				*tailPtr += 100000;
				*readPtr = 1;
			}
			if (*tailPtr >= *memSizePtr) //(read == *readerAmount) &&
			{
				*tailPtr = 0;
			}



			//	fileMap.closeFileMap();
		//	m_vertexCount = 3;
			// Load the vertex array with data.
		//	vertices[0].position = XMFLOAT4(-1.0f, -1.0f, 0.0f, 0.0f);  // Bottom left.
		//	vertices[0].texture = XMFLOAT2(0.0f, 1.0f);
		//
		//	vertices[1].position = XMFLOAT4(0.0f, 1.0f, 0.0f, 0.0f);  // Top middle.
		//	vertices[1].texture = XMFLOAT2(0.5f, 0.0f);
		//
		//	vertices[2].position = XMFLOAT4(1.0f, -1.0f, 0.0f, 0.0f);  // Bottom right.
		//	vertices[2].texture = XMFLOAT2(1.0f, 1.0f);
			
			// Set up the description of the static vertex buffer.
			// vertex Buffer
			vertexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			vertexBufferDesc.ByteWidth = sizeof(VertexType)* m_vertexCount;
			vertexBufferDesc.BindFlags = D3D11_BIND_VERTEX_BUFFER;
			vertexBufferDesc.CPUAccessFlags = 0;
			vertexBufferDesc.MiscFlags = 0;
			vertexBufferDesc.StructureByteStride = 0;

			// Give the subresource structure a pointer to the vertex data.
			vertexData.pSysMem = vertices;
			vertexData.SysMemPitch = 0;
			vertexData.SysMemSlicePitch = 0;

			// Now create the vertex buffer.
			result = device->CreateBuffer(&vertexBufferDesc, &vertexData, &m_pVertexBuffer);
			if (FAILED(result))
			{
				return false;
			}

			// Set up the description of the static index buffer.
			indexBufferDesc.Usage = D3D11_USAGE_DEFAULT;
			indexBufferDesc.ByteWidth = sizeof(unsigned long)* m_indexCount;
			indexBufferDesc.BindFlags = D3D11_BIND_INDEX_BUFFER;
			indexBufferDesc.CPUAccessFlags = 0;
			indexBufferDesc.MiscFlags = 0;
			indexBufferDesc.StructureByteStride = 0;

			// Give the subresource structure a pointer to the index data.
			indexData.pSysMem = indices;
			indexData.SysMemPitch = 0;
			indexData.SysMemSlicePitch = 0;

			// Create the index buffer.
			result = device->CreateBuffer(&indexBufferDesc, &indexData, &m_pIndexBuffer);
			if (FAILED(result))
			{
				return false;
			}

			// Release the arrays now that the vertex and index buffers have been created and loaded.
			delete[] vertices;
			vertices = 0;

			delete[] indices;
			indices = 0;

			//updateMaterial(cBuf, pBuf);

		//MOVE HEADER TAIL STUFF TO RTV update:

	//	if (messageType == 2)
	//	{
	//
	//
	//		memcpy(&m_worldMatrix, (char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + (sizeof(DirectX::XMFLOAT4X4)), sizeof(DirectX::XMFLOAT4X4));
	//
	//
	//		unsigned int tempH = *headPtr;
	//
	//		if (*tailPtr < *memSizePtr) // read == *readerAmount) &&
	//		{
	//			*tailPtr += 100000;
	//			*readPtr = 1;
	//		}
	//		if (*tailPtr >= *memSizePtr) //(read == *readerAmount) &&
	//		{
	//			*tailPtr = 0;
	//		}
	//	}


	

	return true;
}


void ModelClass::ShutdownBuffers()
{
	// Release the index buffer.
	if (m_pIndexBuffer)
	{
		m_pIndexBuffer->Release();
		m_pIndexBuffer = 0;
	}

	// Release the vertex buffer.
	if (m_pVertexBuffer)
	{
		m_pVertexBuffer->Release();
		m_pVertexBuffer = 0;
	}

	return;
}

void ModelClass::RenderBuffers(ID3D11DeviceContext* deviceContext)
{
	unsigned int stride;
	unsigned int offset;


	// Set vertex buffer stride and offset.
	stride = sizeof(VertexType);
	offset = 0;

	// Set the vertex buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetVertexBuffers(0, 1, &m_pVertexBuffer, &stride, &offset);

	// Set the index buffer to active in the input assembler so it can be rendered.
	deviceContext->IASetIndexBuffer(m_pIndexBuffer, DXGI_FORMAT_R32_UINT, 0);

	// Set the type of primitive that should be rendered from this vertex buffer, in this case triangles.
	deviceContext->IASetPrimitiveTopology(D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);

	return;
}



void ModelClass::ReleaseModel()
{
	if (m_pModel)
	{
		delete[] m_pModel;
		m_pModel = 0;
	}

	return;
}

void ModelClass::setIndexCount(int i)
{
	m_indexCount = i;

}

void ModelClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void ModelClass::GetPosition(float& x, float& y, float& z)
{
	x = m_positionX;
	y = m_positionY;
	z = m_positionZ;
	return;
}



bool ModelClass::LoadTexture(ID3D11Device* pDevice, WCHAR* filename)
{
	bool result;


	// Create the texture object.
	m_pTexture = new TextureClass;
	if (!m_pTexture)
	{
		return false;
	}



	// Initialize the texture object.
	result = m_pTexture->Initialize(pDevice, filename);
	if (!result)
	{
		return false;
	}




	return true;
}




ID3D11ShaderResourceView* ModelClass::GetTexture()
{
	return m_pTexture->GetTexture();
}


DirectX::XMMATRIX ModelClass::getWorldMatrix()
{
	return m_worldMatrix;

}

int ModelClass::getModelID()
{
	return m_modelID;
}

void ModelClass::setWorldMatrix(DirectX::XMFLOAT4X4& tempMatrix)
{

	m_worldMatrix = XMLoadFloat4x4(&tempMatrix);

}

int ModelClass::getMatID()
{

	return matID;

}

void ModelClass::setMaterialID(int ID)
{
	matID = ID;

}