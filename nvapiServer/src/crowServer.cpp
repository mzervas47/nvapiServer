#include <iostream>
#include <thread>
#include "crowServer.h"







void makeRes(int& horRes, int& verRes) {

	if (horRes == 3840) {
		verRes = 2160;
	}
	else if (horRes == 1920) {
		verRes = 1080;
	}
	else {
		std::cerr << "Error: Unsupported horizontal resolution: " << horRes << std::endl;
		verRes = 1080;
	}

}

void handleRequest(const crow::request& req, crow::response& res, int& horRes, int& verRes, float& frameRate) {
	
	auto json_data = crow::json::load(req.body);
	if (!json_data) {
		res.code = 400;
		res.write("Invalid JSON");
		res.end();
		return;
	}
	
	horRes = json_data["resolution"].i();
	makeRes(horRes, verRes);
	frameRate = static_cast<float>(json_data["frameRate"].d());

	std::cout << "Requested Resolution: " << horRes << " * " << verRes << std::endl;
	std::cout << "Requested Frame Rate: " << frameRate << std::endl;

	res.code = 200;
	res.write("Data received and processed");
	res.end();
}

static OverlayManager overlayManagerInstance;


void startCrowServer(int& horRes, int& verRes, float& frameRate) {

	crow::SimpleApp app;

	CROW_ROUTE(app, "/applySettings").methods(crow::HTTPMethod::POST)([&](const crow::request& req, crow::response& res) {
		handleRequest(req, res, horRes, verRes, frameRate);

		{
			std::lock_guard<std::mutex> lock(mtx);
			nvapiTask task;
			task.taskType = nvapiTask::Apply_Settings;
			taskQueue.push(task);
			updated = true;
		}
		cv.notify_one();

		});

	CROW_ROUTE(app, "/identifyDisplays").methods(crow::HTTPMethod::POST)([]() {

		
		
		{
			std::lock_guard<std::mutex> lock(mtx);
			nvapiTask task;
			task.taskType = nvapiTask::Identify_Displays;
			taskQueue.push(task);
			updated = true;
		}
		cv.notify_one();

		return crow::response(200, "Identify Displays task enqueued");
		
		

		});

	app.port(8080).multithreaded().run();

}




