#pragma once
#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include<opencv2\opencv.hpp>

enum ResponseType {
	IR,
	COLOR,
	NONE,
};

class RealSenseAPI
{
public:
	void initialize();
	void queryImage(cv::Mat& inputIMage, ResponseType type);
	~RealSenseAPI() {
		senseManager->Release();
	};

private:
	Intel::RealSense::SenseManager* senseManager;
	Intel::RealSense::Capture::Device *device;
	Intel::RealSense::Status status;

};