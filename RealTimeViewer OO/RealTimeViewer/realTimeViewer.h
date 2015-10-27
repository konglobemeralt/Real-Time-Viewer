#ifndef _REALTIMEVIEWER_H_
#define _REALTIMEVIEWER_H_

#include "d3dclass.h"
#include "cameraClass.h"
#include "ShaderShader.h"
#include "modelClass.h"
#include "lightClass.h"
#include "fileMapping.h"
#include "materialClass.h"


const bool FULL_SCREEN = false;
const bool VSYNC_ENABLED = true;
const float SCREEN_DEPTH = 1000.0f;
const float SCREEN_NEAR = 1.f; //0.1

class realTimeViewer
{
public:
	realTimeViewer();
	~realTimeViewer();

	bool Initialize(HINSTANCE, HWND, int, int);
	void Shutdown();
	bool frame();
	//bool m_renterToTexture;


private:
	bool HandleInput(float);
	bool Render();
	bool RenderGraphics();
	
	void update();

	int numberOfModels;
	vector<ModelClass> modelVector;
	vector<MaterialClass> materialVector;

	D3DClass* m_Direct3D;
	CameraClass* m_Camera;
	ShaderShaderClass* m_ShaderShader;
	ModelClass  m_model;
	MaterialClass m_material;
	LightClass *m_Light;
	fileMapping *m_fileMap;
};


#endif
