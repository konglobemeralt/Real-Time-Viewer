#ifndef _CAMERACLASS_H_
#define _CAMERACLASS_H_


#include <DirectXMath.h>


using namespace DirectX;

struct CameraData
{
	XMFLOAT4X4 camViewMatrix;
};

class CameraClass
{
public:
	CameraClass();
	~CameraClass();

	void SetPosition(float, float, float);
	void SetRotation(float, float, float);

	XMFLOAT3 GetPosition();
	XMFLOAT3 GetRotation();

	void Render(void* cBuf, void* pBuf);
	void GetViewMatrix(XMMATRIX&);

private:

	float m_positionX, m_positionY, m_positionZ;
	float m_rotationX, m_rotationY, m_rotationZ;

	XMMATRIX m_viewMatrix;


};

#endif