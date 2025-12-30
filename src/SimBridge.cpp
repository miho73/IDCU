#include "SimBridge.h"

HANDLE hSimConnect = NULL;

SIMCONNECT_CLIENT_DATA_ID ClientDataID = 1;

enum SYS_STATE_REQUESTS {
  REQUEST_ACFT_LOADED,
};

enum EVENT_ID {
  EVENT_ACFT_LOADED,
};

enum ACFT_TYPES {
  ACFT_TYPE_UNKNOWN,
  ACFT_TYPE_A32NX,
  ACFT_TYPE_A388,
};

enum DATA_DEFINE_ID {
  ACFT_STATE_DEFINITION
};

struct ACFT_STATE_DATA {
  bool idcu_available;
};

void CALLBACK IDCUDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext);
void IDCUSimConnectEventHandler(SIMCONNECT_RECV_EVENT* evt);
void IDCUSimConnectSystemStateHandler(SIMCONNECT_RECV_SYSTEM_STATE* state);
void IDCUSimConnectFilenameHandler(SIMCONNECT_RECV_EVENT_FILENAME* evt);

ACFT_TYPES current_acft_type = ACFT_TYPE_UNKNOWN;

void InitializeSimBridge() {
  HRESULT hr;

  ECAMAmber("SIMCONNECT INIT", "IN PROG");

  do {
	hr = SimConnect_Open(&hSimConnect, "IDCU", nullptr, 0, 0, 0);
	if (FAILED(hr)) {
	  MEMORed("\nSIMCONNECT CONN FAULT");
	  ECAMBlue("MSFS APP", "START");
	  for (int i = 5; i > 0; i--) {
		MEMOGreen("RETRY IN " + std::to_string(i) + "s\x1b[1A");
		Sleep(1000);
	  }
	  MoveUp(3);
	}
  } while (hSimConnect == NULL);

  hr = SimConnect_MapClientDataNameToID(hSimConnect, "IDCU_STATE", ClientDataID);
  hr &= SimConnect_AddToClientDataDefinition(hSimConnect, ACFT_STATE_DEFINITION, SIMCONNECT_CLIENTDATAOFFSET_AUTO, sizeof(ACFT_STATE_DATA));
  hr &= SimConnect_CreateClientData(hSimConnect, ClientDataID, sizeof(ACFT_STATE_DATA), SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);

  if (FAILED(hr)) {
    MEMORed("\nSIMCONNECT INIT FAULT\n                                                            \n                                                            \n                                                            ");
    halt();
  }

  FlagUp(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
}

ULONGLONG last_reconnect_at = 0;
void DispatchSimBridgeMessage() {
  if (hSimConnect != NULL) {
    HRESULT hr = SimConnect_CallDispatch(hSimConnect, IDCUDispatchProc, NULL);

    if (FAILED(hr)) {
	  BooleanFalse(&simconnect_avail);
      FlagUp(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
      FlagUp(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);
    }
  }
  else {
	BooleanFalse(&simconnect_avail);
    FlagUp(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
    FlagUp(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);
  }

  if(!simconnect_avail && GetTickCount64() - last_reconnect_at > 5000) {
	last_reconnect_at = GetTickCount64();

    HRESULT hr = SimConnect_Open(&hSimConnect, "IDCU", NULL, 0, 0, 0);
    if (SUCCEEDED(hr)) {
      hr = SimConnect_MapClientDataNameToID(hSimConnect, "IDCU_STATE", ClientDataID);
      hr &= SimConnect_AddToClientDataDefinition(hSimConnect, ACFT_STATE_DEFINITION, SIMCONNECT_CLIENTDATAOFFSET_AUTO, sizeof(ACFT_STATE_DATA));
      hr &= SimConnect_CreateClientData(hSimConnect, ClientDataID, sizeof(ACFT_STATE_DATA), SIMCONNECT_CLIENT_DATA_REQUEST_FLAG_CHANGED);
      
      if (SUCCEEDED(hr)) {
        FlagUp(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
      }
	}
  }
}

void CALLBACK IDCUDispatchProc(SIMCONNECT_RECV* pData, DWORD cbData, void* pContext) {
  switch (pData->dwID) {
    case SIMCONNECT_RECV_ID_OPEN: {
      if (simconnect_ecam_msg & SIMCONNECT_NOT_OPENED) {
		BooleanTrue(&simconnect_avail);
        FlagDown(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
        FlagDown(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);
      }
      break;
    }
    case SIMCONNECT_RECV_ID_QUIT: {
      BooleanFalse(&simconnect_avail);
	  FlagUp(&simconnect_ecam_msg, SIMCONNECT_NOT_OPENED);
      FlagUp(&simconnect_ecam_msg, SIMCONNECT_DISCONNECTED);
      break;
    }
	case SIMCONNECT_RECV_ID_EVENT: {
	  SIMCONNECT_RECV_EVENT* evt = (SIMCONNECT_RECV_EVENT*)pData;
	  IDCUSimConnectEventHandler(evt);
	  break;
	}
    case SIMCONNECT_RECV_ID_EVENT_FILENAME: {
	  SIMCONNECT_RECV_EVENT_FILENAME* evt = (SIMCONNECT_RECV_EVENT_FILENAME*)pData;
	  IDCUSimConnectFilenameHandler(evt);
    }
    case SIMCONNECT_RECV_ID_SYSTEM_STATE: {
      SIMCONNECT_RECV_SYSTEM_STATE* state = (SIMCONNECT_RECV_SYSTEM_STATE*)pData;
      IDCUSimConnectSystemStateHandler(state);
      break;
    }
  }
}

void IDCUSimConnectEventHandler(SIMCONNECT_RECV_EVENT* evt) {
  switch (evt->uEventID) {
    
  }
}

void IDCUSimConnectFilenameHandler(SIMCONNECT_RECV_EVENT_FILENAME* evt) {
  switch (evt->uEventID) {
    case EVENT_ACFT_LOADED: {
	  string str(evt->szFileName);
      if (str.ends_with(A32NX_ACFT_PATH)) {
		FlagDown(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
		current_acft_type = ACFT_TYPE_A32NX;
      }
      else if (str.ends_with(A388_ACFT_PATH)) {
		FlagDown(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
		current_acft_type = ACFT_TYPE_A388;
      }
      else {
		FlagUp(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
		current_acft_type = ACFT_TYPE_UNKNOWN;
      }
      break;
    }
  }
}

void IDCUSimConnectSystemStateHandler(SIMCONNECT_RECV_SYSTEM_STATE* state) {
  switch (state->dwRequestID) {
    case REQUEST_ACFT_LOADED: {
      string str(state->szString);
	  transform(str.begin(), str.end(), str.begin(), tolower);
      if (str.ends_with(A32NX_ACFT_PATH)) {
        FlagDown(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
        current_acft_type = ACFT_TYPE_A32NX;
      }
      else if (str.ends_with(A388_ACFT_PATH)) {
        FlagDown(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
        current_acft_type = ACFT_TYPE_A388;
      }
      else {
        FlagUp(&simconnect_ecam_msg, SIMCONNECT_UNSUPPORTED_ACFT);
        current_acft_type = ACFT_TYPE_UNKNOWN;
      }
      break;
    }
  }
}
