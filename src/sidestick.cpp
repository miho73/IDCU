#include "ECAM.h"

LPDIRECTINPUTDEVICE8 pSidestick = NULL;

void ProcessSidestickInput() {
  HRESULT hr = pSidestick->Poll();

  if (FAILED(hr)) {
    FlagUp(&joystick_ecam_msg, SIDESTICK_POLLING_FAULT);
    FlagUp(&joystick_ecam_msg, SIDESTICK_NOT_FOUND);
    pSidestick = NULL;
    return;
  }
  else
    FlagDown(&joystick_ecam_msg, SIDESTICK_POLLING_FAULT);

  DIJOYSTATE2 js;
  hr = pSidestick->GetDeviceState(sizeof(DIJOYSTATE2), &js);

  if (FAILED(hr)) {
    if (
      hr == DIERR_INPUTLOST ||
      hr == DIERR_NOTACQUIRED ||
      hr == DIERR_NOTINITIALIZED ||
      hr == DIERR_UNPLUGGED
      ) {
      pSidestick = NULL;
      FlagUp(&joystick_ecam_msg, SIDESTICK_NOT_FOUND);
      FlagUp(&joystick_ecam_msg, SIDESTICK_GET_FAULT);
    }
    return;
  }
  else
    FlagDown(&joystick_ecam_msg, SIDESTICK_GET_FAULT);
}