#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"RealSenseAPI.h"
#include<vector>

using namespace cv;
using std::vector;

Mat gamma(float gamma) {
	Mat lut(256, 1, CV_8UC1);
	for (int i = 0; i < 256; i++) {
		lut.data[i] = 255 * pow((float) i / 255, 1.0 / gamma);
	}

	return lut;
}

int main()
{
	Mat gamma2 = gamma(2.0);

	Mat irImage16U(480, 640, CV_16UC1, Scalar(0)); // IR�󂯎��p�A�O���[�X�P�[���A1ch
	Mat irImage8U(480, 640, CV_8UC1, Scalar(0)); // 8bit�ϊ��p�A�O���[�X�P�[���A1ch
	Mat irImage8UC3(480, 640, CV_8UC3, Scalar(25, 50, 200)); // �O���[�X�P�[���A3ch�A�\���p
	Mat binImage(480, 640, CV_8UC1, Scalar(0)); // 2�l�A�֊s���o�p

	Mat colorImage8U(480, 640, CV_8UC3, Scalar(0, 0, 0)); // Color�󂯎��p�A�J���[�A3ch�A�\���p
	Mat colorImage8UG(480, 640, CV_8UC3, Scalar(25, 50, 200)); // �O���[�X�P�[���A3ch�A�������o�p

	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(640,480), true); // ����o�͗p�i���j
	
	// Load haar-like cascade classifier
	CascadeClassifier casc_reye("haarcascade_lefteye_2splits.xml"); // ���ړ���
	CascadeClassifier casc_leye("haarcascade_righteye_2splits.xml"); // �E�ړ���

	RealSenseAPI realSense;
	realSense.initialize();

												 /* �E�C���h�E���쐬 */
	namedWindow("bin");
	namedWindow("ir", WINDOW_AUTOSIZE);
	namedWindow("color", WINDOW_AUTOSIZE);
	waitKey(1);
	int loopCount = 0;

	int maxArea = 800;
	int minArea = 70;
	int thresh = 100;

	while (1)
	{
		vector<vector<Point>> contours;
		vector<Rect> leyes, reyes;
		char key = waitKey(1);

		// IR for gaze detection
		realSense.queryImage(irImage16U, ResponseType::IR);
		irImage16U.convertTo(irImage8U, CV_8UC1); // 16bit -> 8bit
		//irImage16U.convertTo(irImage8UC3, CV_8UC3);
		cvtColor(irImage8U, irImage8UC3, CV_GRAY2BGR); // 1ch(Y) -> 3ch(Y, Y, Y)

		irImage8U = ~irImage8U; // Invert Black and white
		threshold(irImage8U, binImage, thresh, 255, CV_THRESH_BINARY);
		Mat element(3, 3, CV_8UC1); // �t�B���^�T�C�Y
		erode(binImage, binImage, element); // ���k(�m�C�Y����)�A�Ώۃs�N�Z���̋ߖT�̂����ő�
		erode(binImage, binImage, element);
		dilate(binImage, binImage, element); // �c���i�����j�A�Ώۃs�N�Z���̋ߖT�̂����ŏ�
		dilate(binImage, binImage, element);
		dilate(binImage, binImage, element);

		// Color for face/eye detection
		realSense.queryImage(colorImage8U, ResponseType::COLOR);
		cvtColor(colorImage8U, colorImage8UG, CV_RGB2GRAY);
		cvtColor(colorImage8UG, colorImage8UG, CV_GRAY2BGR);

		// imwrite("pre.png", colorImage8UG);
		LUT(colorImage8UG, gamma2, colorImage8UG);
		// imwrite("post.png", colorImage8UG);

		casc_leye.detectMultiScale(colorImage8UG, leyes);

		std::cout << "(" << leyes.size() << ',' << reyes.size() << ')';
		int leyeIndex = -1, reyeIndex = -1;
		int leyeMaxArea = 0, reyeMaxArea = 0;
		for (int i = 0, n = leyes.size(); i < n; i++) {
			int area = leyes[i].width * leyes[i].height;
			if (leyeMaxArea < area) {
				leyeIndex = i;
				leyeMaxArea = area;
			}
		}

		if (leyeIndex != -1) {
			Rect leye = leyes[leyeIndex];
			casc_reye.detectMultiScale(colorImage8UG(Rect(0, 0, leye.x, 480)), reyes);
			
			for (int i = 0, n = reyes.size(); i < n; i++) {
				if (reyes[i].x + reyes[i].width < leye.x) {
					int area = reyes[i].width * reyes[i].height;
					if (reyeMaxArea < area) {
						reyeIndex = i;
						reyeMaxArea = area;
					}
				}
			}

			rectangle(colorImage8UG, leye, cv::Scalar(255, 128, 0), 2);
			line(colorImage8UG, Point(leye.x, 0), Point(leye.x, 480), Scalar(255, 255, 255), 2);
		}

		if (leyeIndex != -1 && reyeIndex != -1) {
			Rect leye = leyes[leyeIndex];
			Rect reye = reyes[reyeIndex];

			rectangle(colorImage8UG, reye, cv::Scalar(128, 255, 0), 2);
			/*
			// �֊s(Contour)���o�ARETR_EXTERNAL�ōł��O���̂݁ACHAIN_APPROX_NONE�ł��ׂĂ̗֊s�_�i�֊s���\������_�j���i�[
			cv::findContours(binImage(leye), contours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // TODO: Color-IR�L�����u���[�V����
			for (int j = 0, n = contours.size(); j < n; j++)
			{
				if (cv::contourArea(contours[j])>maxArea || cv::contourArea(contours[j])<minArea) // �֊s�T�C�Y�t�B���^�i���j
				{
					continue;
				}
				Moments moment = cv::moments(contours[j]); // �֊s�d�S�t�B���^�i���j
				Point point = Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
				if (point.x < 48 || 640 - 48 * 2 < point.x)
				{
					continue;
				}

				if (point.y < 50 || 480 - 50 * 2 < point.y)
				{
					continue;
				}

				Rect rect = boundingRect(contours[j]) + Point(leye.x, leye.y); // �_�̏W���ɊO�ڂ���X���Ă��Ȃ���`�����߂�
				rectangle(colorImage8UG, rect, cv::Scalar(255, 0, 255), 2);
			}
			*/
		}

		cv::imshow("color", colorImage8UG);
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

		//writer << irImage8UC3;
		writer << colorImage8UG;
	}

	cv::destroyAllWindows();

	return 0;

}
