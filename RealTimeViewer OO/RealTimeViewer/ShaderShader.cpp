#include "ShaderShader.h"


ShaderShaderClass::ShaderShaderClass()
{
	m_vertexShader = 0;
	m_pixelShader = 0;
	m_layout = 0;
	m_sampleState = 0;
	m_matrixBuffer = 0;
	m_lightBuffer = 0;
	m_materialBuffer = 0;
}


ShaderShaderClass::~ShaderShaderClass()
{
}

bool ShaderShaderClass::Initialize(ID3D11Device* device, HWND hwnd)
{
	bool result;

	//We use the shadow geometry shader to do backface culling on objects that use the shadow shaders.

	//initializing shader files!
	result = InitializeShader(device, hwnd, L"vertexShader.vs", L"pixelShader.ps");
	if (!result)
	{
		return false;
	}

	return true;
}

void ShaderShaderClass::Shutdown()
{
	ShaderShaderClass();
	return;
}


bool ShaderShaderClass::Render(ID3D11DeviceContext* deviceContext,
	int indexCount,
	XMMATRIX& worldMatrix,
	XMMATRIX& viewMatrix,
	XMMATRIX& projectionMatrix,
	ID3D11ShaderResourceView* texture,
	XMFLOAT4& matColor, XMFLOAT4& matSpecColor, float matReflect, float matSpecRolloff,
	XMFLOAT3& lightDirection,
	XMFLOAT4& diffuseColor)
{

	//shadow map texture is sent in as input, and then sent to the setshaderparameters to set it in the shader before rendering
	//The shadow map texture is called "depthMapTexture" in the parameters, and is the ShaderResourceView taken from the RenderTextureclass.

	bool result;


	//Setting shader parameters, preparing them for render.
	result = SetShaderParameters(deviceContext,
		worldMatrix,
		viewMatrix,
		projectionMatrix,
		texture,
		matColor, matSpecColor, matReflect, matSpecRolloff,
		lightDirection,
		diffuseColor);
	if (!result)
	{
		return false;
	}

	// Now render the prepared buffers with the shader.
	RenderShader(deviceContext, indexCount);

	return true;
}

bool ShaderShaderClass::InitializeShader(ID3D11Device* device, HWND hwnd, WCHAR* vsFilename, WCHAR* psFilename)
{
	HRESULT result;
	ID3D10Blob* errorMessage;
	ID3D10Blob* vertexShaderBuffer;
	ID3D10Blob* pixelShaderBuffer;
	D3D11_INPUT_ELEMENT_DESC polygonLayout[3];
	unsigned int numElements;
	D3D11_SAMPLER_DESC samplerDesc;
	D3D11_BUFFER_DESC matrixBufferDesc;
	D3D11_BUFFER_DESC lightBufferDesc;
	D3D11_BUFFER_DESC materialBufferDesc;

	errorMessage = 0;
	vertexShaderBuffer = 0;
	pixelShaderBuffer = 0;

	//shadow vertex shader
	result = D3DCompileFromFile(vsFilename, NULL, NULL, "vertexShader", "vs_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &vertexShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		//if shader failed to compile
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, vsFilename);
		}
		// if it couldn't find the shader and it was nothing in the error message
		else
		{
			MessageBox(hwnd, (LPCSTR)vsFilename, (LPCSTR)"Missing Vertex Shader File", MB_OK);
		}

		return false;
	}

	//shadow pixel shader
	result = D3DCompileFromFile(psFilename, NULL, NULL, "pixelShader", "ps_5_0", D3D10_SHADER_ENABLE_STRICTNESS, 0, &pixelShaderBuffer, &errorMessage);
	if (FAILED(result))
	{
		//if shader failed to compile
		if (errorMessage)
		{
			OutputShaderErrorMessage(errorMessage, hwnd, psFilename);
		}
		// if it couldn't find the shader and it was nothing in the error message
		else
		{
			MessageBox(hwnd, (LPCSTR)psFilename, (LPCSTR)"Missing Pixel Shader File", MB_OK);
		}

		return false;
	}


	result = device->CreateVertexShader(vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), NULL, &m_vertexShader);
	if (FAILED(result))
	{
		return false;
	}



	result = device->CreatePixelShader(pixelShaderBuffer->GetBufferPointer(), pixelShaderBuffer->GetBufferSize(), NULL, &m_pixelShader);
	if (FAILED(result))
	{
		return false;
	}

	//INPUT LAYOUT:
	// Create the layout description for input into the vertex shader.

	polygonLayout[0].SemanticName = "POSITION";
	polygonLayout[0].SemanticIndex = 0;
	polygonLayout[0].Format = DXGI_FORMAT_R32G32B32A32_FLOAT;
	polygonLayout[0].InputSlot = 0;
	polygonLayout[0].AlignedByteOffset = 0;
	polygonLayout[0].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[0].InstanceDataStepRate = 0;

	polygonLayout[1].SemanticName = "TEXCOORD";
	polygonLayout[1].SemanticIndex = 0;
	polygonLayout[1].Format = DXGI_FORMAT_R32G32_FLOAT;
	polygonLayout[1].InputSlot = 0;
	polygonLayout[1].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[1].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[1].InstanceDataStepRate = 0;

	polygonLayout[2].SemanticName = "NORMAL";
	polygonLayout[2].SemanticIndex = 0;
	polygonLayout[2].Format = DXGI_FORMAT_R32G32B32_FLOAT;
	polygonLayout[2].InputSlot = 0;
	polygonLayout[2].AlignedByteOffset = D3D11_APPEND_ALIGNED_ELEMENT;
	polygonLayout[2].InputSlotClass = D3D11_INPUT_PER_VERTEX_DATA;
	polygonLayout[2].InstanceDataStepRate = 0;

	//count of elements in the layout
	numElements = sizeof(polygonLayout) / sizeof(polygonLayout[0]);

	result = device->CreateInputLayout(polygonLayout, numElements, vertexShaderBuffer->GetBufferPointer(), vertexShaderBuffer->GetBufferSize(), &m_layout);
	if (FAILED(result)){ return false; }

	// we no longer need the shader buffers, so release them
	vertexShaderBuffer->Release();
	vertexShaderBuffer = 0;


	pixelShaderBuffer->Release();
	pixelShaderBuffer = 0;


	// Create a texture sampler state description.
	samplerDesc.Filter = D3D11_FILTER_MIN_MAG_MIP_LINEAR;
	samplerDesc.AddressU = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressV = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.AddressW = D3D11_TEXTURE_ADDRESS_WRAP;
	samplerDesc.MipLODBias = 0.0f;
	samplerDesc.MaxAnisotropy = 1;
	samplerDesc.ComparisonFunc = D3D11_COMPARISON_ALWAYS;
	samplerDesc.BorderColor[0] = 0;
	samplerDesc.BorderColor[1] = 0;
	samplerDesc.BorderColor[2] = 0;
	samplerDesc.BorderColor[3] = 0;
	samplerDesc.MinLOD = 0;
	samplerDesc.MaxLOD = D3D11_FLOAT32_MAX;

	//Create the texture sampler state.
	result = device->CreateSamplerState(&samplerDesc, &m_sampleState);
	if (FAILED(result))
	{
		return false;
	}



	//CONSTANT BUFFER DESCRIPTIONS:
	//this is the dynamic matrix constant buffer that is in the VERTEX SHADER
	matrixBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	matrixBufferDesc.ByteWidth = sizeof(MatrixBufferType);
	matrixBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	matrixBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	matrixBufferDesc.MiscFlags = 0;
	matrixBufferDesc.StructureByteStride = 0;

	//create a pointer to constant buffer, so we can acess the vertex shader constant buffer within this class
	result = device->CreateBuffer(&matrixBufferDesc, NULL, &m_matrixBuffer);
	if (FAILED(result)) { return false; }



	//Setup the description of the light dynamic constant buffer that is in the pixel shader.
	//Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	lightBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	lightBufferDesc.ByteWidth = sizeof(LightBufferType);
	lightBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	lightBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	lightBufferDesc.MiscFlags = 0;
	lightBufferDesc.StructureByteStride = 0;

	//Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&lightBufferDesc, NULL, &m_lightBuffer);
	if (FAILED(result))
	{
		return false;
	}



	//Setup the description of the material buffer that is in the pixel shader.
	//Note that ByteWidth always needs to be a multiple of 16 if using D3D11_BIND_CONSTANT_BUFFER or CreateBuffer will fail.
	materialBufferDesc.Usage = D3D11_USAGE_DYNAMIC;
	materialBufferDesc.ByteWidth = sizeof(MaterialBufferType);
	materialBufferDesc.BindFlags = D3D11_BIND_CONSTANT_BUFFER;
	materialBufferDesc.CPUAccessFlags = D3D11_CPU_ACCESS_WRITE;
	materialBufferDesc.MiscFlags = 0;
	materialBufferDesc.StructureByteStride = 0;

	//Create the constant buffer pointer so we can access the vertex shader constant buffer from within this class.
	result = device->CreateBuffer(&materialBufferDesc, NULL, &m_materialBuffer);
	if (FAILED(result))
	{
		return false;
	}



	return true;

}

void ShaderShaderClass::ShutdownShader()
{
	if (m_layout)
	{
		m_layout->Release();
		m_layout = 0;
	}

	if (m_lightBuffer)
	{
		m_lightBuffer->Release();
		m_lightBuffer = 0;
	}

	if (m_materialBuffer)
	{
		m_materialBuffer->Release();
		m_materialBuffer = 0;
	}


	if (m_matrixBuffer)
	{
		m_matrixBuffer->Release();
		m_matrixBuffer = 0;
	}

	if (m_pixelShader)
	{
		m_pixelShader->Release();
		m_pixelShader = 0;
	}


	if (m_sampleState)
	{
		m_sampleState->Release();
		m_sampleState = 0;
	}


	if (m_vertexShader)
	{
		m_vertexShader->Release();
		m_vertexShader = 0;
	}



	return;
}

void ShaderShaderClass::OutputShaderErrorMessage(ID3D10Blob* errorMessage, HWND hwnd, WCHAR* shaderFilename)
{
	char* compileErrors;
	unsigned long bufferSize, i;
	ofstream fout;


	// Get a pointer to the error message text buffer.
	compileErrors = (char*)(errorMessage->GetBufferPointer());

	// Get the length of the message.
	bufferSize = errorMessage->GetBufferSize();

	// Open a file to write the error message to.
	fout.open("shader-error.txt");

	// Write out the error message.
	for (i = 0; i<bufferSize; i++)
	{
		fout << compileErrors[i];
	}


	// Close the file.
	fout.close();

	// Release the error message.
	errorMessage->Release();
	errorMessage = 0;

	// Pop a message up on the screen to notify the user to check the text file for compile errors.
	MessageBox(hwnd, "Error compiling shader.  Check shader-error.txt for message.", (LPCSTR)shaderFilename, MB_OK);

	return;
}


bool ShaderShaderClass::SetShaderParameters(ID3D11DeviceContext* deviceContext, XMMATRIX& worldMatrix, XMMATRIX& viewMatrix, XMMATRIX& projectionMatrix,
	ID3D11ShaderResourceView* texture, XMFLOAT4& matColor, XMFLOAT4& matSpecColor, float matReflect, float matSpecRolloff, XMFLOAT3& lightDirection, XMFLOAT4& diffuseColor)
{
	HRESULT result;
	D3D11_MAPPED_SUBRESOURCE mappedResource;
	unsigned int bufferNr;
	MatrixBufferType* dataPtr;
	LightBufferType* dataPtr2;
	MaterialBufferType* dataPtr3;

	XMMATRIX worldMatrixC, viewMatrixC, projectionMatrixC;

	//transposing the matrices
	worldMatrixC = XMMatrixTranspose(worldMatrix);
	viewMatrixC = XMMatrixTranspose(viewMatrix);
	projectionMatrixC = XMMatrixTranspose(projectionMatrix);


	//Setting and mapping the constant buffers (that were described in the InitializeShader) for use in the shaders

	//lock the constant buffer for writing
	result = deviceContext->Map(m_matrixBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result)) { return false; }

	//find pointer to data in the matrix constant buffer!
	dataPtr = (MatrixBufferType*)mappedResource.pData;

	//fill in the matrix buffer with the information from the transposed WVP etc matrices
	dataPtr->world = worldMatrixC;
	dataPtr->view = viewMatrixC;
	dataPtr->projection = projectionMatrixC;


	//unlock the constant buffer again
	deviceContext->Unmap(m_matrixBuffer, 0);

	//set the position of the constant buffer in the vertexshader
	bufferNr = 0;

	//setting matrix constant buffer in the VS with its new and updated values
	deviceContext->VSSetConstantBuffers(bufferNr, 1, &m_matrixBuffer);


	//setting the sent in shader texture resource in the pixel shader
	deviceContext->PSSetShaderResources(0, 1, &texture);



	// Lock the light constant buffer so it can be written to.
	result = deviceContext->Map(m_lightBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr2 = (LightBufferType*)mappedResource.pData;

	// Copy the lighting variables into the constant buffer.
	dataPtr2->diffuseColor = diffuseColor;
	dataPtr2->lightDirection = lightDirection;
	dataPtr2->padding = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_lightBuffer, 0);





	// Lock the material constant buffer so it can be written to.
	result = deviceContext->Map(m_materialBuffer, 0, D3D11_MAP_WRITE_DISCARD, 0, &mappedResource);
	if (FAILED(result))
	{
		return false;
	}

	// Get a pointer to the data in the constant buffer.
	dataPtr3 = (MaterialBufferType*)mappedResource.pData;

	// Copy the material variables into the constant buffer.
	dataPtr3->materialColor = matColor;
	dataPtr3->materialSpecColor = matSpecColor;
	dataPtr3->materialReflectivity = matReflect;
	dataPtr3->materialSpecRolloff = matSpecRolloff;
	dataPtr3->padding = 0.0f;
	dataPtr3->padding2 = 0.0f;

	// Unlock the constant buffer.
	deviceContext->Unmap(m_materialBuffer, 0);


	// Set the position of the light constant buffer in the pixel shader.
	bufferNr = 0;

	// Finally set the light constant buffer in the pixel shader with the updated values.
	deviceContext->PSSetConstantBuffers(bufferNr, 1, &m_lightBuffer);


	//set material constant buffer in the pixel shader
	deviceContext->PSSetConstantBuffers(1, 1, &m_materialBuffer);

	return true;

}

void ShaderShaderClass::RenderShader(ID3D11DeviceContext* deviceContext, int index)
{
	deviceContext->IASetInputLayout(m_layout);

	//set which vertex pixel geometry shaders that will be used for this triangle

	deviceContext->VSSetShader(m_vertexShader, NULL, 0);

	deviceContext->PSSetShader(m_pixelShader, NULL, 0);

	//set the clamp and wrap sampler states and which register they belong to
	deviceContext->PSSetSamplers(0, 1, &m_sampleState);


	//aaand render!
	deviceContext->DrawIndexed(index, 0, 0);

	return;
}