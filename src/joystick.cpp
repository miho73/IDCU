#include "joystick.h"

short mode = -1;

bool thrustlever_exists = false;
bool sidestick_exists = false;

GUID X56_THRUSTLEVER_ID;
GUID X56_SIDESTICK_ID;

LPDIRECTINPUT8 pDI = NULL;

void FindJoysticks();

void InitializeJoysticks() {
  HRESULT hr = DirectInput8Create(
    GetModuleHandle(NULL),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    (VOID**)&pDI,
    NULL
  );

  if (FAILED(hr)) {
    MEMORed("DI8 CREATION FAULT");
    ECAMBlue("-ERROR", GetHexErrorCode(hr));
	halt();
  }
}

BOOL CALLBACK EnumJoysticksCallback(const DIDEVICEINSTANCE* pdidInstance, VOID* pContext) {
  wstring productName = pdidInstance->tszProductName;

  if (productName == THRUSTLEVER_NAME && IsEqualGUID(pdidInstance->guidProduct, THRUSTLEVER_GUID)) {
    thrustlever_exists = true;
    X56_THRUSTLEVER_ID = pdidInstance->guidInstance;
  }
  else if (productName == SIDESTICK_NAME && IsEqualGUID(pdidInstance->guidProduct, SIDESTICK_GUID)) {
    sidestick_exists = true;
    X56_SIDESTICK_ID = pdidInstance->guidInstance;
  }

  if (thrustlever_exists && sidestick_exists) return DIENUM_STOP;
  else return DIENUM_CONTINUE;
}

void FindJoysticks() {
  HRESULT hr = pDI->EnumDevices(
    DI8DEVCLASS_GAMECTRL,
    EnumJoysticksCallback,
    NULL,
    DIEDFL_ATTACHEDONLY
  );

  if (FAILED(hr)) {
    MEMORed("DI8 DEVICE ENUM FAULT");
    ECAMBlue("-ERROR", GetHexErrorCode(hr));
    halt();
  }

  if (sidestick_exists) {
    try {
      if (FAILED(pDI->CreateDevice(SIDESTICK_GUID, &pSidestick, NULL))) throw;
      if (FAILED(pSidestick->SetDataFormat(&c_dfDIJoystick2))) throw;
      if (FAILED(pSidestick->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) throw;
      if (FAILED(pSidestick->Acquire())) throw;

      sidestick_avail = true;
      ECAMGreen("SIDESTICK", "AVAIL");
    }
    catch (...) {
      ECAMAmber("SIDESTICK", "INOP");
    }
  }
  else ECAMAmber("SIDESTICK", "MISSING");
  if (thrustlever_exists) {
    try {
      if (FAILED(pDI->CreateDevice(THRUSTLEVER_GUID, &pThrust, NULL))) throw;
      if (FAILED(pThrust->SetDataFormat(&c_dfDIJoystick2))) throw;
      if (FAILED(pThrust->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) throw;
      if (FAILED(pThrust->Acquire())) throw;

      thrustlever_avail = true;
      ECAMGreen("THR LVR", "AVAIL");
    }
    catch (...) {
      ECAMAmber("THR LVR", "INOP");
    }
  }
  else ECAMAmber("THR LVR", "MISSING");

  if (!sidestick_avail || !thrustlever_avail) {
	MEMORed("\nDI8 DEVICE INIT FAULT");
	ECAMBlue("-DEVICE", "CHECK");
	ECAMBlue("-CONNECTION", "VERIFY");
    halt();
  }
}

void ReleaseDirectInput() {
  if (pDI) pDI->Release();
}