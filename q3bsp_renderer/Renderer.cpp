//#include "stdafx.h"
#include "Renderer.h"
#include "VertexFormats.h"
#include "assert.h"

Renderer::Renderer() : _teapotMesh(NULL), _wireFrame(false)
{

}

bool Renderer::InitD3D( HWND hWnd )
{
    d3dInterface = (LPDIRECT3D9)Direct3DCreate9(D3D_SDK_VERSION);    // create the Direct3D interface

    D3DPRESENT_PARAMETERS d3dpp;    // create a struct to hold various device information

    ZeroMemory(&d3dpp, sizeof(d3dpp));    // clear out the struct for use
    d3dpp.Windowed = TRUE;    // windowed / fullscreen
    d3dpp.SwapEffect = D3DSWAPEFFECT_DISCARD;    // discard old frames
    d3dpp.hDeviceWindow = hWnd;    // set the window to be used by Direct3D
    d3dpp.BackBufferFormat = D3DFMT_X8R8G8B8;    // set the back buffer format to 32-bit
    d3dpp.BackBufferWidth = SCREEN_WIDTH;    // set the width of the buffer
    d3dpp.BackBufferHeight = SCREEN_HEIGHT;    // set the height of the buffer
	d3dpp.PresentationInterval = D3DPRESENT_INTERVAL_ONE;
	d3dpp.EnableAutoDepthStencil = TRUE;
	d3dpp.AutoDepthStencilFormat = D3DFMT_D16;

    // create a device class using this information and information from the d3dpp stuct
    HRESULT result = d3dInterface->CreateDevice(D3DADAPTER_DEFAULT,
						  D3DDEVTYPE_HAL,
						  hWnd,
						  D3DCREATE_HARDWARE_VERTEXPROCESSING,
						  &d3dpp,
						  &d3dDevice);

	if(FAILED(result))
	{
		assert(0);
		return false;
	}

	// Turn on the zbuffer
	d3dDevice->SetRenderState( D3DRS_ZENABLE, D3DZB_TRUE );

	d3dDevice->SetRenderState( D3DRS_AMBIENT, D3DCOLOR_XRGB(20,20,20));
	d3dDevice->SetRenderState( D3DRS_LIGHTING, TRUE);

	d3dDevice->SetRenderState( D3DRS_CULLMODE, D3DCULL_NONE );

	d3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_GOURAUD );  
	//d3dDevice->SetRenderState( D3DRS_SHADEMODE, D3DSHADE_FLAT ); 

/*
	result = D3DXCreateTeapot(d3dDevice,&_teapotMesh,NULL);
	if (FAILED(result))
	{
		assert(0);
	}

	ZeroMemory( &_greenMat, sizeof(D3DMATERIAL9) );
	_greenMat.Diffuse.r = _greenMat.Ambient.r = 0;
	_greenMat.Diffuse.g = _greenMat.Ambient.g = 1.0f;
	_greenMat.Diffuse.b = _greenMat.Ambient.b = 0;
	_greenMat.Diffuse.a = _greenMat.Ambient.a = 1.0f;
*/

	// Set up matrix
	GetWindowRect(hWnd, &_clientWin);
	D3DXMATRIX matProj;
	float aspect = (_clientWin.right - _clientWin.left) / (float)(_clientWin.bottom - _clientWin.top);
	D3DXMatrixPerspectiveFovLH( &matProj, D3DX_PI/4, aspect, 1.0f, 3000.0f );
	d3dDevice->SetTransform( D3DTS_PROJECTION, &matProj );

	D3DXCreateLine(d3dDevice, &_xHair);  

	//q3dm1 specific
	_initCamPos = D3DXVECTOR3(675.0f, 80.0f, -2060.0f);

	CreatePointLight(_initCamPos, 1000.0f, 0.5f, 1);
	//arbitrary position in other rooms
	CreatePointLight(D3DXVECTOR3(675.0f, 80.0f, -645.0f), 1000.0f, 0.5f, 2);

	visCamera = new Camera(_initCamPos);

	q3MapRef = new q3map("..\\q3data\\q3dm1.bsp");
	if( !q3MapRef )
	{
		assert(0);
		return false;
	}

	//Creates vertex buffer too
	if(!q3MapRef->LoadMap(d3dDevice, v_buffer))
	{
		assert(0);
		return false;
	}

	return true;
}

void Renderer::RenderD3D()
{
    d3dDevice->Clear(0, NULL, D3DCLEAR_TARGET | D3DCLEAR_ZBUFFER, D3DCOLOR_XRGB(192, 192, 255), 1.0f, 0);

    d3dDevice->BeginScene();    

	D3DXMATRIX viewMatrix;
	visCamera->CalculateViewMatrix(&viewMatrix);
	d3dDevice->SetTransform(D3DTS_VIEW, &viewMatrix);

	//Update visible data
	q3MapRef->BuildVis(visCamera->GetPosition());

	//Render map
	D3DXMATRIX worldMatrix;
	D3DXMatrixTranslation(&worldMatrix ,0.0f, 0.0f, 0.0f);
	d3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix ); 
	q3MapRef->DrawMap(d3dDevice, v_buffer, visCamera->GetPosition());

	DrawCrossHair(_clientWin.right/2, _clientWin.bottom/2, 2); 

	/*
	d3dDevice->SetMaterial( &_greenMat );	
	D3DXMATRIX worldMatrix;
	D3DXMatrixTranslation(&worldMatrix, initCamPos.x, initCamPos.y, initCamPos.z + 20);
	d3dDevice->SetTransform( D3DTS_WORLD, &worldMatrix ); 
	_teapotMesh->DrawSubset(0);	
	*/

    d3dDevice->EndScene();    // ends the 3D scene

    d3dDevice->Present(NULL, NULL, NULL, NULL);    // displays the created frame
}

void Renderer::RayTestMap()
{
	q3MapRef->RayTest(visCamera, d3dDevice, v_buffer);
}

void Renderer::ToggleWireFrame()
{
	if(_wireFrame)
	{
		d3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_SOLID );
		_wireFrame = false;
	}
	else
	{
		d3dDevice->SetRenderState( D3DRS_FILLMODE, D3DFILL_WIREFRAME );
		_wireFrame = true;
	}
}

void Renderer::DrawCrossHair(float x, float y, float width)
{
	int xHairOffset = 10;

	DrawXHair(x-xHairOffset, y, x+xHairOffset, y, width);
	DrawXHair(x, y-xHairOffset, x, y+xHairOffset, width);
}

void Renderer::DrawXHair(float x1, float y1, float x2, float y2, float width)
{
    D3DXVECTOR2 lines[2];

    _xHair->SetWidth( width );
    _xHair->SetAntialias( false );
    _xHair->SetGLLines( true );
    
    lines[0].x = x1;
    lines[0].y = y1;
    lines[1].x = x2;
    lines[1].y = y2;  

    _xHair->Begin( );
    _xHair->Draw( lines, 2, 0xFFFF0000);
    _xHair->End( );
}

void Renderer::CreatePointLight(const D3DXVECTOR3 &position, float range, float atten, int index)
{
	D3DLIGHT9 light;
	ZeroMemory( &light, sizeof(D3DLIGHT9) );
	light.Type       = D3DLIGHT_POINT;
	light.Diffuse.r  = 1.0f;
	light.Diffuse.g  = 1.0f;
	light.Diffuse.b  = 1.0f;

	light.Position = position;
   
	light.Attenuation0 = atten;//0.1f;
	light.Range = range;//200.0f;
	d3dDevice->SetLight( index, &light );
	d3dDevice->LightEnable( index, TRUE ); 
}

void Renderer::DestroyD3D( )
{
	v_buffer->Release();
	d3dDevice->Release();
	d3dInterface->Release();

	delete q3MapRef;

	v_buffer = 0;
	d3dDevice = 0;
	d3dInterface = 0;
}


