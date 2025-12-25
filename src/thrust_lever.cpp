#include "IDCU.h"

enum ThrustButtons {
  MODE1 = 34,
  MODE2 = 35,
  MODE3 = 36,
};

LPDIRECTINPUTDEVICE8 pThrust = NULL;

struct ButtonState128 {
  uint32_t chunks[4];

  ButtonState128() {
    std::memset(chunks, 0, sizeof(chunks));
  }
};

bool isThrustLeverInitialized = false;
ButtonState128 prevButtonState;

void PressHandler(int btnID);
void PrintMode();

void InitThrust() {
  DIJOYSTATE2 js;
  if (SUCCEEDED(pThrust->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
	ButtonState128 currentButtonState;
	for (int i = 0; i < 128; ++i)
	  if (js.rgbButtons[i] & 0x80)
		currentButtonState.chunks[i / 32] |= (1 << (i % 32));

	for (int i = 0; i < 4; i++)
	  prevButtonState.chunks[i] = currentButtonState.chunks[i];

	if (currentButtonState.chunks[MODE1 / 32] & (1 << (MODE1 % 32))) mode = 1;
	else if (currentButtonState.chunks[MODE2 / 32] & (1 << (MODE2 % 32))) mode = 2;
	else if (currentButtonState.chunks[MODE3 / 32] & (1 << (MODE3 % 32))) mode = 3;
	else {
	  MEMOAmber("\nMODE NOT SET");
	  ECAMBlue("-MODE SEL", "ROTATE", false);
	}
  }
  else {
	MEMORed("\nTHR LVR FAULT");

	exit(-1);
  }
}

void ReadThrustLeverInput() {
  HRESULT hr = pThrust->Poll();
  if (FAILED(hr)) {
    pThrust->Acquire();
	return;
  }

  DIJOYSTATE2 js;
  if (SUCCEEDED(pThrust->GetDeviceState(sizeof(DIJOYSTATE2), &js))) {
	if (mode == -1) {
	  if (js.rgbButtons[MODE1] & 0x80) mode = 1;
	  else if (js.rgbButtons[MODE2] & 0x80) mode = 2;
	  else if (js.rgbButtons[MODE3] & 0x80) mode = 3;

	  if (mode != -1) {
		MEMOGreen("\r                                                            \x1b[A\r                                                            \rIDCU AVAIL\n");
	  }
	  else return;
	}

	ButtonState128 currentButtonState;
	for (int i = 0; i < 128; ++i)
	  if (js.rgbButtons[i] & 0x80)
		currentButtonState.chunks[i / 32] |= (1 << (i % 32));

	for (int i = 0; i < 4; i++) {
	  uint32_t changes = currentButtonState.chunks[i] ^ prevButtonState.chunks[i];

	  if (changes != 0) {
		for (int bit = 0; bit < 32; bit++) {
		  uint32_t mask = (1 << bit);
		  if (changes & mask) {
			int btnID = i * 32 + bit + 1;

			if (currentButtonState.chunks[i] & mask) PressHandler(btnID);
			else {
			  //cout << "[EVT] Button " << btnID << " RELEASED" << endl;
			}
		  }
		}
	  }

	  prevButtonState.chunks[i] = currentButtonState.chunks[i];
	}
  }
}

void PressHandler(int btnID) {
  switch (btnID) {
	case MODE1:
	  mode = 1;
	  PrintMode();
	  break;
	case MODE2:
	  mode = 2;
	  PrintMode();
	  break;
	case MODE3:
	  mode = 3;
	  PrintMode();
	  break;
  }
}

void PrintMode() {
  string msg = "\x1b[AMODE ";

  switch (mode) {
	case 1:
	  msg += "FCTL";
	  break;
	case 2:
	  msg += "NAV ";
	  break;
	case 3:
	  msg += "COM ";
	  break;
	default:
	  msg += "UNK ";
	  break;
  }

  MEMOGreen(msg);
}