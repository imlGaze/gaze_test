#pragma once
#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include<opencv2\opencv.hpp>

using namespace cv;

enum ResponseType {
	IR,
	COLOR,
	NONE,
};

class RealSenseAPI
{
public:
	bool initialize();
	bool setLaserPower(int val);

	bool queryImage(Mat& inputImage, ResponseType type);

	bool queryIRImage(Mat &irGray, Mat &irBinary, int thresh);
	bool queryColorImage(Mat &color, Mat &gray, Mat &colorBinary, int thresh);

	~RealSenseAPI() {
		senseManager->Release();
	};

private:
	Intel::RealSense::SenseManager* senseManager;
	Intel::RealSense::Capture::Device *device;
	Intel::RealSense::Status status;

};