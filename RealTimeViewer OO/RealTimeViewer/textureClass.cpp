#include "textureclass.h"
#include <DDSTextureLoader.h>


TextureClass::TextureClass()
{
	m_texture = 0;
}



TextureClass::~TextureClass()
{
	Shutdown();
}


bool TextureClass::Initialize(ID3D11Device* device, WCHAR* filename)
{
	HRESULT result = 0;af

	

	result = DirectX::CreateDDSTextureFromFile(device, filename, NULL, &m_texture);

	if (FAILED(result))
	{
		return false;
	}

	return true;
}


void TextureClass::Shutdown()
{
	// Release the texture resource.
	if (m_texture)
	{
		m_texture->Release();
		m_texture = 0;
	}

	return;
}


ID3D11ShaderResourceView* TextureClass::GetTexture()
{
	return m_texture;
}