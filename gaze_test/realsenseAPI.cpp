#include"RealSenseAPI.h"
#include<opencv2\opencv.hpp>
#include<iostream>

using namespace Intel::RealSense;

void RealSenseAPI::initialize()
{
	/* RealSense�̏����� */
	senseManager = SenseManager::CreateInstance();
	senseManager->EnableStream(Capture::STREAM_TYPE_IR, 640, 480, 30);
	senseManager->EnableStream(Capture::STREAM_TYPE_COLOR, 640, 480, 30);
	senseManager->Init();

	device = senseManager->QueryCaptureManager()->QueryDevice();
	if (device != nullptr)
	{
		device->ResetProperties(Capture::STREAM_TYPE_ANY);
		device->SetMirrorMode(Capture::Device::MirrorMode::MIRROR_MODE_DISABLED);
		device->SetIVCAMLaserPower(1);
	}
	
}

void RealSenseAPI::queryImage(cv::Mat& inputImage, ResponseType type)
{
;
	status = senseManager->AcquireFrame(true);
	if (status < Status::STATUS_NO_ERROR)
	{
		return ;
	}
	const Capture::Sample *sample = senseManager->QuerySample();
	//����IR
	if (sample)
	{
		if (type == IR && sample->ir)
		{
			Image::ImageData data = {}; //={}�\���̂̏��������@;
			Image::Rotation rotation = sample->ir->QueryRotation();
			status = sample->ir->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_Y16, rotation, Image::OPTION_ANY, &data);
			if (status >= Status::STATUS_NO_ERROR)
			{
				/* ������srcImage�ɃJ�����摜���R�s�[ */
				memcpy(inputImage.data, data.planes[0], data.pitches[0] * 480);
				senseManager->ReleaseFrame();
				sample->ir->ReleaseAccess(&data);
			}
		}
		else if (type == COLOR && sample->color)
		{
			Image::ImageData data = {}; //={}�\���̂̏��������@;
			Image::Rotation rotation = sample->color->QueryRotation();
			status = sample->color->AcquireAccess(Image::ACCESS_READ, Image::PIXEL_FORMAT_RGB24, rotation, Image::OPTION_ANY, &data);
			if (status >= Status::STATUS_NO_ERROR)
			{
				memcpy(inputImage.data, data.planes[0], data.pitches[0] * 480);
				senseManager->ReleaseFrame();
				sample->color->ReleaseAccess(&data);
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