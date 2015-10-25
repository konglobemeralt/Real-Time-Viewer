#include "realTimeViewer.h"
#include "modelClass.h"
#include "ShaderShader.h"
#include <new>

realTimeViewer::realTimeViewer()
{

	m_Direct3D = 0;
	m_Camera = 0;
	//m_model = 0;
	m_Light = 0;
	m_ShaderShader = 0;

}

realTimeViewer::~realTimeViewer(){};


bool realTimeViewer::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;
	
	char videoCard[128];
	int videoMemory;
	
	//Create the fileMapping object
	m_fileMap = new fileMapping;
	if (!m_fileMap)
	{
		return false;
	}
	else
	{
		m_fileMap->openFileMap();
	}

	
	
	//Create the Direct3D object.
	m_Direct3D = (D3DClass*)_aligned_malloc(sizeof(D3DClass), 16);
	new (m_Direct3D)D3DClass();
	if (!m_Direct3D)
	{
		return false;
	}

	//Initialize the Direct3D object with the given values
	result = m_Direct3D->Initialize(screenWidth, screenHeight, VSYNC_ENABLED, hwnd, FULL_SCREEN, SCREEN_DEPTH, SCREEN_NEAR);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize DirectX 11.", L"Error", MB_OK);
		return false;
	}



	//Create the camera object.
	m_Camera = (CameraClass*)_aligned_malloc(sizeof(CameraClass), 16);
	new (m_Camera)CameraClass();
	if (!m_Camera)
	{
		return false;
	}

	
	//Create the light object.
	m_Light = new LightClass;
	if (!m_Light)
	{
		return false;
	}

	//Initialize the light object.
	m_Light->SetDiffuseColor(1.0f, 1.0f, 1.0f, 1.0f);
	m_Light->SetDirection(0.0f, 0.0f, 1.0f);
	


// //Create a modelObject!
// m_model = new ModelClass(); 
// result = m_model->Initialize(m_Direct3D->GetDevice(), L"missy2.dds", m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());
// 
// if (result == false)
// {
// 	MessageBox(hwnd, L"Could not initialize model.", L"ERROR", MB_OK);
// 	//return false;
// 	return true;
// }
	

	//create shader object
	m_ShaderShader = new ShaderShaderClass;
	if (!m_ShaderShader)
	{
		return false;
	}

	//initialize the depth shader object.
	result = m_ShaderShader->Initialize(m_Direct3D->GetDevice(), hwnd);
	if (!result)
	{
		MessageBox(hwnd, L"Could not initialize the shader object.", L"Error", MB_OK);
		return false;
	}


	return true;
}


void realTimeViewer::Shutdown()
{

	//Release the depth shader object.
	if (m_ShaderShader)
	{
		m_ShaderShader->Shutdown();
		//delete m_ShaderShader;
		m_ShaderShader = 0;
	}

	//Release the camera object.
	if (m_Camera)
	{
		m_Camera->~CameraClass();
		_aligned_free(m_Camera);
		m_Camera = 0;
	}

	//Release the camera object.
	if (m_Light)
	{
		m_Light->~LightClass();
		_aligned_free(m_Light);
		m_Light = 0;
	}

//	//Release the camera object.
//	if (m_model)
//	{
//		m_model->~ModelClass();
//		_aligned_free(m_model);
//		m_model = 0;
//	}


	//Release the Direct3D object.
	if (m_Direct3D)
	{
		m_Direct3D->Shutdown();
		m_Direct3D->~D3DClass();
		_aligned_free(m_Direct3D);
		m_Direct3D = 0;
	}

	if (m_fileMap)
	{
		m_fileMap->closeFileMap();
	}


	return;
}


bool realTimeViewer::frame()
{
	bool result, foundHeight;
	XMFLOAT3 position;
	float height;
	float radians;



	//Render the graphics.
	result = RenderGraphics();
	if (!result)
	{
		return false;
	}

	return result;
}


bool realTimeViewer::RenderGraphics()
{
	bool result = false;
	XMMATRIX worldMatrix, viewMatrix, projectionMatrix, orthoMatrix;

	int modelCount, renderCount, index;
	float positionX, positionY, positionZ, radius;
	XMFLOAT4 color;
	


	// Clear the scene.
	m_Direct3D->BeginScene(0.4f, 0.2f, 0.8f, 1.0f);

	// Generate the view matrix based on the camera's position.
	m_Camera->Render(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());
	
	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);
	m_Light->Render(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());

	//update model if necessary
	for (int i = 0; i < modelVector.size(); i++)
	{

		if (modelVector.at(i).GetIndexCount() > 0)
		{
		
		
		modelVector.at(i).Render(m_Direct3D->GetDeviceContext());
			// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.

			
			//Move the model to the location it should be rendered at.
		worldMatrix = modelVector.at(i).getWorldMatrix();
	
	
		
			//Put the model vertex and index buffers on the graphics pipeline to prepare for drawing.
	
			result = m_ShaderShader->Render(m_Direct3D->GetDeviceContext(),
				modelVector.at(i).GetIndexCount(),
				worldMatrix,
				viewMatrix,
				projectionMatrix,
				modelVector.at(i).GetTexture(),
				modelVector.at(i).getMatColor(), 
				modelVector.at(i).getMatSpecColor(), 
				modelVector.at(i).getMatReflectivity(), 
				modelVector.at(i).getMatSpecRolloff(),
				m_Light->GetDirection(), 
				m_Light->GetDiffuseColor());

			if (!result)
			{
				return false;
			}

	}
	}
	
	update();


	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();



	

	return true;
}

void realTimeViewer::update()
{


	int messageType = -1;
	void* pBuf = m_fileMap->returnPbuf();
	void* cBuf = m_fileMap->returnControlbuf();

	unsigned int *headP = (unsigned int*)cBuf;
	unsigned int *tailP = headP + 1;
	unsigned int *readerAmount = headP + 2;
	unsigned int *freeMem = headP + 3;
	unsigned int *memSize = headP + 4;

	if (*tailP != *headP)
	{

		memcpy(&messageType, (char*)pBuf, sizeof(int));




		int modelID = -1;


		



		//Create mesh
		if (messageType == 0)
		{

			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));


			if (modelID > modelVector.size())
			{
				m_model.Initialize(m_Direct3D->GetDevice(), L"missy.dds", m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());
				modelVector.push_back(m_model);

			}

			modelID = -1;

		}

		//Transform mesh
		if (messageType == 2)
		{
			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));

			XMFLOAT4X4 tempMatrix;
			memcpy(&tempMatrix, (char*)pBuf + sizeof(int)+sizeof(int)+sizeof(int)+sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + (sizeof(DirectX::XMFLOAT4X4)), sizeof(DirectX::XMFLOAT4X4));


			if (modelID > 0)
				modelVector.at(modelID - 1).setWorldMatrix(tempMatrix);

			else
				modelVector.at(modelID).setWorldMatrix(tempMatrix);


		}


		//Transform material
		if (messageType == 4)
		{
			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));

			
						
			if (modelID > 0)
				modelVector.at(modelID - 1).updateMaterial(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());

			else
				modelVector.at(modelID).updateMaterial(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());

			


		}




		//Delete Mesh
		if (messageType == 5)
		{
			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));


			if (modelID > 0)
				modelVector.at(modelID - 1).setIndexCount(-1);

			else
				modelVector.at(modelID).setIndexCount(-1);


		}

		//vertex changed mesh
		if (messageType == 6)
		{
			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));


			if (modelID > 0)
				modelVector.at(modelID - 1).UpdateBuffers(m_Direct3D->GetDevice(), m_fileMap->returnControlbuf(), m_fileMap->returnPbuf(), L"missy.dds");

			else
				modelVector.at(modelID).UpdateBuffers(m_Direct3D->GetDevice(), m_fileMap->returnControlbuf(), m_fileMap->returnPbuf(), L"missy.dds");


			modelID = -1;

		}

		


		//extrude changed mesh
		if (messageType == 7)
		{


		
			//ModelID
			memcpy(&modelID, (char*)pBuf + (sizeof(int)* 3) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(DirectX::XMFLOAT4X4) + sizeof(XMFLOAT4)+sizeof(XMFLOAT4)+sizeof(float)+sizeof(float), sizeof(int));


			if (modelID > 0)
				modelVector.at(modelID - 1).UpdateBuffers(m_Direct3D->GetDevice(), m_fileMap->returnControlbuf(), m_fileMap->returnPbuf(), L"missy.dds");

			else
				modelVector.at(modelID).UpdateBuffers(m_Direct3D->GetDevice(), m_fileMap->returnControlbuf(), m_fileMap->returnPbuf(), L"missy.dds");


			modelID = -1;

		}

unsigned int tempH = *headP;

		if (*tailP < *memSize) // read == *readerAmount) &&
		{
			*tailP += 100000;
			//*readP = 1;
		}
		if (*tailP >= *memSize) //(read == *readerAmount) &&
		{
			*tailP = 0;
		}


	}

	

}

