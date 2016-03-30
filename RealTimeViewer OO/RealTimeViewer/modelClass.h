#ifndef MODEL_CLASS_H
#define MODEL_CLASS_H

#include <d3d11.h>
#include <DirectXMath.h>
#include <fstream>



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

	bool Initialize(ID3D11Device*, void* cBuf, void* pBuf);
	void Shutdown();
	void Render(ID3D11DeviceContext*);


	bool UpdateBuffers(ID3D11Device* pDevice, void* cBuf, void*);

	//bool updateMaterial(void* cBuf, void* pBuf);

	int GetIndexCount();



	void SetPosition(float, float, float);
	void GetPosition(float&, float&, float&);
	void setIndexCount(int i);

	void setMaterialID(int ID);
	int getMatID();
	int getModelID();

	DirectX::XMMATRIX getWorldMatrix();

	XMFLOAT4 getDiffuse();
	void setWorldMatrix(DirectX::XMFLOAT4X4& tempMatrix);

private:
	bool InitializeBuffers(ID3D11Device*, void* cBuf, void* pBuf);
	void ShutdownBuffers();
	void RenderBuffers(ID3D11DeviceContext*);



	void ReleaseModel();

	ID3D11Buffer* m_pVertexBuffer;
	ID3D11Buffer* m_pIndexBuffer;

	ModelType* m_pModel;

	float m_positionX, m_positionY, m_positionZ;

	int m_vertexCount;
	int m_indexCount;
	int m_modelID;

	int matID;

	DirectX::XMMATRIX m_worldMatrix;





};



#endif