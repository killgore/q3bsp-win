//#include "stdafx.h"
#include "Utilities.h"
#include "VertexFormats.h"

bool InitD3D( HWND hWnd )
{
	bool ret = false;

    d3dInterface = Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = FALSE;    // windowed / fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
    d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer

    // create a device class using this information and information from the d3dpp stuct
    HRESULT result = d3dInterface->CreateDevice(D3DADAPTER_DEFAULT,
						  D3DDEVTYPE_HAL,
						  hWnd,
						  D3DCREATE_SOFTWARE_VERTEXPROCESSING,
						  &d3dpp,
						  &d3dDevice);

	if( result == D3D_OK )
		ret = true;

	return ret;
}

void RenderD3D( )
{

    d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 0, 0), 1.0f, 0);

    d3dDevice->BeginScene();    // begins the 3D scene

    // do 3D rendering on the back buffer here
	DrawTriangle();

    d3dDevice->EndScene();    // ends the 3D scene

    d3dDevice->Present(NULL, NULL, NULL, NULL);    // displays the created frame

}

void DestroyD3D( )
{
	v_buffer->Release();
	d3dDevice->Release();
	d3dInterface->Release();

	v_buffer = 0;
	d3dDevice = 0;
	d3dInterface = 0;
}

void DrawTriangle()
{

	XYZR_DIFF_VERT triangle[] =
	{
		{320.0f, 50.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 0, 255)},
		{520.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(0, 255, 0)},
		{120.0f, 400.0f, 1.0f, 1.0f, D3DCOLOR_XRGB(255, 0, 0)},
	};

	UINT buffer_size = 3 * sizeof(XYZR_DIFF_VERT);

	d3dDevice->CreateVertexBuffer( buffer_size, 0, XYZR_DIFF, D3DPOOL_MANAGED, &v_buffer, NULL );

	void** pVMem;

	HRESULT res = v_buffer->Lock( 0, 0, (void**)&pVMem, 0 );

	if( res != D3D_OK )
	{
		__asm{ int 3 };
	}

	memcpy(pVMem, triangle, sizeof(triangle));

	v_buffer->Unlock();

	d3dDevice->SetFVF(XYZR_DIFF);

	d3dDevice->SetStreamSource(0, v_buffer, 0, sizeof(XYZR_DIFF_VERT));

	d3dDevice->DrawPrimitive(D3DPT_TRIANGLELIST, 0, 1);

}

void CreateShader()
{

/*	
HRESULT D3DXCompileShaderFromFile(
  LPCSTR pSrcFile,
  CONST D3DXMACRO* pDefines,
  LPD3DXINCLUDE pInclude,
  LPCSTR pFunctionName,
  LPCSTR pProfile,
  DWORD Flags,
  LPD3DXBUFFER* ppShader,
  LPD3DXBUFFER * ppErrorMsgs,
  LPD3DXCONSTANTTABLE * ppConstantTable
);
*/

}