#pragma once

#include <mutex>
#include <condition_variable>
#include "crow.h"
#include "nvapiHelper.h"
#include "overlayManager.h"


extern std::mutex mtx;
extern std::condition_variable cv;
extern bool updated;


void startCrowServer(int& resolution, int& verticalResolution, float& frameRate);



