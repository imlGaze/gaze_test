#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"realsesenseAPI.h"
#include<vector>

using namespace cv;
using std::vector;

int main()
{
	Mat irImage16U(480, 640, CV_16UC1, Scalar(0)); //深度用mat
	Mat irImage8UC3(480, 640, CV_8UC3, Scalar(25,50,200));
	Mat irImage8U(480, 640, CV_8UC1, Scalar(0));
	Mat binImage(480, 640, CV_8UC1, Scalar(0));
	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(640,480), true);
	RealsenseAPI realsense;
	realsense.initialize();
												 /* ウインドウを作成 */
	namedWindow("bin");
	namedWindow("ir", WINDOW_AUTOSIZE);
	waitKey(1);
	int loopCount = 0;

	int maxArea = 800;
	int minArea = 70;
	int thresh = 100;

	while (1)
	{
		vector<vector<Point>> contours;
		char key = waitKey(1);
		
		realsense.queryImage(irImage16U);
		irImage16U.convertTo(irImage8U, CV_8UC1);
		//irImage16U.convertTo(irImage8UC3, CV_8UC3);
		cvtColor(irImage8U, irImage8UC3, CV_GRAY2BGR);

		irImage8U = ~irImage8U;
		threshold(irImage8U, binImage, thresh, 255, CV_THRESH_BINARY);
		Mat element(3, 3, CV_8UC1);
		erode(binImage, binImage, element);
		erode(binImage, binImage, element);
		dilate(binImage,binImage, element);
		dilate(binImage, binImage, element);
		dilate(binImage, binImage, element);
		cv::findContours(binImage, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE);

		for (int i = 0,n = contours.size();i < n; i++)
		{
			if (cv::contourArea(contours[i])>maxArea || cv::contourArea(contours[i])<minArea)
			{
				continue;
			}
			Moments moment = cv::moments(contours[i]);
			Point point = Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
			if (point.x < 48 || 640 - 48*2 < point.x)
			{
				continue;
			}

			if (point.y < 50 || 480 - 50 *2 < point.y)
			{
				continue;
			}

			Rect rect = boundingRect(contours[i]);
			rectangle(irImage8UC3, rect,cv::Scalar(255,0,0),2);

		}
		cv::imshow("ir", irImage8UC3);
		cv::imshow("bin", binImage);

		std::cout << ".";
		loopCount++;
		if (loopCount >40)
		{
			loopCount = 0;
			std::cout << std::endl;
		}
		if (key == 'q')
		{
			break;
		}

		if ('s' == key)
		{
			std::cout << "maxArea" << std::endl;
			std::cin >> maxArea;
			std::cout << "minArea" << std::endl;
			std::cin >> minArea;
			std::cout << "thresh" << std::endl;
			std::cin >> thresh;
		}

		writer << irImage8UC3;

	}

	cv::destroyAllWindows();
	return 0;

}
