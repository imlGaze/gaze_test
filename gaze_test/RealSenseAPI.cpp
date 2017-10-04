#include"RealSenseAPI.h"
#include <RealSense/Session.h>
#include <RealSense/SenseManager.h>
#include <RealSense/SampleReader.h>
#include <RealSense/Face/FaceModule.h>
#include <RealSense/Face/FaceData.h>
#include <RealSense/Face/FaceConfiguration.h>
#include<opencv2\opencv.hpp>
#include<iostream>

using namespace Intel::RealSense;


bool RealSenseAPI::initialize()
{
	/* RealSense�̏����� */
	senseManager = SenseManager::CreateInstance();
	senseManager->EnableStream(Capture::STREAM_TYPE_IR, 640, 480, 30);
	senseManager->EnableStream(Capture::STREAM_TYPE_COLOR, 640, 480, 30);
	senseManager->EnableFace();
	senseManager->Init();

	device = senseManager->QueryCaptureManager()->QueryDevice();
	if (device != nullptr)
	{
		device->ResetProperties(Capture::STREAM_TYPE_ANY);
		device->SetMirrorMode(Capture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		device->SetIVCAMLaserPower(1);

		fmod = senseManager->QueryFace();
		fdata = fmod->CreateOutput();
		fconfig = fmod->CreateActiveConfiguration();
		fconfig->detection.isEnabled = true;
		fconfig->detection.maxTrackedFaces = 3; // TODO: 1?
		fconfig->landmarks.isEnabled = true;
		landmarkPoints = new Face::FaceData::LandmarkPoint[fconfig->landmarks.numLandmarks];
		fconfig->ApplyChanges();

		return true;
	}


	return false;
}

bool RealSenseAPI::queryImage(Mat& inputImage, ResponseType type)
{
	status = senseManager->AcquireFrame(true);
	if (status < Status::STATUS_NO_ERROR)
	{
		return false;
	}

	const Capture::Sample *sample = senseManager->QuerySample();
	if (sample)
	{
		Image *img = type == IR ? sample->ir : sample->color;
		Image::PixelFormat format = type == IR ? Image::PIXEL_FORMAT_Y16 : Image::PIXEL_FORMAT_RGB24;

		Image::ImageData data = {}; //={}�\���̂̏��������@;
		Image::Rotation rotation = img->QueryRotation();

		status = img->AcquireAccess(Image::ACCESS_READ, format, rotation, Image::OPTION_ANY, &data);

		if (status >= Status::STATUS_NO_ERROR)
		{
			/* ������srcImage�ɃJ�����摜���R�s�[ */
			memcpy(inputImage.data, data.planes[0], data.pitches[0] * 480);
			senseManager->ReleaseFrame();
			img->ReleaseAccess(&data);
			return true;
		}
		else
		{
			std::cout << "error" << std::endl;
			return false;
		}
	}
	else
	{
		std::cout << "sample is nullptr";
		return false;
	}
}


bool RealSenseAPI::queryIRImage(Mat &irGray, Mat &irBinary, int thresh) {
	Size size = irGray.size();
	Mat irBuffer16U(size.height, size.width, CV_16UC1, Scalar(0, 0, 0));
	Mat irBuffer8UC1(size.height, size.width, CV_8UC1, Scalar(0));
	if (!queryImage(irBuffer16U, ResponseType::IR)) {
		return false;
	}
	
	irBuffer16U.convertTo(irBuffer8UC1, CV_8UC3); // 16bit -> 8bit
	cvtColor(irBuffer8UC1, irGray, CV_GRAY2BGR); //  // 1ch(Y) -> 3ch(B, G, R)

	irBuffer8UC1 = ~irBuffer8UC1; // Invert Black and white
	threshold(irBuffer8UC1, irBinary, thresh, 255, CV_THRESH_BINARY);
	return true;
}

bool RealSenseAPI::queryColorImage(Mat &colorColor, Mat &colorGray, Mat &colorBinary, int thresh) {
	Size size = colorColor.size();
	Mat colorBuffer8UC1(size.height, size.width, CV_8UC1, Scalar(0));
	if (!queryImage(colorColor, ResponseType::COLOR)) {
		return false;
	}

	cvtColor(colorColor, colorBuffer8UC1, CV_RGB2GRAY); // Gray scale
	cvtColor(colorBuffer8UC1, colorGray, CV_GRAY2BGR); //  // 1ch(Y) -> 3ch(B, G, R)
	
	colorBuffer8UC1 = ~colorBuffer8UC1;
	threshold(colorBuffer8UC1, colorBinary, thresh, 255, CV_THRESH_BINARY);
	return true;
}

// https://communities.intel.com/thread/111745
bool RealSenseAPI::queryFace(std::vector<cv::Point> &landmarks) {
	status = senseManager->AcquireFrame(true);
	if (status >= Status::STATUS_NO_ERROR) {
		fdata->Update();

		for (int i = 0, n = fdata->QueryNumberOfDetectedFaces(); i < n; i++) {
			Face::FaceData::Face *face = fdata->QueryFaceByIndex(i);

			if (face) {
				Face::FaceData::LandmarksData *lmdata = face->QueryLandmarks();
				if (lmdata) {
					int numPoints = lmdata->QueryNumPoints();
					if (numPoints == fconfig->landmarks.numLandmarks) {
						lmdata->QueryPoints(landmarkPoints);

						for (int j = 0; j < numPoints; j++) {
							if (landmarkPoints->confidenceImage) {
								if (Face::FaceData::LandmarkType::LANDMARK_EYE_RIGHT_CENTER <= landmarkPoints[j].source.alias && landmarkPoints[j].source.alias <= Face::FaceData::LandmarkType::LANDMARK_EYEBROW_LEFT_LEFT) {
									landmarks.push_back(Point(landmarkPoints[j].image.x, landmarkPoints[j].image.y));
								}
							}
						}

						senseManager->ReleaseFrame();
						return true;
					}
				}
			}
		}
	}

	senseManager->ReleaseFrame();
	return false;
}

