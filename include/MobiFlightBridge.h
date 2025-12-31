#pragma once

#include <Windows.h>
#include <queue>
#include <string>
#include <algorithm>
#include "ECAM.h"
#include "SimConnect.h"

using namespace std;

#define MF_CLIENT_DATA_COMMAND "MobiFlight.Command"
#define MF_IDCU_CLIENT_DATA_COMMAND "IDCU.Command"
#define MF_IDCU_CLIENT_DATA_LVARS  "IDCU.LVars"
#define MF_IDCU_CLIENT_DATA_RESPONSE "IDCU.Response"

#define MF_MSG_BLOCK_SIZE 1024

namespace A32NX_CMD {
  const string PREFIX = "MF.SimVars.Set.";

  const string E_GEAR_UP = "GEAR_UP";
  const string E_GEAR_DOWN = "GEAR_DOWN";
  const string L_PARK_BRK = "A32NX_PARK_BRAKE_LEVER_POS";

  const string E_GND_SPOILER_ARM = "SPOILERS_ARM_ON";
  const string E_GND_SPOILER_DISARM = "SPOILERS_ARM_OFF";
  const string E_FLAPS_SET_0 = "FLAPS_UP";
  const string E_FLAPS_SET_1 = "FLAPS_1";
  const string E_FLAPS_SET_2 = "FLAPS_2";
  const string E_FLAPS_SET_3 = "FLAPS_3";
  const string E_FLAPS_SET_4 = "FLAPS_DOWN";

  const string L_NO_SMOKING_LVR = "XMLVAR_SWITCH_OVHD_INTLT_NOSMOKING_POSITION";
  const string E_SEATBELT_LVR = "CABIN_SEATBELTS_ALERT_SWITCH_TOGGLE";
  const string E_ENG_MASTER_SW_ON = "FUELSYSTEM_VALVE_OPEN";
  const string E_ENG_MASTER_SW_OFF = "FUELSYSTEM_VALVE_CLOSE";

  const string L_MASTER_CAUTION_BTN = "A32NX_MASTER_CAUTION";
  const string L_MASTER_WARNING_BTN = "A32NX_MASTER_WARNING";

  const string A_MCDU_BRIGHTNESS = "A32NX_MCDU_L_BRIGHTNESS";

  const string E_BARO_INC_L = "A32NX.FCU_EFIS_L_BARO_INC";
  const string E_BARO_DEC_L = "A32NX.FCU_EFIS_L_BARO_DEC";
  const string E_BARO_PUSH_L = "A32NX.FCU_EFIS_L_BARO_PUSH";
  const string E_BARO_PULL_L = "A32NX.FCU_EFIS_L_BARO_PULL";
  const string E_BARO_UNIT_L = "A32NX_FCU_EFIS_L_BARO_IS_INHG";
  const string E_BARO_INC_R = "A32NX.FCU_EFIS_R_BARO_INC";
  const string E_BARO_DEC_R = "A32NX.FCU_EFIS_R_BARO_DEC";
  const string E_BARO_PUSH_R = "A32NX.FCU_EFIS_R_BARO_PUSH";
  const string E_BARO_PULL_R = "A32NX.FCU_EFIS_R_BARO_PULL";
  const string E_BARO_UNIT_R = "A32NX_FCU_EFIS_R_BARO_IS_INHG";

  const string L_ECAM_PAGE = "A32NX_ECAM_SD_CURRENT_PAGE_INDEX";

  const string E_FCU_SPD_PUSH = "A32NX.FCU_SPD_PUSH";
  const string E_FCU_SPD_PULL = "A32NX.FCU_SPD_PULL";

  const string E_FCU_HDG_PUSH = "A32NX.FCU_HDG_PUSH";
  const string E_FCU_HDG_PULL = "A32NX.FCU_HDG_PULL";

  const string E_FCU_ALT_PUSH = "A32NX.FCU_ALT_PUSH";
  const string E_FCU_ALT_PULL = "A32NX.FCU_ALT_PULL";

  const string E_FCU_LOC_BTN = "A32NX.FCU_LOC_PUSH";
  const string E_FCU_APPR_BTN = "A32NX.FCU_APPR_PUSH";

  const string E_FCU_LS_PUSH = "A32NX.FCU_EFIS_L_LS_PUSH";
  const string E_FCU_AP1_PUSH = "A32NX.FCU_AP_1_PUSH";
  const string E_FCU_AP2_PUSH = "A32NX.FCU_AP_2_PUSH";
  const string E_FCU_AP_DISC_PUSH = "A32NX.FCU_AP_DISCONNECT_PUSH";
  const string E_FCU_ATHR_PUSH = "A32NX.FCU_ATHR_PUSH";

  const string E_FCU_TRK_FPA_BTN = "A32NX.FCU_TRK_FPA_TOGGLE_PUSH";
  const string E_FCU_SPD_MACH_BTN = "A32NX.FCU_SPD_MACH_TOGGLE_PUSH";
  const string E_FCU_METRIC_ALT_BTN = "A32NX.FCU_METRIC_ALT_TOGGLE_PUSH";

  const string E_FCU_VS_PUSH = "A32NX.FCU_VS_PUSH";
  const string E_FCU_VS_PULL = "A32NX.FCU_VS_PULL";
  const string E_FCU_VS_SEL = "A32NX.FCU_VS_SET";

  const string E_FCU_ALT_INC = "A32NX.FCU_ALT_INC";
  const string E_FCU_ALT_DEC = "A32NX.FCU_ALT_DEC";
  const string E_FCU_ALT_INC_SET = "A32NX.FCU_ALT_INCREMENT_SET";

  const string L_TERR_L = "A32NX_EFIS_TERR_L_ACTIVE";
  const string L_TERR_R = "A32NX_EFIS_TERR_R_ACTIVE";

  const string L_ND_CSTR = "A32NX.FCU_EFIS_L_CSTR_PUSH";
  const string L_ND_WPT = "A32NX.FCU_EFIS_L_WPT_PUSH";
  const string L_ND_VORD = "A32NX.FCU_EFIS_L_VORD_PUSH";
  const string L_ND_ARPT = "A32NX.FCU_EFIS_L_ARPT_PUSH";

  const string L_ND_RANGE = "A32NX_FCU_EFIS_L_EFIS_RANGE";
  const string L_ND_MODE = "A32NX_FCU_EFIS_L_EFIS_MODE";

  const string E_DOME_LT = "CABIN_LIGHTS_SET";
  const string L_ANN_LT = "A32NX_OVHD_INTLT_ANN";
  const string L_STROBE_LT = "LIGHTING_STROBE_0";
  const string E_NAV_LT = "NAV_LIGHTS_SET";
  const string E_LOGO_LT = "LOGO_LIGHTS_SET";
  const string L_LDG_LT1 = "LIGHTING_LANDING_1";
  const string L_LDG_LT2 = "LIGHTING_LANDING_2";
  const string L_LDG_LT3 = "LIGHTING_LANDING_3";
  const string E_RWY_TURNOFF_LT_SET = "TAXI_LIGHTS_SET";
  const string E_BEACON_SET = "BEACON_LIGHTS_SET";

  const string E_LT_POTENTIOMETER = "LIGHT_POTENTIOMETER_SET";

  const string E_COM1_XFER_FREQ = "COM1_RADIO_SWAP";
  const string E_COM1_RADIO_SET = "COM_STBY_RADIO_SET_HZ";
  const string L_VHF_SEL = "A32NX_RMP_L_SELECTED_MODE";

  const string E_ELT_INC = "ELEV_TRIM_UP";
  const string E_ELT_DEC = "ELEV_TRIM_DN";
}

enum ECAM_PAGE_IDX {
  ENG,	BLEED,	PRESS,	ELEC, HYD,
  FUEL,	APU,	COND,	DOOR, WHEEL,
  FCTL,	STS,	CRUISE
};

enum DATA_DEFINE_ID {
  DEFINITION_MF_COMMAND,
  DEFINITION_MF_IDCU_COMMAND,
  DEFINITION_MF_IDCU_RESPONSE,
  DEFINITION_LVAR_GND_SPD_BRK_ARMED	  = 1000,
  DEFINITION_LVAR_FLAPS_HANDLE		  = 1001,
  DEFINITION_LVAR_ENG_MSTR1			  = 1002,
  DEFINITION_LVAR_ENG_MSTR2			  = 1003,
  DEFINITION_LVAR_MCDU_BRIGHTNESS	  = 1004,
  DEFINITION_LVAR_SEATBEALT_SIGN	  = 1005,
  DEFINITION_LVAR_BARO_MODE			  = 1006,
  DEFINITION_LVAR_TERR_L			  = 1007,
  DEFINITION_LVAR_TERR_R			  = 1008,
  DEFINITION_LVAR_ND_MODE			  = 1009,
  DEFINITION_LVAR_ND_RANGE			  = 1010,
  DEFINITION_LVAR_INTEG_ANN_LT		  = 1011,
  DEFINITION_LVAR_STROBE_LT			  = 1012,
  DEFINITION_LVAR_LDG_LT			  = 1013,
  DEFINITION_LVAR_NOSE_LT			  = 1014,
  DEFINITION_LVAR_COM1_FREQ		      = 1015,
};

enum CLIENT_DATA_ID {
  CLIENT_DATA_MF_COMMAND,
  CLIENT_DATA_MF_IDCU_COMMAND,
  CLIENT_DATA_MF_IDCU_LVARS,
  CLIENT_DATA_MF_IDCU_RESPONSE,
};

enum CLIENT_DATA_REQUEST_ID {
  REQUEST_MF_RESPONSE,
  REQUEST_MF_SPD_BRK,
  REQUEST_MF_FLAPS_HANDLE,
  REQUEST_MF_ENG_MSTR1,
  REQUEST_MF_ENG_MSTR2,
  REQUEST_MF_MCDU_BRIGHTNESS,
  REQUEST_MF_SEATBEALT_SIGN,
  REQUEST_MF_BARO_MODE,
  REQUEST_MF_TERR_L,
  REQUEST_MF_TERR_R,
  REQUEST_MF_ND_MODE,
  REQUEST_MF_ND_RANGE,
  REQUEST_MF_INTEG_ANN_LT,
  REQUEST_MF_STROBE_LT,
  REQUEST_MF_LDG_LT,
  REQUEST_MF_NOSE_LT,
  REQUEST_MF_COM1_FREQ,
};

struct MFCommandStruct {
  char command[1024];
  bool execLock;
  int hold = 0;
};

struct ACFT_STATUS {
  int groundSpoilerArmed;
  int flapsHandlePos;
  int engMasterState1;
  int engMasterState2;
  float mcduBrightness;
  int seatbeltSign;
  int baroMode;
  int vsKnob = 0;
  int terrOnL = 0;
  int terrOnR = 0;
  int ndMode = 3;
  int ndRange = 0;
  int integAnnLt = 1;
  int strobeLt = 0;
  int ldgLT = 2;
  int noseLT = 2;
  int domeLT = 1;

  int ecamBRT = 70;
  int pfdBRT = 70;
  int fcuLCDBRT = 70;
  int pedFldLT = 0;
  int captFldLT = 0;
  int integLT = 70;

  int com1Coarse = 0;
  int com1Fine = 0;
};

extern HANDLE hSimConnect;
extern ACFT_STATUS acft_status;

void DispatchSimConnectMessage();
void SendMobiFlightCommand(const string& command, bool lock, int hold = 0);

void SetLVAR(const string& lvarName, int value);
void SetLVAR(const string& lvarName, float value);
void SetAVAR(const string& avarName, int value);
void PressLVAR(const string& lvarName);
void FireEVT(const string& keyEvent);
void FireEVT(const string& keyEvent, int value);
void FireEVT_f(const string& keyEvent, float value);
void FireEVT(const string& keyEvent, int v1, int v2);
void ToggleEVT(const string& avarName);
void ToggleEVT(const string& keyEvent, int v1);
