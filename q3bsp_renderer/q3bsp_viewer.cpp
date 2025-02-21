// include the basic windows header files and the Direct3D header file
#include <windows.h>
#include <windowsx.h>
#include "Renderer.h"
#include "q3bsp_viewer.h"
#include "q3map.h"
#include "assert.h"

// the WindowProc function prototype
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

int lastMouseX = 0;
int lastMouseY = 0;

RECT winRct; 
LONG winCenter[2];

bool useMouse = false;

// the entry point for any Windows program
int WINAPI WinMain(HINSTANCE hInstance,
                   HINSTANCE hPrevInstance,
                   LPSTR lpCmdLine,
                   int nCmdShow)
{
    HWND hWnd;
    WNDCLASSEX wc;

    ZeroMemory(&wc, sizeof(WNDCLASSEX));

    wc.cbSize = sizeof(WNDCLASSEX);
    wc.style = CS_HREDRAW | CS_VREDRAW;
    wc.lpfnWndProc = WindowProc;
    wc.hInstance = hInstance;
    wc.hCursor = LoadCursor(NULL, IDC_ARROW);
    wc.lpszClassName = L"WindowClass";

    RegisterClassEx(&wc);

    hWnd = CreateWindowEx(WS_EX_CLIENTEDGE,// | WS_EX_TOPMOST,
                          L"WindowClass",
                          L"Q3 Map Viewer",
                          WS_OVERLAPPEDWINDOW,    
                          0, 0,    // the starting x and y positions should be 0
                          SCREEN_WIDTH, SCREEN_HEIGHT,    // set the window to 640 x 480
                          NULL,
                          NULL,
                          hInstance,
                          NULL);
	
    ShowWindow(hWnd, nCmdShow);

	q3r = new Renderer();

    // set up and initialize Direct3D
    if(!q3r->InitD3D(hWnd))
	{
		assert(0);
		return 0;
	}

    // enter the main loop:
    MSG msg;

	GetClientRect(hWnd,&winRct);

	winCenter[0] = winRct.right / 2;
	winCenter[1] = winRct.bottom / 2;

    while(TRUE)
    {
        while(PeekMessage(&msg, NULL, 0, 0, PM_REMOVE))
        {
            TranslateMessage(&msg);
            DispatchMessage(&msg);
        }

        if(msg.message == WM_QUIT)
            break;

        q3r->RenderD3D();

		ProcessKeys(hWnd);
    }

    // clean up DirectX and COM
    q3r->DestroyD3D();

    return msg.wParam;
}


// this is the main message handler for the program
LRESULT CALLBACK WindowProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch(message)
    {
        case WM_DESTROY:
            {
                PostQuitMessage(0);
                return 0;
            } break;

		case WM_LBUTTONDOWN: 
			{
				q3r->RayTestMap();
				//SetCapture(hWnd);
				break;
			}

		case WM_LBUTTONUP:
			{
				//ReleaseCapture();
				break;
			}

		case WM_MOUSEMOVE:
			{
				if(useMouse)
				{
					// Retrieve mouse screen position
					int x = (short)LOWORD(lParam);
					int y = (short)HIWORD(lParam);

					// Check to see if the left button is held down:
					bool leftButtonDown = wParam & MK_LBUTTON;

					// Check if right button down:
					bool rightButtonDown = wParam & MK_RBUTTON;

					if( x != lastMouseX )
					{
						int diffX = x - lastMouseX;
						if( diffX < 0 )
						{
							q3r->GetCamera()->Yaw(-CAM_ROT_SPEED_MOUSE*abs(diffX));
						}
						else
						{
							q3r->GetCamera()->Yaw(CAM_ROT_SPEED_MOUSE*abs(diffX));
						}

						lastMouseX = x;
					}

					if( y != lastMouseY )
					{
						int diffY = y - lastMouseY;
						if( diffY < 0 )
						{
							q3r->GetCamera()->Pitch(-CAM_ROT_SPEED_MOUSE*abs(diffY));
						}
						else
						{
							q3r->GetCamera()->Pitch(CAM_ROT_SPEED_MOUSE*abs(diffY));
						}

						lastMouseY = y;
					}
				}				
				break;
			}
    }

    return DefWindowProc (hWnd, message, wParam, lParam);
}

void ProcessKeys(HWND hWnd)
{
    // check the 'escape' key
    if(KEY_DOWN(VK_ESCAPE))
        PostMessage(hWnd, WM_DESTROY, 0, 0);

	if(KEY_DOWN(VK_UP) || KEY_DOWN('W'))
	{
		q3r->GetCamera()->MoveForward(CAM_MOVE_SPEED);
	}

	if(KEY_DOWN(VK_DOWN) || KEY_DOWN('S'))
	{
		q3r->GetCamera()->MoveForward(-CAM_MOVE_SPEED);
	}

	if(KEY_DOWN(VK_LEFT) || KEY_DOWN('A'))
	{
		q3r->GetCamera()->MoveRight(-CAM_MOVE_SPEED);
	}

	if(KEY_DOWN(VK_RIGHT) || KEY_DOWN('D'))
	{
		q3r->GetCamera()->MoveRight(CAM_MOVE_SPEED);
	}

	if(KEY_DOWN('X'))
	{
		useMouse = !useMouse;
	}

	if(KEY_DOWN(VK_ADD))
	{
		q3r->GetCamera()->MoveUp(CAM_MOVE_SPEED);
	}

	if(KEY_DOWN(VK_SUBTRACT))
	{
		q3r->GetCamera()->MoveUp(-CAM_MOVE_SPEED);
	}

	if(KEY_DOWN(VK_NUMPAD4) || KEY_DOWN('Q'))
	{
		q3r->GetCamera()->Yaw(-CAM_ROT_SPEED_KB);
	}

	if(KEY_DOWN(VK_NUMPAD6) || KEY_DOWN('E'))
	{
		q3r->GetCamera()->Yaw(CAM_ROT_SPEED_KB);
	}

	if(KEY_DOWN(VK_NUMPAD8) )//|| KEY_DOWN('C'))
	{
		q3r->GetCamera()->Pitch(-CAM_ROT_SPEED_KB);
	}

	if(KEY_DOWN(VK_NUMPAD2) )//|| KEY_DOWN('X'))
	{
		q3r->GetCamera()->Pitch(CAM_ROT_SPEED_KB);
	}
/*
	if(KEY_DOWN(VK_NUMPAD7))
	{
		q3r->visCamera->Roll(CAM_ROT_SPEED_KB);
	}

	if(KEY_DOWN(VK_NUMPAD9))
	{
		q3r->visCamera->Roll(-CAM_ROT_SPEED_KB);
	}
*/
	if(KEY_DOWN(VK_NUMPAD5) || KEY_DOWN('I'))
	{
		ULONG t = GetTickCount();
		if((t - wireTick) > 500)
		{
			q3r->ToggleWireFrame();
			wireTick = t;
		}
	}

	if(KEY_DOWN('F'))
	{
		q3r->RayTestMap();
	}
}