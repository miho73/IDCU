#include "MobiFlightBridge.h"

extern HANDLE hSimConnect = NULL;

HRESULT InitSimConnect();
void FireMFCommand();
void CALLBACK IDCUDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
void ProcessMFResposneClientData(SIMCONNECT_RECV* pData);
void updateACFTStatus(DATA_DEFINE_ID defID, float val);

ULONGLONG last_reconnect_at = 0;  // 마지막 재연결 시도 시점

queue<MFCommandStruct> mf_command_queue;
bool mf_exec_lock = false;

ACFT_STATUS acft_status;

const int SIZE_LVAR = sizeof(uint32_t);

void DispatchSimConnectMessage() {
  if (simconnect_avail) {
    HRESULT hr = SimConnect_CallDispatch(hSimConnect, IDCUDispatchProc, NULL);
    if (FAILED(hr)) InitSimConnect();
    else { // 연결 정상시 명령어 전송 시도
      FireMFCommand();
    }
  }

  // 연결 시도
  if (!simconnect_avail && GetTickCount64() - last_reconnect_at > 5000) {
    last_reconnect_at = GetTickCount64();
    InitSimConnect();
  }
}

HRESULT MapVAR(
  HRESULT hr,
  DATA_DEFINE_ID defID,
  CLIENT_DATA_REQUEST_ID requestID
) {
  hr &= SimConnect_AddToClientDataDefinition(
    hSimConnect,
    defID,
    SIZE_LVAR * (defID - 1000),
    SIZE_LVAR, 0
  );
  hr &= SimConnect_RequestClientData(
    hSimConnect,
    CLIENT_DATA_MF_IDCU_LVARS,
    requestID,
    defID,
    SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
    SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED,
    0, 0, 0
  );

  return hr;
}

// SimConnect & MobiFlight Client Data 영역 초기화
HRESULT InitSimConnect() {
  hSimConnect = NULL;
  BooleanFalse(&simconnect_avail);
  FlagUp(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);

  HRESULT hr = SimConnect_Open(&hSimConnect, "IDCU", nullptr, 0, 0, 0);

  // IDCU client 생성 요청
  hr &= SimConnect_MapClientDataNameToID(
    hSimConnect,
    MF_CLIENT_DATA_COMMAND,
    CLIENT_DATA_MF_COMMAND
  );
  hr &= SimConnect_AddToClientDataDefinition(
    hSimConnect,
    DEFINITION_MF_COMMAND,
    0, MF_MSG_BLOCK_SIZE, 0
  );

  char addClientCmd[] = "MF.Clients.Add.IDCU";
  hr &= SimConnect_SetClientData(
    hSimConnect,
    CLIENT_DATA_MF_COMMAND,
    DEFINITION_MF_COMMAND,
    SIMCONNECT_CLIENT_DATA_SET_FLAG_DEFAULT,
    0,
    MF_MSG_BLOCK_SIZE,
    &addClientCmd
  );

  // MobiFlight IDCU ClientData 매핑
  hr &= SimConnect_MapClientDataNameToID(
    hSimConnect,
    MF_IDCU_CLIENT_DATA_COMMAND,
    CLIENT_DATA_MF_IDCU_COMMAND
  );
  hr &= SimConnect_MapClientDataNameToID(
    hSimConnect,
    MF_IDCU_CLIENT_DATA_RESPONSE,
    CLIENT_DATA_MF_IDCU_RESPONSE
  );
  hr &= SimConnect_MapClientDataNameToID(
    hSimConnect,
    MF_IDCU_CLIENT_DATA_LVARS,
    CLIENT_DATA_MF_IDCU_LVARS
  );

  // MobiFlight IDCU ClientData 정의
  hr &= SimConnect_AddToClientDataDefinition(
    hSimConnect,
    DEFINITION_MF_IDCU_COMMAND,
    0, MF_MSG_BLOCK_SIZE, 0
  );
  hr &= SimConnect_AddToClientDataDefinition(
    hSimConnect,
    DEFINITION_MF_IDCU_RESPONSE,
    0, MF_MSG_BLOCK_SIZE, 0
  );

  hr &= MapVAR(hr, DEFINITION_LVAR_GND_SPD_BRK_ARMED, REQUEST_MF_SPD_BRK);
  hr &= MapVAR(hr, DEFINITION_LVAR_FLAPS_HANDLE,      REQUEST_MF_FLAPS_HANDLE);
  hr &= MapVAR(hr, DEFINITION_LVAR_ENG_MSTR1,         REQUEST_MF_ENG_MSTR1);
  hr &= MapVAR(hr, DEFINITION_LVAR_ENG_MSTR2,         REQUEST_MF_ENG_MSTR2);
  hr &= MapVAR(hr, DEFINITION_LVAR_MCDU_BRIGHTNESS,   REQUEST_MF_MCDU_BRIGHTNESS);
  hr &= MapVAR(hr, DEFINITION_LVAR_SEATBEALT_SIGN,    REQUEST_MF_SEATBEALT_SIGN);
  hr &= MapVAR(hr, DEFINITION_LVAR_BARO_MODE,         REQUEST_MF_BARO_MODE);
  hr &= MapVAR(hr, DEFINITION_LVAR_TERR_L,            REQUEST_MF_TERR_L);
  hr &= MapVAR(hr, DEFINITION_LVAR_TERR_R,            REQUEST_MF_TERR_R);
  hr &= MapVAR(hr, DEFINITION_LVAR_ND_MODE,           REQUEST_MF_ND_MODE);
  hr &= MapVAR(hr, DEFINITION_LVAR_ND_RANGE,          REQUEST_MF_ND_RANGE);
  hr &= MapVAR(hr, DEFINITION_LVAR_INTEG_ANN_LT,      REQUEST_MF_INTEG_ANN_LT);
  hr &= MapVAR(hr, DEFINITION_LVAR_STROBE_LT,         REQUEST_MF_STROBE_LT);
  hr &= MapVAR(hr, DEFINITION_LVAR_LDG_LT,            REQUEST_MF_LDG_LT);
  hr &= MapVAR(hr, DEFINITION_LVAR_NOSE_LT,           REQUEST_MF_NOSE_LT);
  hr &= MapVAR(hr, DEFINITION_LVAR_COM1_FREQ,         REQUEST_MF_COM1_FREQ);
  
  // Response, LVARS ClientData 요청
  hr &= SimConnect_RequestClientData(
    hSimConnect,
    CLIENT_DATA_MF_IDCU_RESPONSE,
    REQUEST_MF_RESPONSE,
    DEFINITION_MF_IDCU_RESPONSE,
    SIMCONNECT_CLIENT_DATA_PERIOD_ON_SET,
    SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED,
    0, 0, 0
  );
  
  // 받아올 LVAR 요청
  SendMobiFlightCommand("MF.SimVars.Clear", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_SPOILERS_ARMED)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_FLAPS_HANDLE_INDEX)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(A:FUELSYSTEM VALVE SWITCH:1, Bool)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(A:FUELSYSTEM VALVE SWITCH:2, Bool)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_MCDU_L_BRIGHTNESS)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(A:CABIN SEATBELTS ALERT SWITCH, Bool)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_FCU_EFIS_L_DISPLAY_BARO_VALUE_MODE)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_EFIS_TERR_L_ACTIVE)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_EFIS_TERR_R_ACTIVE)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_FCU_EFIS_L_EFIS_MODE)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_FCU_EFIS_L_EFIS_RANGE)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:A32NX_OVHD_INTLT_ANN)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:LIGHTING_STROBE_0)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:LIGHTING_LANDING_2)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(L:LIGHTING_LANDING_1)", false, 80);
  SendMobiFlightCommand("MF.SimVars.Add.(A:COM STANDBY FREQUENCY:1, kHz)", false, 80);

  if (SUCCEEDED(hr)) {
    BooleanTrue(&simconnect_avail);
    FlagDown(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);
  }

  return hr;
}

void SetLVAR(const string& lvarName, int value) {
  string command = A32NX_CMD::PREFIX + to_string(value) + " (>L:" + lvarName + ")";
  SendMobiFlightCommand(command, false);
}

void SetLVAR(const string& lvarName, float value) {
  string command = A32NX_CMD::PREFIX + to_string(value) + " (>L:" + lvarName + ")";
  SendMobiFlightCommand(command, false);
}

void SetAVAR(const string& avarName, int value) {
  string command = A32NX_CMD::PREFIX + to_string(value) + " (>A:" + avarName + ")";
  SendMobiFlightCommand(command, false);
}

void PressLVAR(const string& lvarName) {
  string command = A32NX_CMD::PREFIX + "1 (>L:" + lvarName + ")";
  SendMobiFlightCommand(command, false);

  command = A32NX_CMD::PREFIX + "0 (>L:" + lvarName + ")";
  SendMobiFlightCommand(command, false, 600);
}

void FireEVT(const string& keyEvent) {
  string command = A32NX_CMD::PREFIX + "(>K:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
}

void FireEVT(const string& keyEvent, int value) {
  string command = A32NX_CMD::PREFIX + to_string(value) + " (>K:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
}

void FireEVT_f(const string& keyEvent, float value) {
  string command = A32NX_CMD::PREFIX + to_string(value) + " (>K:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
}

void FireEVT(const string& keyEvent, int v1, int v2) {
  string command = A32NX_CMD::PREFIX + to_string(v1) + " " + to_string(v2) + " (>K:2:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
}

void ToggleEVT(const string& keyEvent) {
  string command = A32NX_CMD::PREFIX + "(>K:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
  SendMobiFlightCommand("", false, 80);
}

void ToggleEVT(const string& keyEvent, int v1) {
  string command = A32NX_CMD::PREFIX + to_string(v1) + " (>K:" + keyEvent + ")";
  SendMobiFlightCommand(command, false);
  SendMobiFlightCommand("", false, 80);
}

// MobiFlight.Command ClientData에 명령어 작성
void SendMobiFlightCommand(const string& command, bool lock, int hold) {
  MFCommandStruct mf_command;

  strcpy_s(mf_command.command, command.c_str());
  mf_command.execLock = lock;
  mf_command.hold = hold;

  mf_command_queue.push(mf_command);

  if (mf_command_queue.empty()) {
    FireMFCommand();
  }
}

// 명령 큐 처음에 있는 명령어 전송
void FireMFCommand() {
  if (mf_exec_lock || mf_command_queue.empty()) return;
  if (mf_command_queue.size() > 64) {
    FlagUp(&simconnect_ecam_msg, SIMCONNECT_MSG_QUEUE_EXCEED);
  }
  else {
    FlagDown(&simconnect_ecam_msg, SIMCONNECT_MSG_QUEUE_EXCEED);
  }

  char cmdStruct[1024];

  mf_exec_lock = true;
  MFCommandStruct commandStruct = mf_command_queue.front();
  mf_command_queue.pop();

  Sleep(commandStruct.hold);

  strcpy_s(cmdStruct, commandStruct.command);

  HRESULT hr = SimConnect_SetClientData(
    hSimConnect,
    CLIENT_DATA_MF_IDCU_COMMAND,
    DEFINITION_MF_IDCU_COMMAND,
    0,
    0,
    1024,
    &cmdStruct
  );

  if (FAILED(hr)) {
    FlagUp(&simconnect_ecam_msg, SIMCONNECT_MSG_SEND_FAULT);
#ifdef DEBUG
    ECAMRed("SIMCONNECT CMD SEND FAIL", GetHexErrorCode(hr));
#endif
  }
  if (SUCCEEDED(hr)) {
	FlagDown(&simconnect_ecam_msg, SIMCONNECT_MSG_SEND_FAULT);
#ifdef DEBUG
    ECAMGreen("SIMCONNECT CMD SENT", commandStruct.command);
#endif
  }

  mf_exec_lock = commandStruct.execLock;
}

// SimConnect 메시지 디스패치 콜백
void CALLBACK IDCUDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
  switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_CLIENT_DATA: {
	  ProcessMFResposneClientData(pData);
      break;
	}
  }
}

// SimConnect Response ClientData 처리
void ProcessMFResposneClientData(SIMCONNECT_RECV* pData) {
  SIMCONNECT_RECV_CLIENT_DATA* pCData = (SIMCONNECT_RECV_CLIENT_DATA*)pData;
  string str = std::string((char*)(&pCData->dwData));
  
  switch (pCData->dwDefineID) {
    case DEFINITION_MF_IDCU_RESPONSE: {
      cout << pCData->dwDefineID << " " << str << endl;
      mf_exec_lock = false;
      break;
    }
    case DEFINITION_LVAR_GND_SPD_BRK_ARMED:
    case DEFINITION_LVAR_FLAPS_HANDLE:
    case DEFINITION_LVAR_ENG_MSTR1:
    case DEFINITION_LVAR_ENG_MSTR2:
	case DEFINITION_LVAR_MCDU_BRIGHTNESS:
	case DEFINITION_LVAR_SEATBEALT_SIGN:
	case DEFINITION_LVAR_BARO_MODE:
    case DEFINITION_LVAR_TERR_L:
	case DEFINITION_LVAR_TERR_R:
	case DEFINITION_LVAR_ND_MODE:
	case DEFINITION_LVAR_ND_RANGE:
	case DEFINITION_LVAR_INTEG_ANN_LT:
	case DEFINITION_LVAR_STROBE_LT:
	case DEFINITION_LVAR_LDG_LT:
    case DEFINITION_LVAR_NOSE_LT:
	case DEFINITION_LVAR_COM1_FREQ:
    {
	  float* data = (float*)(&pCData->dwData);
	  updateACFTStatus((DATA_DEFINE_ID)(pCData->dwDefineID), *data);
      break;
    }
  }
}

void updateACFTStatus(
  DATA_DEFINE_ID defID,
  float val
) {
  switch(defID) {
    case DEFINITION_LVAR_GND_SPD_BRK_ARMED: {
	  acft_status.groundSpoilerArmed = (int)val;
      break;
    }
    case DEFINITION_LVAR_FLAPS_HANDLE: {
	  acft_status.flapsHandlePos = (int)val;
      break;
    }
    case DEFINITION_LVAR_ENG_MSTR1: {
      acft_status.engMasterState1 = (int)val;
      break;
    }
    case DEFINITION_LVAR_ENG_MSTR2: {
      acft_status.engMasterState2 = (int)val;
      break;
    }
    case DEFINITION_LVAR_MCDU_BRIGHTNESS: {
      acft_status.mcduBrightness = val;
      break;
    }
    case DEFINITION_LVAR_SEATBEALT_SIGN: {
      acft_status.seatbeltSign = (int)val;
      break;
    }
    case DEFINITION_LVAR_BARO_MODE: {
	  acft_status.baroMode = (int)val;
	  break;
    }
    case DEFINITION_LVAR_TERR_L: {
      acft_status.terrOnL = (int)val;
      break;
    }
    case DEFINITION_LVAR_TERR_R: {
      acft_status.terrOnR = (int)val;
      break;
    }
    case DEFINITION_LVAR_ND_MODE: {
      acft_status.ndMode = (int)val;
      break;
    }
    case DEFINITION_LVAR_ND_RANGE: {
      acft_status.ndRange = (int)val;
      break;
    }
    case DEFINITION_LVAR_INTEG_ANN_LT: {
	  acft_status.integAnnLt = (int)val;
	  break;
    }
    case DEFINITION_LVAR_STROBE_LT: {
      acft_status.strobeLt = (int)val;
      break;
    }
    case DEFINITION_LVAR_LDG_LT: {
      acft_status.ldgLT = (int)val;
      break;
    }
    case DEFINITION_LVAR_NOSE_LT: {
	  acft_status.noseLT = (int)val;
	  break;
    }
    case DEFINITION_LVAR_COM1_FREQ: {
      int freq = (int)(val);
      acft_status.com1Coarse = freq / 1000;
      acft_status.com1Fine = freq % 1000;
	  break;
    }
  }
}