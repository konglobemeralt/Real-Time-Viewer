#include "materialClass.h"
#include <string>

MaterialClass::MaterialClass()
{
	m_matColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_matSpecColor = DirectX::XMFLOAT4(0.5f, 0.5f, 0.5f, 1.0f);
	m_matReflectivity = 1.0f;
	m_matSpecRolloff = 1.0f;

	m_diffuseR = 1.0f;
	m_diffuseG = 1.0f;
	m_diffuseB = 1.0f;

	//m_texturePath = (WCHAR *)malloc(sizeof(WCHAR)* (500 + 1));
	//m_texturePath = L"missy.dds\0";
	wcscpy(m_texturePath, L"missy.dds\0");
	
}

MaterialClass::~MaterialClass()
{
	
	//ReleaseTexture();
}


bool MaterialClass::updateMaterial(void* cBuf, void* pBuf)
{


	int usedSpace = 0;
	
	memcpy(&m_matColor, (char*)pBuf + sizeof(int) + sizeof(int) + sizeof(int), sizeof(DirectX::XMFLOAT4));
	//memcpy(&m_matSpecColor, (char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4), sizeof(DirectX::XMFLOAT4));
	//memcpy(&m_matReflectivity, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), sizeof(float));
	//memcpy(&m_matSpecRolloff, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(float), sizeof(float));

	//delete[]m_texturePath;
	int texExist;
	memcpy(&texExist, (char*)pBuf + sizeof(int) + sizeof(int), sizeof(int));
	


	
	if (texExist == 1)
	{
		
		////ReleaseTexture();
		////delete[]m_texturePath;
		//m_texturePath = (WCHAR *)malloc(sizeof(WCHAR)* (500 + 1));
		////int TextPathSize = 0;
		////memcpy(m_texturePath, L"frontdesk.dds\0", 150);
		//
		//m_texturePath = L"frontdesk.dds\0";
		//m_diffuseTexturePath = L"frontdesk.dds\0";

		m_matColor = DirectX::XMFLOAT4(1.0f, 1.0f, 1.0f, 1.0f);
	
	


		memcpy(&tempPath, (char*)pBuf + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), (sizeof(char) * 500));
		
		std::string test = tempPath;
		std::size_t found = test.find_last_of("/\\");
		std::string test2 = test.substr(found+1);

		std::wstring widestr = std::wstring(test2.begin(), test2.end());
		
		WCHAR* jollygoodString = (WCHAR*)widestr.c_str();
		
		wcscpy(m_texturePath, jollygoodString);
		//	mbscpy()
		//m_texturePath = jollygoodString;
		

		int sg = 0;
		
		return true;
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

	WCHAR* temp = m_texturePath;

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


