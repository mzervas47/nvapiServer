#include <WinSock2.h>
#include <Windows.h>
#include <tchar.h>
#include <stdio.h>
#include "SDKDDKver.h"
#include <string>
#include "crowServer.h"
#include <thread>
#include <chrono>
#include "nvapiHelper.h"






std::mutex mtx;
std::condition_variable cv;
std::queue<nvapiTask> taskQueue;
bool updated = false;



int _tmain(int argc, _TCHAR* argV[]) {


	NvAPI_Status ret = nvInitialize();

	if (ret != NVAPI_OK) {
		printf("Failed it initialize NVAPI.Exiting: %d\n", ret);
	}

	int horRes = 0;
	int verRes = 0;
	float rr = 0;

	std::thread serverThread(startCrowServer, std::ref(horRes), std::ref(verRes), std::ref(rr));
	
	std::thread nvapiThread([&]() {
		while (true) {

			std::unique_lock<std::mutex> lock(mtx);
			cv.wait(lock, [] { return updated; });

			if (!taskQueue.empty()) {
				nvapiTask currentTask = taskQueue.front();
				taskQueue.pop();

				switch (currentTask.taskType) {
					case nvapiTask::Identify_Displays:
						identifyConnectedDisplays();
						break;

					case nvapiTask::Apply_Settings:
						{
							NvAPI_Status ret = ApplyCustomDisplay(horRes, verRes, rr);
							if (ret != NVAPI_OK) {
								printf("ApplyCustomDisplay failed = 0x%x\n", ret);
							}
							else {
								printf("\nApplyCustomDisplay success.\n");
							}

						}
						break;
				}
			}
			

			updated = false;
		}
		});

	serverThread.join();
	nvapiThread.join();

	return 0;

}





