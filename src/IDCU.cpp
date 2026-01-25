#include "IDCU.h"

int main() {
  // 커서 숨기기
  HANDLE consoleHandle = GetStdHandle(STD_OUTPUT_HANDLE);
  CONSOLE_CURSOR_INFO info;
  info.dwSize = 100;
  info.bVisible = FALSE;
  SetConsoleCursorInfo(consoleHandle, &info);

  // DirectInput 초기화
  InitializeDirectInput();

  // ECAM 초기 표시
  ECAMPrint();

  // 메인 루프
  while (true) {
	JoystickLoop();
    DispatchSimConnectMessage();
    Sleep(30);
  }
}
