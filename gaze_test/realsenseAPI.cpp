#include"realsesenseAPI.h"
#include<opencv2\opencv.hpp>
#include<iostream>

using namespace Intel::RealSense;

void RealsenseAPI::initialize()
{
	/* RealSenseの初期化 */
	senseManager = SenseManager::CreateInstance();
	senseManager->EnableStream(Capture::STREAM_TYPE_IR, 640, 480, 30);
	senseManager->Init();

	device = senseManager->QueryCaptureManager()->QueryDevice();
	if (device != nullptr)
	{
		device->ResetProperties(Capture::STREAM_TYPE_ANY);
		device->SetMirrorMode(Capture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		device->SetIVCAMLaserPower(1);
	}
	
}

void RealsenseAPI::queryImage(cv::Mat& inputImage)
{
;
	status = senseManager->AcquireFrame(true);
	if (status < Status::STATUS_NO_ERROR)
	{
		return;
	}
	const Capture::Sample *sample = senseManager->QuerySample();
	//ここIR
	if (sample)
	{
		if (sample->ir)
		{
			Image::ImageData data = {}; //={}構造体の初期化方法;
			Image::Rotation rotation = sample->ir->QueryRotation();
			status = sample->ir->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_Y16, rotation, Image::OPTION_ANY, &data);
			if (status >= Status::STATUS_NO_ERROR)
			{
				/* ここでsrcImageにカメラ画像をコピー */
				memcpy(inputImage.data, data.planes[0], data.pitches[0] * 480);
				senseManager->ReleaseFrame();
				sample->ir->ReleaseAccess(&data);
			}
		}
		else
		{
			std::cout << "error" << std::endl;
		}
	}
	else
	{
		std::cout << "sample is nullptr";
	}
	
	
}