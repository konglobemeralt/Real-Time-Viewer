#include "cameraclass.h"
#include <windows.h>

CameraClass::CameraClass()
{
	m_positionX = 0.0f;
	m_positionY = 0.0f;
	m_positionZ = 0.0f;

	m_rotationX = 0.0f;
	m_rotationY = 0.0f;
	m_rotationZ = 0.0f;

	//m_viewMatrix = DirectX::XMMatrixIdentity();

}



CameraClass::~CameraClass()
{
}


void CameraClass::SetPosition(float x, float y, float z)
{
	m_positionX = x;
	m_positionY = y;
	m_positionZ = z;
	return;
}


void CameraClass::SetRotation(float x, float y, float z)
{
	m_rotationX = x;
	m_rotationY = y;
	m_rotationZ = z;
	return;
}


XMFLOAT3 CameraClass::GetPosition()
{
	return XMFLOAT3(m_positionX, m_positionY, m_positionZ);
}


XMFLOAT3 CameraClass::GetRotation()
{
	return XMFLOAT3(m_rotationX, m_rotationY, m_rotationZ);
}


void CameraClass::Render(void* cBuf, void* pBuf)
{
	CameraData cameraData;


	int messageType = -1;

	int usedSpace = 0;

	unsigned int *headP = (unsigned int*)cBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;


	if (*tailP != *headP)
	{



		memcpy(&messageType, (char*)pBuf + usedSpace, sizeof(int));


		if (messageType == 1)
		{

			memcpy(&cameraData.camViewMatrix, (char*)pBuf + (sizeof(int)), sizeof(XMFLOAT4X4));




			unsigned int tempH = *headP;

			if (*tailP < *memSize) // read == *readerAmount) &&
			{
				*tailP += 10000;
				//*readP = 1;
			}
			if (*tailP >= *memSize) //(read == *readerAmount) &&
			{
				*tailP = 0;
			}


			///USELESS///

			XMVECTOR up, position, lookAt;
			float yaw, pitch, roll;
			XMMATRIX rotationMatrix;

			//Set the default up vector.
			up.m128_f32[0] = 0.0f;
			up.m128_f32[1] = 1.0f;
			up.m128_f32[2] = 0.0f;
			up.m128_f32[3] = 1.0f;

			//Set the default world position.
			position.m128_f32[0] = 0.0f;
			position.m128_f32[1] = 0.0f;
			position.m128_f32[2] = 0.0f;
			position.m128_f32[3] = 1.0f;

			//Set where the camera is looking by default.
			lookAt.m128_f32[0] = 0.0f;
			lookAt.m128_f32[1] = 0.0f;
			lookAt.m128_f32[2] = -1.0f;
			lookAt.m128_f32[3] = 0.0f;

			//Set the yaw, pitch, and roll in radians´.
			pitch = m_rotationX * 0.0174532925f;
			yaw = m_rotationY * 0.0174532925f;
			roll = m_rotationZ * 0.0174532925f;

			//Create the rotation matrix from the above values.
			rotationMatrix = XMMatrixRotationRollPitchYaw(pitch, yaw, roll);

			//Transfoorm the lookAt and upp vector by the rotation matrix.
			lookAt = XMVector3TransformCoord(lookAt, rotationMatrix);
			up = XMVector3TransformCoord(up, rotationMatrix);




			// Translate the rotated camera position to the location of the viewer.
			lookAt = position + lookAt;


			//Create the view matrix from the updated vectors.
			//m_viewMatrix = XMMatrixLookAtRH(position, lookAt, up);


			///NOT SO USELESS///

			m_viewMatrix = XMLoadFloat4x4(&cameraData.camViewMatrix);

		}

	}

	return;
}


void CameraClass::GetViewMatrix(XMMATRIX& viewMatrix)
{
	viewMatrix = m_viewMatrix;
	return;
}

