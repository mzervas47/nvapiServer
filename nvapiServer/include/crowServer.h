#pragma once

#include <mutex>
#include <condition_variable>
#include "crow.h"


extern std::mutex mtx;
extern std::condition_variable cv;
extern bool updated;


void startCrowServer(int& resolution, int& verticalResolution, float& frameRate);



