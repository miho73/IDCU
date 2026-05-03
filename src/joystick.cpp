#include "joystick.h"

int mode = -1;

LPDIRECTINPUT8 pDI = NULL;

void InitializeJoysticks();

void InitializeDirectInput() {
  HRESULT hr = DirectInput8Create(
    GetModuleHandle(NULL),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    (VOID**)&pDI,
    NULL
  );

  if (FAILED(hr)) {
	pDI = NULL;
	FlagUp(&joystick_ecam_msg, DIRECT_INPUT_INIT_FAULT);
  }
  else
	FlagDown(&joystick_ecam_msg, DIRECT_INPUT_INIT_FAULT);
}

void JoystickLoop() {
  if (pThrust == NULL || pSidestick == NULL) {
    InitializeJoysticks();
  }

  if (pThrust != NULL)
    ProcessThrustLeverInput();
  if (pSidestick != NULL)
    ProcessSidestickInput();
}

BOOL CALLBACK EnumJoysticksCallback(
  const DIDEVICEINSTANCE* pdidInstance,
  VOID* pContext
) {
  wstring productName = pdidInstance->tszProductName;
  
  
  if (
      //productName == THRUSTLEVER_NAME &&
      IsEqualGUID(pdidInstance->guidProduct, THRUSTLEVER_GUID)
  ) {
    HRESULT hr = 0;
	hr &= pDI->CreateDevice(pdidInstance->guidInstance, &pThrust, NULL);
	hr &= pThrust->SetDataFormat(&c_dfDIJoystick2);
	hr &= pThrust->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    hr &= pThrust->Acquire();

    if (FAILED(hr)) {
      pThrust = NULL;
	  FlagUp(&joystick_ecam_msg, THRUST_INIT_FAULT);
    }
	else
      FlagDown(&joystick_ecam_msg, THRUST_INIT_FAULT);
  }
  else if (
      //productName == SIDESTICK_NAME &&
      IsEqualGUID(pdidInstance->guidProduct, SIDESTICK_GUID)
  ) {
    HRESULT hr = 0;
    hr &= pDI->CreateDevice(pdidInstance->guidInstance, &pSidestick, NULL);
    hr &= pSidestick->SetDataFormat(&c_dfDIJoystick2);
    hr &= pSidestick->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE);
    hr &= pSidestick->Acquire();

    if (FAILED(hr)) {
	  pSidestick = NULL;
	  FlagUp(&joystick_ecam_msg, SIDESTICK_INIT_FAULT);
    }
	else if(SUCCEEDED(hr))
      FlagDown(&joystick_ecam_msg, SIDESTICK_INIT_FAULT);
  }

  if (pSidestick != NULL && pThrust != NULL) return DIENUM_STOP;
  else return DIENUM_CONTINUE;
}

void InitializeJoysticks() {
  pThrust = NULL;
  pSidestick = NULL;

  HRESULT hr = pDI->EnumDevices(
    DI8DEVCLASS_GAMECTRL,
    EnumJoysticksCallback,
    NULL,
    DIEDFL_ATTACHEDONLY
  );

  if (FAILED(hr)) {
    FlagUp(&joystick_ecam_msg, DIRECT_INPUT_ENUM_FAULT);
    return;
  }

  if (pThrust == NULL)
    FlagUp(&joystick_ecam_msg, THRUST_NOT_FOUND);
  else
    FlagDown(&joystick_ecam_msg, THRUST_NOT_FOUND);

  if (pSidestick == NULL)
    FlagUp(&joystick_ecam_msg, SIDESTICK_NOT_FOUND);
  else
	FlagDown(&joystick_ecam_msg, SIDESTICK_NOT_FOUND);
}

void ReleaseDirectInput() {
  if (pDI) pDI->Release();
}