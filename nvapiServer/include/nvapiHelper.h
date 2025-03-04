#pragma once

#include <queue>
#include <d2dHelper.h>
#include "nvapi.h"
#include "overlayManager.h"
#include "global.h"



NvAPI_Status GetConnectedDisplays(NvU32* displayIds, NvU32* noDisplays);
NvAPI_Status ApplyCustomDisplay(int& horRes, int& verRes, float& rr);
NvAPI_Status nvInitialize();

void loadCustomDisplay(NV_CUSTOM_DISPLAY* customDisplay, int& horRes, int& verRes);
void overlayDisplays(OverlayManager &overlays);

struct nvapiTask {
	enum Type {
		Identify_Displays, Apply_Settings
	} taskType;

};




