#include "joystick.h"

short mode = -1;

bool thrustlever_exists = false;
bool sidestick_exists = false;

GUID X56_THRUSTLEVER_ID;
GUID X56_SIDESTICK_ID;

LPDIRECTINPUT8 pDI = NULL;

void FindJoysticks();
void JoystickMessageLoop();

void InitializeJoysticks() {
  ECAMAmber("DIR INPUT INIT", "IN PROG", false);
  HRESULT hr = DirectInput8Create(
    GetModuleHandle(NULL),
    DIRECTINPUT_VERSION,
    IID_IDirectInput8,
    (VOID**)&pDI,
    NULL
  );

  if (FAILED(hr)) {
    ECAMRed("DIR INPUT INIT", "FAULT");

    MEMORed("\nDIR INPUT FAULT");
    exit(-1);
    return;
  }
  ECAMGreen("DIR INPUT INIT", "COMPLETE");

  FindJoysticks();
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
  ECAMAmber("USB SCAN", "IN PROG", false);

  HRESULT hr = pDI->EnumDevices(
    DI8DEVCLASS_GAMECTRL,
    EnumJoysticksCallback,
    NULL,
    DIEDFL_ATTACHEDONLY
  );

  if (FAILED(hr)) {
    ECAMAmber("USB SCAN", "FAULT");
    MEMORed("\nINPUT SYS LOOKUP FAULT");
    exit(-1);
  }
  else ECAMGreen("USB SCAN", "COMPLETE");

  if (sidestick_exists) {
    try {
      if (FAILED(pDI->CreateDevice(SIDESTICK_GUID, &pSidestick, NULL))) throw 1;
      if (FAILED(pSidestick->SetDataFormat(&c_dfDIJoystick2))) throw 2;
      if (FAILED(pSidestick->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) throw 3;
      if (FAILED(pSidestick->Acquire())) throw 4;

      ECAMGreen("SIDESTICK", "AVAIL");
    }
    catch(int e) {
      sidestick_exists = false;
      ECAMAmber("SIDESTICK", "INOP");
    }
  }
  else ECAMAmber("SIDESTICK", "MISSING");
  if (thrustlever_exists) {
    try {
      if (FAILED(pDI->CreateDevice(THRUSTLEVER_GUID, &pThrust, NULL))) throw 1;
      if (FAILED(pThrust->SetDataFormat(&c_dfDIJoystick2))) throw 2;
      if (FAILED(pThrust->SetCooperativeLevel(GetConsoleWindow(), DISCL_BACKGROUND | DISCL_NONEXCLUSIVE))) throw 3;
      if (FAILED(pThrust->Acquire())) throw 4;

      ECAMGreen("THR LVR", "AVAIL");
    }
    catch (int e) {
      sidestick_exists = false;
      ECAMAmber("THR LVR", "INOP");
    }
  }
  else ECAMAmber("THR LVR", "MISSING");

  if (!sidestick_exists || !thrustlever_exists) {
	MEMORed("\nINPUT SYS INIT FAULT");
	ECAMBlue("-DEVICE", "CHECK");
	ECAMBlue("-CONNECTION", "VERIFY");
	exit(-1);
  }

  InitThrust();

  JoystickMessageLoop();

  if (pDI) pDI->Release();
}

void JoystickMessageLoop() {
  while (true) {
    ReadThrustLeverInput();
    Sleep(30);
  }
}
