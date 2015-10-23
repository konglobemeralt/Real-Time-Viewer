#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>

#include "textureClass.h"


using namespace std;

using DirectX::XMFLOAT2;
using DirectX::XMFLOAT3;
using DirectX::XMFLOAT4;



class ModelClass
{
public:
	struct VertexLoadStruct
	{
		float x, y, z;
	};

	

	struct VertexType
	{
		XMFLOAT4 position;
		XMFLOAT2 texture;
		XMFLOAT3 normal;
	};

	struct ModelType
	{
		float x, y, z;
		float tu, tv;
		float nx, ny, nz;
	};

	ModelClass();
	~ModelClass();

	bool Initialize(ID3D11Device*, WCHAR*, void* cBuf, void* pBuf);
	void Shutdown();
	void Render(ID3D11DeviceContext*);


	bool ModelClass::UpdateBuffers(ID3D11Device* pDevice, void* cBuf, void* pBuf, WCHAR*);

	int GetIndexCount();

	ID3D11ShaderResourceView* GetTexture();

	

	void SetPosition(float, float, float);
	void GetPosition(float&, float&, float&);
	void setIndexCount(int i);

	int getModelID();

	DirectX::XMMATRIX getWorldMatrix();

	XMFLOAT4 getDiffuse();
	void setWorldMatrix(DirectX::XMFLOAT4X4& tempMatrix);

private:
	bool InitializeBuffers(ID3D11Device*, void* cBuf, void* pBuf);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);

	bool LoadTexture(ID3D11Device*, WCHAR*);
	void ReleaseTexture();

	


	

	void ReleaseModel();

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	TextureClass* m_pTexture;
	ModelType* m_pModel;

	float m_positionX, m_positionY, m_positionZ;

	int m_vertexCount;
	int m_indexCount;
	int m_modelID;

	DirectX::XMMATRIX m_worldMatrix;
	

	//material values
	bool m_HasMaterial;
	float m_diffuseR, m_diffuseG, m_diffuseB;
	char m_diffuseTexturePath[50];


};



#endif