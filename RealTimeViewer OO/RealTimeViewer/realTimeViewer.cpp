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

realTimeViewer::~realTimeViewer() {};


bool realTimeViewer::Initialize(HINSTANCE hinstance, HWND hwnd, int screenWidth, int screenHeight)
{
	bool result;

	char videoCard[128];
	int videoMemory;

	//Create the fileMapping object
	m_fileMap = new SharedMemory;
	if (!m_fileMap)
	{
		return false;
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



	//create shader object
	MaterialClass DefaultMaterial;

	//initialize material 
	//DefaultMaterial.LoadTexture(m_Direct3D->GetDevice());

	materialVector.push_back(DefaultMaterial);




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
	m_Direct3D->BeginScene(0.7f, 0.7f, 0.6f, 1.0f);

	//// Generate the view matrix based on the camera's position.
	//m_Camera->Render(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());

	m_Camera->GetViewMatrix(viewMatrix);
	m_Direct3D->GetWorldMatrix(worldMatrix);
	m_Direct3D->GetProjectionMatrix(projectionMatrix);


	//update model if necessary
	for (int i = 0; i < modelVector.size(); i++)
	{

		if (modelVector.at(i).GetIndexCount() > 0)
		{

			//Put the model vertex and index buffers on the graphics pipeline to prepare for drawing.
			int matID = modelVector.at(i).getMatID();

			materialVector.at(matID).LoadTexture(m_Direct3D->GetDevice());

						
			modelVector.at(i).Render(m_Direct3D->GetDeviceContext());
			// Get the world, view, projection, and ortho matrices from the camera and Direct3D objects.


			//Move the model to the location it should be rendered at.
			worldMatrix = modelVector.at(i).getWorldMatrix();

			


			result = m_ShaderShader->Render(m_Direct3D->GetDeviceContext(),
				modelVector.at(i).GetIndexCount(),
				worldMatrix,
				viewMatrix,
				projectionMatrix,
				materialVector.at(matID).GetTexture(),
				materialVector.at(matID).getMatColor(),
				materialVector.at(matID).getMatSpecColor(),
				materialVector.at(matID).getMatReflectivity(),
				materialVector.at(matID).getMatSpecRolloff(),
				XMFLOAT3(m_Light->GetPosition().x, m_Light->GetPosition().y, m_Light->GetPosition().z),
				m_Light->GetDiffuseColor());


			materialVector.at(matID).ReleaseTexture();

			if (!result)
			{
				return false;
			}

		}
	}

	


	// Present the rendered scene to the screen.
	m_Direct3D->EndScene();

	update();



	return true;
}

void realTimeViewer::update()
{


	if (m_fileMap->cb->tail != m_fileMap->cb->head)
	{

		int messageType = -1;
		memcpy(&messageType, (char*)m_fileMap->buffer + m_fileMap->cb->tail, sizeof(int));
		int messageSize = -1;
		memcpy(&messageSize, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int), sizeof(int));

		int modelID = -1;



		if (messageType != -1)
		{
			//Create mesh MESSAGE 0
			if (messageType == 0)
			{


				//ModelID
				memcpy(&modelID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));

				if (modelID > modelVector.size())
				{
					
					m_model.Initialize(m_Direct3D->GetDevice(), ((char*)m_fileMap->buffer + m_fileMap->cb->tail));
					modelVector.push_back(m_model);

				}

				modelID = -1;

			}



			//Create mesh MESSAGE 0
			if (messageType == 1)
			{


				m_Camera->Render((char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int)) ;

			}



			//Transform mesh
			if (messageType == 2)
			{
				//ModelID
				memcpy(&modelID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(XMFLOAT4X4) + sizeof(int) + sizeof(int), sizeof(int));

				XMFLOAT4X4 tempMatrix;
				memcpy(&tempMatrix, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(DirectX::XMFLOAT4X4));

				if (modelID > 0)
					modelVector.at(modelID - 1).setWorldMatrix(tempMatrix);

				else
					modelVector.at(modelID).setWorldMatrix(tempMatrix);

				modelID = -1;
			}

			//material change
			if (messageType == 4)
			{
				int matID = -1;
				//ModelID
	
				//for matID we use messageSize from the mayaplugin since that one is not used for other things
				memcpy(&matID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));
	
				if (matID != -1 && matID < materialVector.size())
				{
	
				
	
					//	if (matID > 0)
					//	{
					//
					//		materialVector.at(matID - 1).updateMaterial(m_fileMap->returnControlbuf(), m_fileMap->returnPbuf());
					//
					//	}
					//
					//	else
					//	{
					materialVector.at(matID).ReleaseTexture();
					
					
					materialVector.at(matID).updateMaterial((char*)m_fileMap->buffer + m_fileMap->cb->tail);
	
	
					//	}
	
	
					modelID = -1;
	
				}
	
	
	
	
	
			}



		//Delete Mesh
		if (messageType == 5)
		{
			//ModelID
			memcpy(&modelID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + (sizeof(int)) + sizeof(int), sizeof(int));
	
	
			if (modelID > 0)
				modelVector.at(modelID - 1).setIndexCount(-1);
	
			else
				modelVector.at(modelID).setIndexCount(-1);
	
			modelID = -1;
		}

		//vertex changed mesh
		if (messageType == 6)
		{
	
			//ModelID
			memcpy(&modelID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));
	
	
	
			if (modelID > 0)
				modelVector.at(modelID - 1).UpdateBuffers(m_Direct3D->GetDevice(), (char*)m_fileMap->buffer + m_fileMap->cb->tail);
	
			else
				modelVector.at(modelID).UpdateBuffers(m_Direct3D->GetDevice(), (char*)m_fileMap->buffer + m_fileMap->cb->tail);
	
	
			modelID = -1;
	
		}
	


		//extrude changed mesh
		if (messageType == 7)
		{
	
	
	
			//ModelID
			memcpy(&modelID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));
	
	
			if (modelID > 0)
				modelVector.at(modelID - 1).UpdateBuffers(m_Direct3D->GetDevice(), (char*)m_fileMap->buffer + m_fileMap->cb->tail);
	
			else
				modelVector.at(modelID).UpdateBuffers(m_Direct3D->GetDevice(), (char*)m_fileMap->buffer + m_fileMap->cb->tail);
	
	
			modelID = -1;
	
		}

			///// ASSIGN NEW HERER
				//Material Assign message
			if (messageType == 8)
			{
				int meshID = -1;
				//MeshID
				memcpy(&meshID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));
	
	
				int matID = -1;
				//matID
				memcpy(&matID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int) + sizeof(int), sizeof(int));
	
	
				if (matID != -1 && meshID != -1)
				{
	
	
					modelVector.at(meshID - 1).setMaterialID(matID);
				}
	}








		//NewMat
		if (messageType == 9)
		{
		
			int matID = -1;
			//ModelID
		
			//for matID we use messageSize from the mayaplugin since that one is not used for other things
			memcpy(&matID, (char*)m_fileMap->buffer + m_fileMap->cb->tail + sizeof(int) + sizeof(int), sizeof(int));
		
			if (matID != -1)
			{
				//Works ish
				//if (matID + 1 > materialVector.size() - 1)
				if (matID > materialVector.size())
				{
					MaterialClass material;
					//material.updateMaterial((char*)m_fileMap->buffer + m_fileMap->cb->tail);
					materialVector.push_back(material);
		
				}
			}
		}




			if (m_fileMap->cb->tail < m_fileMap->memSize) // read == *readerAmount) &&
			{
				m_fileMap->cb->tail += messageSize;
				//*readP = 1;
			}
			if (m_fileMap->cb->tail >= m_fileMap->memSize) //(read == *readerAmount) &&
			{
				m_fileMap->cb->tail = 0;
			}

		}
	}


}