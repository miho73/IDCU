#include "IDCU.h"

enum ThrustButtons {
  SW1 = 6,
  SW2 = 7,
  SW3 = 8,
  SW4 = 9,
  SW5 = 10,
  SW6 = 11,
  TGL1UP = 12,
  TGL1DN = 13,
  TGL2UP = 14,
  TGL2DN = 15,
  TGL3UP = 16,
  TGL3DN = 17,
  TGL4UP = 18,
  TGL4DN = 19,
  BTN_E = 1,
  BTN_F = 2,
  BTN_G = 3,
  SLD = 33,
  BTN_I = 4,
  BTN_H = 5,
  LEVER_K1_UP = 28,
  LEVER_K1_DN = 29,
  LEFT_KNOB_FWD = 30,
  LEFT_KNOB_AFT = 31,
  LEVER_H3_UP = 20,
  LEVER_H3_FWD = 21,
  LEVER_H3_DN = 22,
  LEVER_H3_AFT = 23,
  LEVER_H4_UP = 24,
  LEVER_H4_FWD = 25,
  LEVER_H4_DN = 26,
  LEVER_H4_AFT = 27,
  MODE1 = 34,
  MODE2 = 35,
  MODE3 = 36,
};

LPDIRECTINPUTDEVICE8 pThrust = NULL;

struct ButtonState128 {
  uint32_t chunks[4];

  ButtonState128() {
    std::memset(chunks, 0, sizeof(chunks));
  }
};

bool isThrustLeverInitialized = false;
ButtonState128 prevButtonState;
LONG LEFT_THROTTLE, RIGHT_THROTTLE, RTY1, RTY2, RTY3, RTY4, STKx, STKy;

void PressHandler(int btnID);

void ProcessThrustLeverInput() {
  HRESULT hr = pThrust->Poll();
  if (FAILED(hr)) {
	hr = pThrust->Acquire();
	if (hr == DIERR_UNPLUGGED) {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_DATA_FAULT);
	}
	else if (hr == DIERR_INPUTLOST || hr == DIERR_NOTACQUIRED) {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_LINK_FAULT);
	}
	else {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_FAULT);
	}
	return;
  }

  DIJOYSTATE2 js;
  hr = pThrust->GetDeviceState(sizeof(DIJOYSTATE2), &js);
  if (FAILED(hr)) {
	HRESULT hr = pThrust->Acquire();

	if (hr == DIERR_UNPLUGGED) {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_DATA_FAULT);
	}
	else if (hr == DIERR_NOTACQUIRED || hr == DIERR_INPUTLOST) {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_LINK_FAULT);
	}
	else {
	  BooleanFalse(&thrustlever_avail);
	  FlagUp(&thrust_lever_ecam_msg, THRUST_POLLING_FAULT);
	}
	return;
  }

  ButtonState128 currentButtonState;
  for (int i = 0; i < 128; ++i)
	if (js.rgbButtons[i] & 0x80)
	  currentButtonState.chunks[i / 32] |= (1 << (i % 32));

  if (!isThrustLeverInitialized) {
	isThrustLeverInitialized = true;

	for (int i = 0; i < 4; i++)
	  prevButtonState.chunks[i] = currentButtonState.chunks[i];

	//LEFT_THROTTLE = js.lX;
	//RIGHT_THROTTLE = js.lY;
	RTY1 = js.lZ;
	RTY2 = js.lRz;
	RTY3 = js.rglSlider[0];
	RTY4 = js.rglSlider[1];
	//STKx = js.lRx;
	//STKy = js.lRy;

	return;
  }

  for (int i = 0; i < 4; i++) {
	uint32_t changes = currentButtonState.chunks[i] ^ prevButtonState.chunks[i];

	if (changes != 0) {
	  for (int bit = 0; bit < 32; bit++) {
	    uint32_t mask = (1 << bit);
	    if (changes & mask) {
	  	  int btnID = i * 32 + bit + 1;

	  	  if (currentButtonState.chunks[i] & mask) PressHandler(btnID);
	    }
	  }
	}

	prevButtonState.chunks[i] = currentButtonState.chunks[i];
  }

  if (RTY1 != js.lZ) {
	
  }

  BooleanTrue(&thrustlever_avail);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_DATA_FAULT);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_LINK_FAULT);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_FAULT);
}

void RTY1Handler() {
  if (mode == 1) {
	// PITCH TRIM WHEEL
  }
  else if (mode == 2) {
	// V/S KNOB
  }
}

void RTY2Handler() {
  if (mode == 1) {
	// RUDDER TRIM WHEEL
  }
}

void RTY3Handler() {
  if (mode == 1) {
	// FLOOD LT KNOB
  }
}

void RTY4Handler() {
  if (mode == 1) {
	// INTEG LT KNOB
  }
}

void PressHandler(int btnID) {
  switch (btnID) {
	case MODE1:
	  mode = 1;
	  ECAMPrint();
	  return;
	case MODE2:
	  mode = 2;
	  ECAMPrint();
	  return;
	case MODE3:
	  mode = 3;
	  ECAMPrint();
	  return;
  }

  if (mode == 1) {
	switch (btnID) {
	  case SW1: {
		// GEAR UP
		break;
	  }
	  case SW2: {
		// GEAR DOWN
		break;
	  }
	  case SW3: {
		// SPD BRK DETENT UP
		break;
	  }
	  case SW4: {
		// SPD BRK DETENT DN
		break;
	  }
	  case SW5: {
		// FLAPS DETENT UP
		break;
	  }
	  case SW6: {
		// FLAPS DETENT DN
		break;
	  }
	  case TGL1UP: {
		// SET PARK BRK
		break;
	  }
	  case TGL1DN: {
		// RELEASE PARK BRK
		break;
	  }
	  case TGL2UP: {
		// SEATBELT SIGN ON
		break;
	  }
	  case TGL2DN: {
		// SEATBELT SIGN OFF
		break;
	  }
	  case TGL3UP: {
		// NO SMOKING SIGN ON
		break;
	  }
	  case TGL3DN: {
		// NO SMOKING SIGN OFF
		break;
	  }
	  case TGL4UP: {
		// TOGGLE ENG MASTER 1
		break;
	  }
	  case TGL4DN: {
		// TOGGLE ENG MASTER 2
		break;
	  }
	  case BTN_H: {
		// SHUT MASTER CAUT
		break;
	  }
	  case BTN_I: {
		// SHUT MASTER WARN
		break;
	  }
	  case BTN_E: {
		// T.O. CONFIG
		break;
	  }
	  case BTN_F: {
		// CENTER PITCH TRIM 
		break;
	  }
	  case BTN_G: {
		// CENTER RUDDER TRIM
		break;
	  }
	  case LEFT_KNOB_FWD: {
		// PFD / ND / ECAM BRT
		break;
	  }
	  case LEFT_KNOB_AFT: {
		// PFD / ND / ECAM DIM
		break;
	  }
	  case LEVER_K1_UP: {
		// MCDU BRT
		break;
	  }
	  case LEVER_K1_DN: {
		// MCDU DIM
	  }
	}
  }
  else if (mode == 2) {
	switch (btnID) {
	  case SW1: {
		// PUSH SPD
		break;
	  }
	  case SW2: {
		// PULL SPD
		break;
	  }
	  case SW3: {
		// PUSH HDG
		break;
	  }
	  case SW4: {
		// PULL HDG
		break;
	  }
	  case SW5: {
		// PUSH ALT
		break;
	  }
	  case SW6: {
		// PULL ALT
		break;
	  }
	  case TGL1UP: {
		// TOGGLE AP1
		break;
	  }
	  case TGL1DN: {
		// TOGGLE AP2
		break;
	  }
	  case TGL2UP: {
		// A/THR ENGAGE
		break;
	  }
	  case TGL2DN: {
		// A/THR DISENGAGE
		break;
	  }
	  case TGL3UP: {
		// PUSH QNH
		break;
	  }
	  case TGL3DN: {
		// PULL QNH
		break;
	  }
	  case TGL4UP: {
		// QNH inHg
		break;
	  }
	  case TGL4DN: {
		// QNH hPa
		break;
	  }
	  case BTN_H: {
		// 
		break;
	  }
	  case BTN_I: {
		// 
		break;
	  }
	  case BTN_E: {
		// 
		break;
	  }
	  case BTN_F: {
		// LEVEL OFF
		break;
	  }
	  case BTN_G: {
		// 
		break;
	  }
	  case LEFT_KNOB_FWD: {
		//
		break;
	  }
	  case LEFT_KNOB_AFT: {
		//
		break;
	  }
	  case LEVER_K1_UP: {
		//
		break;
	  }
	  case LEVER_K1_DN: {
		//
	  }
	}
  }
  else if (mode == 3) {

  }
}
