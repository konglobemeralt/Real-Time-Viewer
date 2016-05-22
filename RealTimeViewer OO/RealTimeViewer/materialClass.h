#ifndef _MATERIALCLASS_H_
#define _MATERIALCLASS_H_

#include <d3d11.h>
#include <DirectXMath.h>
#include <d3d11.h>
#include "textureClass.h"


class MaterialClass
{
public:
	MaterialClass();
	~MaterialClass();

	bool Initialize();
	void Shutdown();
	

	bool MaterialClass::updateMaterial(void* cBuf);
	

	DirectX::XMFLOAT4 MaterialClass::getMatColor();
	

	DirectX::XMFLOAT4 MaterialClass::getMatSpecColor();
	

	float MaterialClass::getMatReflectivity();
	

	float MaterialClass::getMatSpecRolloff();

	WCHAR* MaterialClass::getTexturePath();

	ID3D11ShaderResourceView* GetTexture();
	bool LoadTexture(ID3D11Device*);
	void ReleaseTexture();

	
	WCHAR m_texturePath[100];
	char tempPath[200];
private:
	ID3D11ShaderResourceView* m_texture;
	
	//material values
TextureClass* m_pTexture;

bool m_HasMaterial;
float m_diffuseR, m_diffuseG, m_diffuseB;

WCHAR* m_diffuseTexturePath;



DirectX::XMFLOAT4 m_matColor;
DirectX::XMFLOAT4 m_matSpecColor;
float m_matReflectivity;
float m_matSpecRolloff;

};

#endif

