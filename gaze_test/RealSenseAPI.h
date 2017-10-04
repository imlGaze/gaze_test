#pragma once
#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include <RealSense/Face/FaceModule.h>
#include <RealSense/Face/FaceData.h>
#include <RealSense/Face/FaceConfiguration.h>
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
	bool queryImage(Mat& inputImage, ResponseType type);

	bool queryIRImage(Mat &irGray, Mat &irBinary, int thresh);
	bool queryColorImage(Mat &color, Mat &gray, Mat &colorBinary, int thresh);

	bool queryFace(std::vector<cv::Point> &landmarks);

	~RealSenseAPI() {
		senseManager->Release();
		delete landmarkPoints;
	};

private:
	Intel::RealSense::SenseManager *senseManager;
	Intel::RealSense::Capture::Device *device;
	Intel::RealSense::Status status;

	Intel::RealSense::Face::FaceModule *fmod;
	Intel::RealSense::Face::FaceData *fdata;
	Intel::RealSense::Face::FaceConfiguration *fconfig;
	Intel::RealSense::Face::FaceData::LandmarkPoint *landmarkPoints;

};