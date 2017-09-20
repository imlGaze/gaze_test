#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"RealSenseAPI.h"
#include"ProcessUtil.h"
#include<vector>

using namespace cv;
using std::vector;

int main()
{
	Mat irGray(480, 640, CV_8UC3, Scalar(25, 50, 200)); // �O���[�X�P�[���A3ch�A�\���p
	Mat irBinary(480, 640, CV_8UC1, Scalar(0)); // 2�l�A�֊s���o�p

	Mat colorColor(480, 640, CV_8UC3, Scalar(0, 0, 0)); // Color�󂯎��p�A�J���[�A3ch�A�\���p
	Mat colorGray(480, 640, CV_8UC3, Scalar(25, 50, 200)); // �O���[�X�P�[���A3ch�A�������o�p
	Mat colorBinary(480, 640, CV_8UC1, Scalar(0)); // 2�l�A�L�����u���[�V�����p

	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(640,480), true); // ����o�͗p�i���j
	
	// Load haar-like cascade classifier

	RealSenseAPI realSense;
	realSense.initialize();

	ProcessUtil util;
	util.initialize();

	namedWindow("bin");
	namedWindow("ir", WINDOW_AUTOSIZE);
	namedWindow("color", WINDOW_AUTOSIZE);

	while (1)
	{
		char key = waitKey(1);

		realSense.queryIRImage(irGray, irBinary, 100);

		Mat element(3, 3, CV_8UC1); // �t�B���^�T�C�Y
		erode(irBinary, irBinary, element, Point(-1, -1), 2); // ���k(�m�C�Y����)�A�Ώۃs�N�Z���̋ߖT�̂����ő�
		dilate(irBinary, irBinary, element, Point(-1, -1), 3); // �c���i�����j�A�Ώۃs�N�Z���̋ߖT�̂����ŏ�

		realSense.queryColorImage(colorColor, colorGray, colorBinary, 100);

		vector<Rect> faces;
		util.getFaces(colorGray, faces);

		vector<Rect> eyes;
		util.getEyes(colorGray, eyes);

		util.renderRects(colorColor, faces, Scalar(255, 0, 0));
		util.renderRects(colorColor, eyes, Scalar(255, 255, 0));

		vector<Rect> pupils;
		util.getPupils(irBinary, pupils);
		util.renderRects(colorColor,pupils, Scalar(255, 0, 255));

		writer << irGray;

		cv::imshow("color", colorColor);
		cv::imshow("ir", irGray);
		cv::imshow("bin", irBinary);

		if (key == 'q')
		{
			break;
		}
	}

	cv::destroyAllWindows();

	return 0;
}
