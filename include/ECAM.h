#pragma once
#include <iostream>
using namespace std;

#define ECAM_BLUE   "\033[36m"  // Cyan (Action/Label)
#define ECAM_GREEN  "\033[32m"  // Green (Normal)
#define ECAM_AMBER  "\033[33m"  // Yellow (Caution)
#define ECAM_RED    "\033[31m"  // Red (Warning)
#define ECAM_WHITE  "\033[37m"  // White (Title)
#define RESET       "\033[0m"   // Color reset

void MEMOWhite(const string& message);
void MEMOBlue(const string& message);
void MEMOGreen(const string& message);
void MEMOAmber(const string& message);
void MEMORed(const string& message);
void ECAMBlue(const string& message, const string& status, const bool isFinal = true);
void ECAMGreen(const string& message, const string& status, const bool isFinal = true);
void ECAMAmber(const string& message, const string& status, const bool isFinal = true);
void ECAMRed(const string& message, const string& status, const bool isFinal = true);
