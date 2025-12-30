#include "IDCU.h"

bool idcu_avail = false;

int main() {
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info;
  info.dwSize = 100;
  info.bVisible = FALSE;
  SetConsoleCursorInfo(consoleHandle, &info);

  InitializeJoysticks();
  FindJoysticks();

  ECAMPrint();

  while (true) {
    ProcessThrustLeverInput();
    DispatchSimConnectMessage();
    Sleep(30);
  }
}

void halt() {
  system("pause");
  exit(-1);
}