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
	void initialize();
	void queryImage(Mat& inputImage, ResponseType type);

	void queryIRImage(Mat &irGray, Mat &irBinary, int thresh);
	void queryColorImage(Mat &color, Mat &gray, Mat &colorBinary, int thresh);

	~RealSenseAPI() {
		senseManager->Release();
	};

private:
	Intel::RealSense::SenseManager* senseManager;
	Intel::RealSense::Capture::Device *device;
	Intel::RealSense::Status status;

};