#ifndef _MATERIALCLASS_H_
#define _MATERIALCLASS_H_


#include <d3d11.h>
#include <DirectXMath.h>

class MaterialClass
{
public:
	MaterialClass();
	~MaterialClass();

	bool Initialize();
	void Shutdown();

	bool MaterialClass::updateMaterial(void* cBuf, void* pBuf);
	

	DirectX::XMFLOAT4 MaterialClass::getMatColor();
	

	DirectX::XMFLOAT4 MaterialClass::getMatSpecColor();
	

	float MaterialClass::getMatReflectivity();
	

	float MaterialClass::getMatSpecRolloff();
	

private:
	ID3D11ShaderResourceView* m_texture;
	
	//material values
bool m_HasMaterial;
float m_diffuseR, m_diffuseG, m_diffuseB;
char m_diffuseTexturePath[50];

DirectX::XMFLOAT4 m_matColor;
DirectX::XMFLOAT4 m_matSpecColor;
float m_matReflectivity;
float m_matSpecRolloff;

};

#endif

