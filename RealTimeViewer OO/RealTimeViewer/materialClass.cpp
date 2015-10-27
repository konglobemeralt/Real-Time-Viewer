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

}

MaterialClass::~MaterialClass()
{
	
	
}


bool MaterialClass::updateMaterial(void* cBuf, void* pBuf)
{


	int usedSpace = 0;
	//finding material values
	memcpy(&m_matColor, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4), sizeof(DirectX::XMFLOAT4));
	memcpy(&m_matSpecColor, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4), sizeof(DirectX::XMFLOAT4));
	memcpy(&m_matReflectivity, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4), sizeof(float));
	memcpy(&m_matSpecRolloff, (char*)pBuf + usedSpace + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(float), sizeof(float));

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