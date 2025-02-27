#include "global.h"

std::queue<nvapiTask> taskQueue;
std::mutex mtx;
std::condition_variable cv;
bool updated = false;