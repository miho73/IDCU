#pragma once
#include "IDCU.h"
#include <dinput.h>
#include "thrust_lever.h"
#include "sidestick.h"

#pragma comment(lib, "dinput8.lib")
#pragma comment(lib, "dxguid.lib")

#define DIRECTINPUT_VERSION 0x0800

#define THRUSTLEVER_NAME L"Saitek Pro Flight X-56 Rhino Throttle"
#define THRUSTLEVER_GUID { 0xA2210738, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } }
#define SIDESTICK_NAME L"Saitek Pro Flight X-56 Rhino Stick"
#define SIDESTICK_GUID { 0x22210738, 0x0000, 0x0000, { 0x00, 0x00, 0x50, 0x49, 0x44, 0x56, 0x49, 0x44 } }

extern short mode;

void InitializeJoysticks();
void FindJoysticks();
void ReleaseDirectInput();
