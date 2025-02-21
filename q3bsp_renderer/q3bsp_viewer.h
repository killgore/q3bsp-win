#pragma once
#ifndef Q3BSP_RENDERER_H
#define Q3BSP_RENDERER_H

#include "resource.h"

// define the keyboard macros
#define KEY_DOWN(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 1 : 0)
#define KEY_UP(vk_code) ((GetAsyncKeyState(vk_code) & 0x8000) ? 0 : 1)

void ProcessKeys(HWND hWnd);

static ULONG wireTick = 0;

static Renderer *q3r = NULL;

#endif