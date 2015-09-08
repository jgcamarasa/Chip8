#pragma once

#define DISPLAY_FACTOR 8

#define SIDE_PANEL_W 200
#define SIDE_PANEL_EXTRA_H 0
#define WIN_W DISPLAY_W * DISPLAY_FACTOR + SIDE_PANEL_W
#define WIN_H DISPLAY_H * DISPLAY_FACTOR + SIDE_PANEL_EXTRA_H + 200

#include "emulator.h"

enum ControllerStatus
{
	CONTROLLER_STATUS_RUNNING,
	CONTROLLER_STATUS_PAUSED,
};

struct Controller
{
	ControllerStatus status;
	State *state;
};

void drawGUI(State *state, Controller *controller);
void getOpString(uint32 op, char *dest);