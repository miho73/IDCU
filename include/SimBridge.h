#pragma once

#include <Windows.h>
#include <string>
#include <algorithm>
#include "ECAM.h"
#include "SimConnect.h"

#define A32NX_ACFT_PATH "simobjects\\airplanes\\flybywire_a320_neo\\aircraft.cfg"
#define A388_ACFT_PATH  "simobjects\\airplanes\\flybywire_a380_842\\aircraft.cfg"

void InitializeSimBridge();
void DispatchSimBridgeMessage();