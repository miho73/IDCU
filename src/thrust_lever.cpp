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
  H3_FWD = 21,
  H3_UP = 20,
  H3_DN = 22,
  H3_AFT = 23,
  H4_FWD = 25,
  H4_UP = 24,
  H4_DN = 26,
  H4_AFT = 27
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
LONG RTY_LK_1 = -123, RTY_LK_2 = -123, RTY_LK_3 = -123, RTY_LK_4 = -123;

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
	if (RTY_LK_1 == -123) RTY1Handler();
    else if (abs(RTY_LK_1 - RTY1) > 0xFF) {
	  RTY_LK_1 = -123;
      RTY1Handler();
    }
  }
  if (RTY2 != js.lRz) {
    RTY2 = js.lRz;
    if (RTY_LK_2 == -123) RTY2Handler();
    else if (abs(RTY_LK_2 - RTY2) > 0xFF) {
      RTY_LK_2 = -123;
      RTY2Handler();
    }
  }
  if (RTY3 != js.rglSlider[0]) {
    RTY3 = js.rglSlider[0];
    if (RTY_LK_3 == -123) RTY3Handler();
    else if (abs(RTY_LK_3 - RTY3) > 0xFF) {
      RTY_LK_3 = -123;
      RTY3Handler();
    }
  }
  if (RTY4 != js.rglSlider[1]) {
    RTY4 = js.rglSlider[1];
    if (RTY_LK_4 == -123) RTY4Handler();
    else if (abs(RTY_LK_4 - RTY4) > 0xFF) {
      RTY_LK_4 = -123;
      RTY4Handler();
    }
  }

  BooleanTrue(&thrustlever_avail);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_DATA_FAULT);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_LINK_FAULT);
  FlagDown(&thrust_lever_ecam_msg, THRUST_POLLING_FAULT);
}

void RTY1Handler() {
  if (mode == 1) {
	int coarse = RTY1 * 18 / 65536 + 118;
	if (coarse < 118) coarse = 118;
	if (coarse > 136) coarse = 136;

	if (acft_status.com1Coarse != coarse) {
      acft_status.com1Coarse = coarse;
      FireEVT(A32NX_CMD::E_COM1_RADIO_SET, acft_status.com1Coarse * 1e6 + acft_status.com1Fine * 1e3);
    }
  }
  else if (mode == 2) {
    // NOT SET
  }
  else if (mode == 3) {
    // NOT SET
  }
}

void RTY2Handler() {
  if (mode == 1) {
    int fine = RTY2 * 199 / 65536;
    fine *= 5;
    if (fine < 0) fine = 0;
    if (fine > 995) fine = 995;

    if (acft_status.com1Fine != fine) {
      acft_status.com1Fine = fine;
      FireEVT(A32NX_CMD::E_COM1_RADIO_SET, acft_status.com1Coarse * 1e6 + acft_status.com1Fine * 1e3);
    }
  }
  else if(mode == 2) {
	int posH = (RTY2 - 32768) * 60 / 32768;
    int pos = posH * 100;
	if (acft_status.vsKnob != pos) {
      acft_status.vsKnob = pos;
	  FireEVT(A32NX_CMD::E_FCU_VS_SEL, pos);
    }
  }
  else if(mode == 3) {
    // NOT SET
  }
}

void RTY3Handler() {
  if (mode == 1) {
    int pos = max(RTY3 * 100 / 65536 - 7, 0);
    if (acft_status.captFldLT != pos) {
	  acft_status.captFldLT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 83);
    }
  }
  else if (mode == 2) {
    int pos = min(RTY3 * 100 / 65536 + 2, 100);
    if (abs(acft_status.integLT - pos) >= 3 || pos == 0 || pos == 100) {
	  acft_status.integLT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 84);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 85);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 86);
    }
  }
  else if (mode == 3) {
	int pos = max(RTY3 * 100 / 65536, 1);
    if (abs(acft_status.ecamBRT - pos) >= 4 || pos == 1) {
      acft_status.ecamBRT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 92);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 93);
    }
  }
}

void RTY4Handler() {
  if (mode == 1) {
    int pos = max(RTY4 * 100 / 65536 - 2, 0);
    if (acft_status.fcuLCDBRT != pos) {
	  acft_status.fcuLCDBRT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 76);
    }
  }
  else if(mode == 2) {
    int pos = min(RTY4 * 100 / 65536 + 2, 100);
    if (acft_status.fcuLCDBRT != pos) {
	  acft_status.fcuLCDBRT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 87);
    }
  }
  else if (mode == 3) {
    int pos = max(RTY4 * 100 / 65536, 1);
    if (abs(acft_status.pfdBRT - pos) >= 6 || pos == 1) {
      acft_status.pfdBRT = pos;
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 88);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 89);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 90);
      FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, pos, 91);
    }
  }
}

void PressHandler(int btnID) {
  switch (btnID) {  
    case MODE1:
      mode = 1;
	  RTY_LK_1 = RTY1;
	  RTY_LK_2 = RTY2;
	  RTY_LK_3 = RTY3;
      ECAMPrint();
      return;
    case MODE2:
      mode = 2;
      RTY_LK_1 = RTY1;
      RTY_LK_2 = RTY2;
      RTY_LK_3 = RTY3;
      ECAMPrint();
      return;
    case MODE3:
      mode = 3;
      RTY_LK_1 = RTY1;
      RTY_LK_2 = RTY2;
      RTY_LK_3 = RTY3;
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
		ToggleEVT(A32NX_CMD::E_COM1_XFER_FREQ);
        break;
      }
      case BTN_G: {
        ToggleEVT(A32NX_CMD::E_COM1_XFER_FREQ);
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
      case H3_FWD: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, ENG);
        break;
      }
      case H3_UP: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, APU);
        break;
      }
      case H3_AFT: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, COND);
        break;
      }
      case H3_DN: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, BLEED);
        break;
      }
      case H4_FWD: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, PRESS);
        break;
      }
      case H4_UP: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, ELEC);
        break;
      }
      case H4_AFT: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, FUEL);
        break;
      }
      case H4_DN: {
        SetLVAR(A32NX_CMD::L_ECAM_PAGE, HYD);
        break;
      }
    }
  }
  else if (mode == 2) {
    switch (btnID) {
      case SW1: {
        ToggleEVT(A32NX_CMD::E_FCU_SPD_PUSH);
        break;
      }
      case SW2: {
        ToggleEVT(A32NX_CMD::E_FCU_SPD_PULL);
        break;
      }
      case SW3: {
        ToggleEVT(A32NX_CMD::E_FCU_HDG_PULL);
        break;
      }
      case SW4: {
        ToggleEVT(A32NX_CMD::E_FCU_HDG_PUSH);
        break;
      }
      case SW5: {
        ToggleEVT(A32NX_CMD::E_FCU_ALT_PUSH);
        break;
      }
      case SW6: {
        ToggleEVT(A32NX_CMD::E_FCU_ALT_PULL);
        break;
      }
      case TGL1UP: {
        ToggleEVT(A32NX_CMD::E_FCU_AP1_PUSH);
        break;
      }
      case TGL1DN: {
        ToggleEVT(A32NX_CMD::E_FCU_AP2_PUSH);
        break;
      }
      case TGL2UP: {
        ToggleEVT(A32NX_CMD::E_FCU_TRK_FPA_BTN);
        break;
      }
      case TGL2DN: {
        ToggleEVT(A32NX_CMD::E_FCU_ATHR_PUSH);
        break;
      }
      case TGL3UP: {
        ToggleEVT(A32NX_CMD::E_FCU_LOC_BTN);
        break;
      }
      case TGL3DN: {
        ToggleEVT(A32NX_CMD::E_FCU_APPR_BTN);
        break;
      }
      case TGL4UP: {
        ToggleEVT(A32NX_CMD::E_FCU_SPD_MACH_BTN);
        break;
      }
      case TGL4DN: {
        ToggleEVT(A32NX_CMD::E_FCU_METRIC_ALT_BTN);
        break;
      }
      case BTN_H: {
        if (acft_status.terrOnR) SetLVAR(A32NX_CMD::L_TERR_R, 0);
        else SetLVAR(A32NX_CMD::L_TERR_R, 1);
        break;
      }
      case BTN_I: {
        if(acft_status.terrOnL) SetLVAR(A32NX_CMD::L_TERR_L, 0);
		else SetLVAR(A32NX_CMD::L_TERR_L, 1);
        break;
      }
      case BTN_E: {
		FireEVT(A32NX_CMD::E_FCU_VS_PULL);
        break;
      }
      case BTN_F: {
        // NOT SET
        break;
      }
      case BTN_G: {
		FireEVT(A32NX_CMD::E_FCU_VS_PUSH);
        break;
      }
      case SLD: {
		FireEVT(A32NX_CMD::E_FCU_LS_PUSH);
        break;
      }
      case LEFT_KNOB_FWD: {
        ToggleEVT(A32NX_CMD::E_FCU_ALT_INC, 0);
        break;
      }
      case LEFT_KNOB_AFT: {
        ToggleEVT(A32NX_CMD::E_FCU_ALT_DEC, 0);
        break;
      }
      case LEVER_K1_UP: {
		FireEVT(A32NX_CMD::E_FCU_ALT_INC_SET, 1000);
        break;
      }
      case LEVER_K1_DN: {
        FireEVT(A32NX_CMD::E_FCU_ALT_INC_SET, 100);
        break;
      }
      case H3_FWD: {
        FireEVT(A32NX_CMD::L_ND_CSTR);
        break;
      }
      case H3_UP: {
        FireEVT(A32NX_CMD::L_ND_WPT);
        break;
      }
      case H3_AFT: {
        FireEVT(A32NX_CMD::L_ND_VORD);
        break;
      }
      case H3_DN: {
        FireEVT(A32NX_CMD::L_ND_ARPT);
        break;
      }
      case H4_FWD: {
        if (acft_status.ndMode > 0)
		  SetLVAR(A32NX_CMD::L_ND_MODE, acft_status.ndMode - 1);
        break;
      }
      case H4_UP: {
        if(acft_status.ndRange > 0)
		  SetLVAR(A32NX_CMD::L_ND_RANGE, acft_status.ndRange - 1);
        break;
      }
      case H4_AFT: {
        if (acft_status.ndMode < 4)
          SetLVAR(A32NX_CMD::L_ND_MODE, acft_status.ndMode + 1);
        break;
      }
      case H4_DN: {
        if (acft_status.ndRange < 5)
		  SetLVAR(A32NX_CMD::L_ND_RANGE, acft_status.ndRange + 1);
        break;
      }
    }
  }
  else if (mode == 3) {
    switch (btnID) {
      case SW1: {
        if(acft_status.integAnnLt > 0)
          SetLVAR(A32NX_CMD::L_ANN_LT, acft_status.integAnnLt - 1);
        break;
      }
      case SW2: {
		if (acft_status.integAnnLt < 2)
          SetLVAR(A32NX_CMD::L_ANN_LT, acft_status.integAnnLt + 1);
        break;
      }
      case SW3: {
		if (acft_status.strobeLt > 0)
          SetLVAR(A32NX_CMD::L_STROBE_LT, acft_status.strobeLt - 1);
        break;
      }
      case SW4: {
		if (acft_status.strobeLt < 2)
          SetLVAR(A32NX_CMD::L_STROBE_LT, acft_status.strobeLt + 1);
        break;
      }
      case SW5: {
        FireEVT(A32NX_CMD::E_LOGO_LT, 1);
		FireEVT(A32NX_CMD::E_NAV_LT, 1);
        break;
      }
      case SW6: {
        FireEVT(A32NX_CMD::E_LOGO_LT, 0);
        FireEVT(A32NX_CMD::E_NAV_LT, 0);
        break;
      }
      case TGL1UP: {
        if (acft_status.ldgLT > 0) {
		  int nPos = acft_status.ldgLT - 1;
          SetLVAR(A32NX_CMD::L_LDG_LT2, nPos);
          SetLVAR(A32NX_CMD::L_LDG_LT3, nPos);
        }
        break;
      }
      case TGL1DN: {
        if (acft_status.ldgLT < 2) {
		  int nPos = acft_status.ldgLT + 1;
          SetLVAR(A32NX_CMD::L_LDG_LT2, nPos);
          SetLVAR(A32NX_CMD::L_LDG_LT3, nPos);
        }
        break;
      }
      case TGL2UP: {
        if(acft_status.noseLT > 0)
		  SetLVAR(A32NX_CMD::L_LDG_LT1, acft_status.noseLT - 1);
        break;
      }
      case TGL2DN: {
		if (acft_status.noseLT < 2)
		  SetLVAR(A32NX_CMD::L_LDG_LT1, acft_status.noseLT + 1);
        break;
      }
      case TGL3UP: {
        FireEVT(A32NX_CMD::E_RWY_TURNOFF_LT_SET, 2, 1);
        break;
      }
      case TGL3DN: {
        FireEVT(A32NX_CMD::E_RWY_TURNOFF_LT_SET, 2, 0);
        break;
      }
      case TGL4UP: {
        FireEVT(A32NX_CMD::E_BEACON_SET, 0, 1);
        break;
      }
      case TGL4DN: {
        FireEVT(A32NX_CMD::E_BEACON_SET, 0, 0);
        break;
      }
      case BTN_H: {
        SetLVAR(A32NX_CMD::L_VHF_SEL, 2);
        break;
      }
      case BTN_I: {
		FireEVT(A32NX_CMD::L_VHF_SEL, 0);
        break;
      }
      case BTN_E: {
        if (acft_status.domeLT == 0) {
          FireEVT(A32NX_CMD::E_DOME_LT, 1);
          FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, 20, 7);
          acft_status.domeLT = 1;
        }
        else if (acft_status.domeLT == 1) {
          FireEVT(A32NX_CMD::E_DOME_LT, 1);
          FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, 100, 7);
          acft_status.domeLT = 2;
        }
        else if (acft_status.domeLT == 2) {
          FireEVT(A32NX_CMD::E_DOME_LT, 0);
          FireEVT(A32NX_CMD::E_LT_POTENTIOMETER, 0, 7);
          acft_status.domeLT = 0;
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
        // NOT SET
        break;
      }
      case LEFT_KNOB_FWD: {
        ToggleEVT(A32NX_CMD::E_ELT_INC);
        break;
      }
      case LEFT_KNOB_AFT: {
        ToggleEVT(A32NX_CMD::E_ELT_DEC);
        break;
      }
      case LEVER_K1_UP: {
		// NOT SET
        break;
      }
      case LEVER_K1_DN: {
		// NOT SET
        break;
      }
      case H3_FWD: {
        // NOT SET
        break;
      }
      case H3_UP: {
        // NOT SET
        break;
      }
      case H3_AFT: {
        // NOT SET
        break;
      }
      case H3_DN: {
        // NOT SET
        break;
      }
      case H4_FWD: {
        // NOT SET
        break;
      }
      case H4_UP: {
        // NOT SET
        break;
      }
      case H4_AFT: {
        // NOT SET
        break;
      }
      case H4_DN: {
        // NOT SET
        break;
      }
    }
  }
}
