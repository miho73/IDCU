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
void RTY1Handler();
void RTY2Handler();
void RTY3Handler();
void RTY4Handler();

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
	RTY1 = js.lZ;
	RTY1Handler();
  }
  if (RTY2 != js.lRz) {
    RTY2 = js.lRz;
    RTY2Handler();
  }
  if (RTY3 != js.rglSlider[0]) {
    RTY3 = js.rglSlider[0];
    RTY3Handler();
  }
  if (RTY4 != js.rglSlider[1]) {
    RTY4 = js.rglSlider[1];
    RTY4Handler();
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
  if (mode == 1) { // SET FLOOD LT
	
  }
}

void RTY4Handler() {
  if (mode == 1) { // SET INTEG LT
    
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
        FireEVT(A32NX_CMD::E_GEAR_UP);
        break;
      }
      case SW2: {
        FireEVT(A32NX_CMD::E_GEAR_DOWN);
        break;
      }
      case SW3: {
		if (acft_status.groundSpoilerArmed == 0)
          FireEVT(A32NX_CMD::E_GND_SPOILER_ARM);
        break;
      }
      case SW4: {
        if (acft_status.groundSpoilerArmed == 1)
          FireEVT(A32NX_CMD::E_GND_SPOILER_DISARM);
        break;
      }
      case SW5: {
        if (acft_status.flapsHandlePos == 1)
          FireEVT(A32NX_CMD::E_FLAPS_SET_0);
        else if (acft_status.flapsHandlePos == 2)
          FireEVT(A32NX_CMD::E_FLAPS_SET_1);
        else if (acft_status.flapsHandlePos == 3)
          FireEVT(A32NX_CMD::E_FLAPS_SET_2);
        else if (acft_status.flapsHandlePos == 4)
          FireEVT(A32NX_CMD::E_FLAPS_SET_3);
        break;
      }
      case SW6: {
        if (acft_status.flapsHandlePos == 0)
          FireEVT(A32NX_CMD::E_FLAPS_SET_1);
        else if (acft_status.flapsHandlePos == 1)
          FireEVT(A32NX_CMD::E_FLAPS_SET_2);
        else if (acft_status.flapsHandlePos == 2)
          FireEVT(A32NX_CMD::E_FLAPS_SET_3);
        else if (acft_status.flapsHandlePos == 3)
          FireEVT(A32NX_CMD::E_FLAPS_SET_4);
        break;
      }
      case TGL1UP: {
		SetLVAR(A32NX_CMD::L_PARK_BRK, 0);
        break;
      }
      case TGL1DN: {
        SetLVAR(A32NX_CMD::L_PARK_BRK, 1);
        break;
      }
      case TGL2UP: {
        if (acft_status.seatbeltSign == 0)
          ToggleEVT(A32NX_CMD::E_SEATBELT_LVR);
        break;
      }
      case TGL2DN: {
        if (acft_status.seatbeltSign == 1)
          ToggleEVT(A32NX_CMD::E_SEATBELT_LVR);
        break;
      }
      case TGL3UP: {
		SetLVAR(A32NX_CMD::L_NO_SMOKING_LVR, 0);
        break;
      }
      case TGL3DN: {
        SetLVAR(A32NX_CMD::L_NO_SMOKING_LVR, 2);
        break;
      }
      case TGL4UP: {
        if (acft_status.engMasterState1 == 0) FireEVT(A32NX_CMD::E_ENG_MASTER_SW_ON, 1);
        else FireEVT(A32NX_CMD::E_ENG_MASTER_SW_OFF, 1);
        break;
      }
      case TGL4DN: {
        if (acft_status.engMasterState2 == 0) FireEVT(A32NX_CMD::E_ENG_MASTER_SW_ON, 2);
        else FireEVT(A32NX_CMD::E_ENG_MASTER_SW_OFF, 2);
        break;
      }
      case BTN_H: {
        SetLVAR(A32NX_CMD::L_MASTER_CAUTION_BTN, 0);
        break;
      }
      case BTN_I: {
        SetLVAR(A32NX_CMD::L_MASTER_WARNING_BTN, 0);
        break;
      }
      case BTN_E: {
        if (acft_status.baroMode == 0) {
          FireEVT(A32NX_CMD::E_BARO_PUSH_L);
          FireEVT(A32NX_CMD::E_BARO_PUSH_R);
        }
        else {
          FireEVT(A32NX_CMD::E_BARO_PULL_L);
          FireEVT(A32NX_CMD::E_BARO_PULL_R);
        }
        break;
      }
      case BTN_F: {
        // NOT SET
        break;
      }
      case BTN_G: {
        // NOT SET
        break;
      }
      case SLD: {
        if (acft_status.baroMode == 1) {
          SetLVAR(A32NX_CMD::E_BARO_UNIT_L, 1);
          SetLVAR(A32NX_CMD::E_BARO_UNIT_R, 1);
        }
        else if (acft_status.baroMode == 2) {
          SetLVAR(A32NX_CMD::E_BARO_UNIT_L, 0);
          SetLVAR(A32NX_CMD::E_BARO_UNIT_R, 0);
        }
		break;
      }
      case LEFT_KNOB_FWD: {
        ToggleEVT(A32NX_CMD::E_BARO_INC_L);
        ToggleEVT(A32NX_CMD::E_BARO_INC_R);
        break;
      }
      case LEFT_KNOB_AFT: {
        ToggleEVT(A32NX_CMD::E_BARO_DEC_L);
        ToggleEVT(A32NX_CMD::E_BARO_DEC_R);
        break;
      }
      case LEVER_K1_UP: {
		if (acft_status.mcduBrightness <= 7.5f)
          SetLVAR(A32NX_CMD::A_MCDU_BRIGHTNESS, acft_status.mcduBrightness + 0.5f);
        break;
      }
      case LEVER_K1_DN: {
        if (acft_status.mcduBrightness >= 0.0f)
          SetLVAR(A32NX_CMD::A_MCDU_BRIGHTNESS, acft_status.mcduBrightness - 0.5f);
        break;
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
