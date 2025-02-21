#pragma once
#ifndef RENDERER_H
#define RENDERER_H 

#include <d3d9.h>
#include <d3dx9math.h>
#include "Camera.h"
#include "q3map.h"

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// include the Direct3D Library file
#pragma comment (lib, "d3d9.lib")

class Renderer
{
public:
	Renderer();
	~Renderer();

	typedef interface ID3DXBuffer *LPD3DXBUFFER;
	typedef interface ID3DXConstantTable *LPD3DXCONSTANTTABLE;

	bool InitD3D( HWND hWnd );
	void RenderD3D();
	void DestroyD3D();
	void InitCamera();
	void ToggleWireFrame();
	void RayTestMap();
	void DrawCrossHair(float x, float y, float width);
	Camera* GetCamera() const { return visCamera; };

private:
	LPDIRECT3D9 d3dInterface;    //Direct3D interface
	LPDIRECT3DDEVICE9 d3dDevice;    //Device class

	LPDIRECT3DVERTEXBUFFER9 v_buffer;

	LPD3DXBUFFER pShader;
	LPD3DXBUFFER pErrorMsgs;
	LPD3DXCONSTANTTABLE pConstantTable;

	ID3DXMesh *_teapotMesh;
	D3DMATERIAL9 _greenMat;
	D3DMATERIAL9 _blueMat;

	Camera *visCamera;

	RECT _clientWin;

	q3map  *q3MapRef; 

	bool _wireFrame;
	D3DXVECTOR3 _initCamPos;

	ID3DXLine *_xHair; 

	void DrawXHair(float x1, float y1, float x2, float y2, float width);

	void CreatePointLight(const D3DXVECTOR3 &position, float range, float atten, int index);

};

#endif //D3DUTILITIES_H