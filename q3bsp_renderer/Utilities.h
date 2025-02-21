#pragma once
#ifndef D3DUTILITIES_H
#define D3DUTILITIES_H 

#include <d3d9.h>
//#include <d3dx10.h>

#define SCREEN_WIDTH  1024
#define SCREEN_HEIGHT 768

// include the Direct3D Library file
//#pragma comment (lib, "d3d9.lib")

typedef interface ID3DXBuffer *LPD3DXBUFFER;
typedef interface ID3DXConstantTable *LPD3DXCONSTANTTABLE;

static LPDIRECT3D9 d3dInterface;    //Direct3D interface
static LPDIRECT3DDEVICE9 d3dDevice;    //Device class

static LPDIRECT3DVERTEXBUFFER9 v_buffer;

static LPD3DXBUFFER pShader;
static LPD3DXBUFFER pErrorMsgs;
static LPD3DXCONSTANTTABLE pConstantTable;

bool InitD3D( HWND hWnd );

void RenderD3D();

void DestroyD3D();

void DrawTriangle();

void CreateShader();

#endif //D3DUTILITIES_H