#include "ecam.h"

#define TOTAL_WIDTH 45

enum ECAMState {
  ECAM_STATE_RED    = 0,
  ECAM_STATE_GREEN  = 1,
  ECAM_STATE_AMBER  = 2,
  ECAM_STATE_BLUE   = 3,
};

void ECAMPrint(const string& label, const string& status, ECAMState state, const bool isFinal) {
  string statusColor = RESET;
  
  switch (state) {
    case ECAM_STATE_RED:    statusColor = ECAM_RED;   break;
    case ECAM_STATE_GREEN:  statusColor = ECAM_GREEN; break;
    case ECAM_STATE_AMBER:  statusColor = ECAM_AMBER; break;
	case ECAM_STATE_BLUE:   statusColor = ECAM_BLUE;  break;
    default: statusColor = ECAM_WHITE; break;
  }

  int dots = TOTAL_WIDTH - label.length();
  if (dots < 0) dots = 0;

  string dotsStr(dots, '.');

  cout << "\r"
       << ECAM_BLUE << label << " " << dotsStr
       << RESET << " "
       << statusColor << status << RESET;

  if (isFinal) cout << '\n';
}

void MEMOWhite(const string& message) {
  cout << ECAM_WHITE << message << RESET << '\n';
}

void MEMOBlue(const string& message) {
  cout << ECAM_BLUE << message << RESET << '\n';
}

void MEMOGreen(const string& message) {
  cout << ECAM_GREEN << message << RESET << '\n';
}

void MEMOAmber(const string& message) {
  cout << ECAM_AMBER << message << RESET << '\n';
}

void MEMORed(const string& message) {
  cout << ECAM_RED << message << RESET << '\n';
}

void ECAMAmber(const string& message, const string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_AMBER, isFinal);
}

void ECAMGreen(const string& message, const string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_GREEN, isFinal);
}

void ECAMBlue(const string& message, const string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_BLUE, isFinal);
}

void ECAMRed(const string& message, const string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_RED, isFinal);
}

void MoveUp(int lines) {
  cout << "\x1b[" << lines << "A";
}

string GetHexErrorCode(const HRESULT hr) {
  stringstream ss;
  ss << hex << uppercase << setfill('0') << setw(8) << hr;

  return ss.str();
}


bool sidestick_avail = false;
bool thrustlever_avail = false;
bool simconnect_avail = false;

uint32_t thrust_lever_ecam_msg = 0x0;
uint32_t sidestick_ecam_msg = 0x0;
uint32_t simconnect_ecam_msg = SIMCONNECT_DISCONNECTED;

void BooleanFalse(bool* value) {
  if (*value) {
    *value = false;
    ECAMPrint();
  }
}

void BooleanTrue(bool* value) {
  if (!*value) {
    *value = true;
    ECAMPrint();
  }
}

void FlagUp(uint32_t* memory, uint32_t flag) {
  if ((*memory & flag) == 0) {
    *memory |= flag;
    ECAMPrint();
  }
}

void FlagDown(uint32_t* memory, uint32_t flag) {
  if (*memory & flag) {
	*memory ^= flag;
    ECAMPrint();
  }
}

void ECAMPrint() {
  system("cls");

  if (sidestick_avail) {
    ECAMGreen("SIDESTICK", "AVAIL");
  } else {
    ECAMAmber("SIDESTICK", "INOP");
  }

  if (thrustlever_avail) {
    ECAMGreen("THR LVR", "AVAIL");
  } else {
    ECAMAmber("THR LVR", "INOP");
  }

  if(simconnect_avail) {
    ECAMGreen("SIMCONNECT", "AVAIL");
  } else {
    ECAMAmber("SIMCONNECT", "INOP");
  }

  cout << "\n";

  if (thrust_lever_ecam_msg & THRUST_POLLING_LINK_FAULT) {
    MEMOAmber("THR LVR LINK FAULT");
	ECAMBlue("-IDCU WINDOW", "SELECT");
    ECAMBlue("-OTHER PROGRAM", "EXIT");
	MEMOGreen("AUTO RECOVER ACT");
  }
  if (thrust_lever_ecam_msg & THRUST_POLLING_DATA_FAULT) {
    MEMORed("THR LVR LOST CONN");
    ECAMBlue("-DEVICE", "CHECK");
    ECAMBlue("-CONNECTION", "VERIFY");
  }
  if (thrust_lever_ecam_msg & THRUST_POLLING_FAULT) {
    MEMOAmber("THR LVR SYS FAULT");
  }

  if (simconnect_ecam_msg & SIMCONNECT_DISCONNECTED) {
    MEMORed("SIMCONNECT DISC");
    ECAMBlue("-MSFS APP", "START");
	MEMOGreen("AUTO RECONN ACT");
  }

  if (thrust_lever_ecam_msg == 0) {
    switch (mode) {
      case 1:
      MEMOGreen("MODE FCTL");
      break;
      case 2:
      MEMOGreen("MODE NAV");
      break;
      case 3:
      MEMOGreen("MODE COM");
      break;
      default:
      MEMOAmber("MODE NOT SET");
      ECAMBlue("-MODE SEL", "ROTATE");
    }
  }
}