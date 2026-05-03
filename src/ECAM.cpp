#include "ecam.h"

#define TOTAL_WIDTH 45

enum ECAMState {
  ECAM_STATE_RED    = 0,
  ECAM_STATE_GREEN  = 1,
  ECAM_STATE_AMBER  = 2,
  ECAM_STATE_BLUE   = 3,
};

void ECAMPrint(const std::string& label, const std::string& status, ECAMState state, const bool isFinal) {
  std::string statusColor = RESET;
  
  switch (state) {
    case ECAM_STATE_RED:    statusColor = ECAM_RED;   break;
    case ECAM_STATE_GREEN:  statusColor = ECAM_GREEN; break;
    case ECAM_STATE_AMBER:  statusColor = ECAM_AMBER; break;
	case ECAM_STATE_BLUE:   statusColor = ECAM_BLUE;  break;
    default: statusColor = ECAM_WHITE; break;
  }

  int dots = TOTAL_WIDTH - label.length();
  if (dots < 0) dots = 0;

  std::string dotsStr(dots, '.');

  std::cout << "\r"
       << ECAM_BLUE << label << " " << dotsStr
       << RESET << " "
       << statusColor << status << RESET;

  if (isFinal) std::cout << '\n';
}

void MEMOWhite(const std::string& message) {
  std::cout << ECAM_WHITE << message << RESET << '\n';
}

void MEMOBlue(const std::string& message) {
  std::cout << ECAM_BLUE << message << RESET << '\n';
}

void MEMOGreen(const std::string& message) {
  std::cout << ECAM_GREEN << message << RESET << '\n';
}

void MEMOAmber(const std::string& message) {
  std::cout << ECAM_AMBER << message << RESET << '\n';
}

void MEMORed(const std::string& message) {
  std::cout << ECAM_RED << message << RESET << '\n';
}

void ECAMAmber(const std::string& message, const std::string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_AMBER, isFinal);
}

void ECAMGreen(const std::string& message, const std::string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_GREEN, isFinal);
}

void ECAMBlue(const std::string& message, const std::string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_BLUE, isFinal);
}

void ECAMRed(const std::string& message, const std::string& status, const bool isFinal) {
  ECAMPrint(message, status, ECAM_STATE_RED, isFinal);
}

void MoveUp(int lines) {
  std::cout << "\x1b[" << lines << "A";
}

std::string GetHexErrorCode(const HRESULT hr) {
  std::stringstream ss;
  ss << std::hex << std::uppercase << std::setfill('0') << std::setw(8) << hr;

  return ss.str();
}

uint32_t joystick_ecam_msg = DIRECT_INPUT_INIT_FAULT;
uint32_t simconnect_ecam_msg = SIMCONNECT_DISCONNECTED;

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
  std::cout << "\033[2J\033[H";

  // RED MESSAGES
  if (joystick_ecam_msg & DIRECT_INPUT_INIT_FAULT) {
    MEMORed("DI8 INIT FAULT");
  }
  if (joystick_ecam_msg & DIRECT_INPUT_ENUM_FAULT) {
    MEMORed("DI8 DEVICE ENUM FAULT");
  }
  if (joystick_ecam_msg & THRUST_NOT_FOUND) {
    MEMORed("THR LVR NOT FOUND");
    ECAMBlue("-DEVICE", "CONNECT");
	ECAMBlue("-CONNECTION", "VERIFY");
  }
  if (joystick_ecam_msg & SIDESTICK_NOT_FOUND) {
    MEMORed("SIDESTICK NOT FOUND");
    ECAMBlue("-DEVICE", "CONNECT");
    ECAMBlue("-CONNECTION", "VERIFY");
  }
  if (joystick_ecam_msg & THRUST_INIT_FAULT) {
    MEMORed("THR LVR INIT FAULT");
  }
  if (joystick_ecam_msg & SIDESTICK_INIT_FAULT) {
    MEMORed("SIDESTICK INIT FAULT");
  }
  if (simconnect_ecam_msg & SIMCONNECT_DISCONNECTED) {
    MEMORed("SIMCONNECT DISC");
    ECAMBlue("-MSFS APP", "START");
  }

  // AMBER MESSAGES
  if (joystick_ecam_msg & THRUST_POLLING_FAULT) {
    MEMOAmber("THR LVR POLLING FAULT");
  }
  if (joystick_ecam_msg & THRUST_GET_FAULT) {
    MEMOAmber("THR LVR GET FAULT");
  }
  if (joystick_ecam_msg & SIDESTICK_POLLING_FAULT) {
    MEMOAmber("SIDE STICK POLLING FAULT");
  }
  if (joystick_ecam_msg & SIDESTICK_GET_FAULT) {
    MEMOAmber("SIDE STICK GET FAULT");
  }
  if(simconnect_ecam_msg & SIMCONNECT_MSG_SEND_FAULT) {
    MEMOAmber("SIMCONNECT MSG NOT SENT");
  }
  if (simconnect_ecam_msg & SIMCONNECT_MSG_QUEUE_EXCEED) {
    MEMOAmber("SIMCONNECT CMD BUFFER FULL");
  }
  if (simconnect_ecam_msg & SIMCONNECT_SYNC_IN_PROG) {
    MEMOAmber("SIMCONNECT SYNC IN PROG");
  }

  // GREEN MESSAGES
  if (joystick_ecam_msg == 0) {
    switch (mode) {
      case 1:
      MEMOGreen("MODE FCTL");
      break;
      case 2:
      MEMOGreen("MODE NAV");
      break;
      case 3:
      MEMOGreen("MODE LT");
      break;
      default:
      MEMOAmber("MODE NOT SET");
      ECAMBlue("-MODE SEL", "ROTATE");
    }
  }
}