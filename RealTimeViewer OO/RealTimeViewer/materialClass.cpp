#include "materialClass.h"


MaterialClass::MaterialClass()
{
	m_matColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_matSpecColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_matReflectivity = 1.0f;
	m_matSpecRolloff = 1.0f;

	m_diffuseR = 1.0f;
	m_diffuseG = 1.0f;
	m_diffuseB = 1.0f;

	m_texturePath = (WCHAR *)malloc(sizeof(WCHAR)* (500 + 1));
	m_texturePath = L"missy.dds\0";
	
}

MaterialClass::~MaterialClass()
{
	
	//ReleaseTexture();
}


bool MaterialClass::updateMaterial(void* cBuf, void* pBuf)
{



	//Control Buffer
	unsigned int* headPtr = (unsigned int*)cBuf;
	unsigned int* tailPtr = headPtr + 1;
	unsigned int* readPtr = headPtr + 2;
	unsigned int* freeMemPtr = headPtr + 3;
	unsigned int* memSizePtr = headPtr + 4;


	int usedSpace = 0;
	
	memcpy(&m_matColor, (char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int), sizeof(DirectX::XMFLOAT4));
//memcpy(&m_matSpecColor, (char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4), sizeof(DirectX::XMFLOAT4));
//memcpy(&m_matReflectivity, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), sizeof(float));
//memcpy(&m_matSpecRolloff, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(float), sizeof(float));

	int textureChech = 0;
	memcpy(&textureChech, (char*)pBuf + sizeof(int)+sizeof(int), sizeof(int));
	
	//if (textureChech == 1)
	//{
	//	ReleaseTexture();
	//	int TextPathSize = 0;
	//	memcpy(&TextPathSize, (char*)pBuf +sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4), sizeof(int));
	//	//delete[]m_texturePath;
		
	//char* tempText = (char *)malloc(sizeof(char)* (TextPathSize+1));
	//memcpy(&tempText, (char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4), sizeof(char)*(TextPathSize + 1));
	//	m_texturePath = L"alpha.dds";
	//	m_diffuseTexturePath = L"alpha.dds";
	//}
	
	unsigned int tempH = *headPtr;

	if (*tailPtr < *memSizePtr) // read == *readerAmount) &&
	{
		*tailPtr += 10000;
		*readPtr = 1;
	}
	if (*tailPtr >= *memSizePtr) //(read == *readerAmount) &&
	{
		*tailPtr = 0;
	}

	return true;
}

DirectX::XMFLOAT4 MaterialClass::getMatColor()
{
	return m_matColor;
}

DirectX::XMFLOAT4 MaterialClass::getMatSpecColor()
{
	return m_matSpecColor;
}

float MaterialClass::getMatReflectivity()
{
	return m_matReflectivity;
}

float MaterialClass::getMatSpecRolloff()
{
	return m_matSpecRolloff;
}

WCHAR* MaterialClass::getTexturePath()
{
	return m_texturePath;
}



bool MaterialClass::LoadTexture(ID3D11Device* pDevice)
{
	bool result;


	// Create the texture object.
	m_pTexture = new TextureClass;
	if (!m_pTexture)
	{
		return false;
	}



	// Initialize the texture object.
	result = m_pTexture->Initialize(pDevice, m_texturePath);
	if (!result)
	{
		return false;
	}




	return true;
}

ID3D11ShaderResourceView* MaterialClass::GetTexture()
{
	return m_pTexture->GetTexture();
}


void MaterialClass::ReleaseTexture()
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