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
}

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
  REQUEST_MF_BARO_MODE
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
void FireEVT(const string& keyEvent, int v1, int v2);
void ToggleEVT(const string& avarName);
