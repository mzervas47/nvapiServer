#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include "nvapiHelper.h" 

struct nvapiTask;
extern std::queue<nvapiTask> taskQueue;
extern std::mutex mtx;  
extern std::condition_variable cv;
extern bool updated;
