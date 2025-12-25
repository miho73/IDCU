#include "ecam.h"

#define TOTAL_WIDTH 45

enum ECAMState {
  ECAM_STATE_RED = 0,
  ECAM_STATE_GREEN = 1,
  ECAM_STATE_AMBER = 2,
  ECAM_STATE_BLUE = 3,
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
