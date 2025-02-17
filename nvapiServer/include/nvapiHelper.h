#pragma once

#include "nvapi.h"
#include <queue>
#include <d2dHelper.h>


NvAPI_Status NvAPI_DISP_IdentifyDisplay(NvU32 displayID);
NvAPI_Status GetConnectedDisplays(NvU32* displayIds, NvU32* noDisplays);
NvAPI_Status ApplyCustomDisplay(int& horRes, int& verRes, float& rr);
NvAPI_Status nvInitialize();

void loadCustomDisplay(NV_CUSTOM_DISPLAY* customDisplay, int& horRes, int& verRes);
void identifyConnectedDisplays();

struct nvapiTask {
	enum Type {
		Identify_Displays, Apply_Settings
	} taskType;

};

extern std::queue<nvapiTask> nvapiTaskQueue;
extern std::mutex taskQueueMutex;
extern std::condition_variable cv;

