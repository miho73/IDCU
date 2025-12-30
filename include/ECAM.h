#pragma once
#include <iostream>
#include <Windows.h>
#include <sstream>
#include <iomanip>
#include "joystick.h"

using namespace std;

#define ECAM_BLUE   "\033[36m"  // Cyan (Action/Label)
#define ECAM_GREEN  "\033[32m"  // Green (Normal)
#define ECAM_AMBER  "\033[33m"  // Yellow (Caution)
#define ECAM_RED    "\033[31m"  // Red (Warning)
#define ECAM_WHITE  "\033[37m"  // White (Title)
#define RESET       "\033[0m"   // Color reset

#define THRUST_POLLING_LINK_FAULT	1
#define THRUST_POLLING_DATA_FAULT	2
#define THRUST_POLLING_FAULT		4

#define SIMCONNECT_DISCONNECTED		1
#define SIMCONNECT_NOT_OPENED		2
#define SIMCONNECT_NOT_IN_SIM		4
#define SIMCONNECT_UNSUPPORTED_ACFT	8

extern bool sidestick_avail;
extern bool thrustlever_avail;
extern bool simconnect_avail;

extern uint32_t thrust_lever_ecam_msg;
extern uint32_t sidestick_ecam_msg;
extern uint32_t simconnect_ecam_msg;

string GetHexErrorCode(const HRESULT hr);

void MEMOWhite(const string& message);
void MEMOBlue(const string& message);
void MEMOGreen(const string& message);
void MEMOAmber(const string& message);
void MEMORed(const string& message);
void ECAMBlue(const string& message, const string& status, const bool isFinal = true);
void ECAMGreen(const string& message, const string& status, const bool isFinal = true);
void ECAMAmber(const string& message, const string& status, const bool isFinal = true);
void ECAMRed(const string& message, const string& status, const bool isFinal = true);
void MoveUp(int lines);

void BooleanFalse(bool* value);
void BooleanTrue(bool* value);
void FlagUp(uint32_t* memory, uint32_t flag);
void FlagDown(uint32_t* memory, uint32_t flag);
void ECAMPrint();