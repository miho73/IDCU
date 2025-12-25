#include "IDCU.h"

void PrintBanner() {
  cout << "IDCU Started";
}

void SetSpdLog() {
  set_level(level::info);
  set_pattern("[%Y-%m-%d %T.%e] [%^%5l%$] %v");
}

int main() {
  SetSpdLog();

  info("IDCU Application Starting...");
}
