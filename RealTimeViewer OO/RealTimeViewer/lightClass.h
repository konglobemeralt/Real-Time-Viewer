#ifndef _LIGHTCLASS_H_
#define _LIGHTCLASS_H_

#include <DirectXMath.h>
#include <Windows.h>
using namespace DirectX;

struct LightData
{
	XMFLOAT4 position;
	XMFLOAT4 color;
};

class LightClass
{
public:
	LightClass();
	~LightClass();

	
	void SetDiffuseColor(float, float, float, float);
	void SetDirection(float, float, float);

	void SetPosition(float, float, float, float);
	
	void Render(void* cBuf, void* pBuf);
	

	XMFLOAT4 GetDiffuseColor();
	XMFLOAT3 GetDirection();
	XMFLOAT4 GetPosition();
	

private:

	XMFLOAT4 m_diffuseColor;
	
	XMFLOAT3 m_direction;

	XMFLOAT4 m_position;
	


};

#endif