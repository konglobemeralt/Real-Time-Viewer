#include "lightclass.h"


LightClass::LightClass()
{
}



LightClass::~LightClass()
{
}

void LightClass::SetDiffuseColor(float red, float green, float blue, float alpha)
{
	m_diffuseColor = XMFLOAT4(red, green, blue, alpha);
	return;
}


void LightClass::SetDirection(float x, float y, float z)
{
	m_direction = XMFLOAT3(x, y, z);
	return;
}


XMFLOAT4 LightClass::GetDiffuseColor()
{
	return m_diffuseColor;
}


XMFLOAT3 LightClass::GetDirection()
{
	return m_direction;
}


void LightClass::Render(void* cBuf, void* pBuf)
{
	LightData lightData;


	int messageType = -1;

	int usedSpace = 0;

	unsigned int *headP = (unsigned int*)cBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;










	memcpy(&lightData.position, (char*)pBuf + *tailP + sizeof(int) + sizeof(int) + sizeof(int), sizeof(XMFLOAT4));
	memcpy(&lightData.color, (char*)pBuf + *tailP + sizeof(int) + sizeof(int) + sizeof(int) + sizeof(XMFLOAT4), sizeof(XMFLOAT4));


	/*memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int), &translation, sizeof(XMFLOAT4));
	memcpy((char*)pBuf + usedSpace + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(XMFLOAT4), &color, sizeof(XMFLOAT4));*/


	unsigned int tempH = *headP;

	if (*tailP < *memSize) // read == *readerAmount) &&
	{
		*tailP += 1000;
		//*readP = 1;
	}
	if (*tailP >= *memSize) //(read == *readerAmount) &&
	{
		*tailP = 0;
	}


	///NOT SO USELESS///
	//m_viewMatrix = XMLoadFloat4x4(&cameraData.camViewMatrix);

	//XMStoreFloat3(&m_position, lightData.position);
	SetDiffuseColor(lightData.color.x, lightData.color.y, lightData.color.z, lightData.color.w);
	SetPosition(lightData.position.x, lightData.position.y, lightData.position.z, lightData.position.w);




	return;
}

void LightClass::SetPosition(float x, float y, float z, float w)
{
	m_position.x = x;
	m_position.y = y;
	m_position.z = z;
	m_position.w = w;
	return;
}

XMFLOAT4 LightClass::GetPosition()
{
	return m_position;
}