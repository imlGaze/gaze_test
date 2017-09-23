#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"RealSenseAPI.h"
#include"ProcessUtil.h"
#include<vector>

#include "const.h"

using namespace cv;
using std::vector;

int do_photo()
{
	Mat irGray(IR_HEIGHT, IR_WIDTH, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、表示用
	Mat irBinary(IR_HEIGHT, IR_WIDTH, CV_8UC1, Scalar(0)); // 2値、輪郭抽出用

	Mat colorColor(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC3, Scalar(0, 0, 0)); // Color受け取り用、カラー、3ch、表示用
	Mat colorGray(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、特徴抽出用
	Mat colorBinary(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC1, Scalar(0)); // 2値、キャリブレーション用

	RealSenseAPI realSense;
	if (!realSense.initialize()) {
		std::cout << "ERROR: No device was found" << std::endl;
	}
	// realSense.setLaserPower(2);

	ProcessUtil util;
	util.initialize();

	namedWindow("irGray", WINDOW_AUTOSIZE);
	namedWindow("colorColor", WINDOW_AUTOSIZE);

	int counter = 0;

	while (1)
	{
		char key = waitKey(1);

		if (!realSense.queryIRImage(irGray, irBinary, 100)) {
			// No signal
			irGray = Scalar(25, 50, 200);
			putText(irGray, "No signal", Point(30, 30), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255), 1);
		}
		if (!realSense.queryColorImage(colorColor, colorGray, colorBinary, 100)) {
			// No signal
			colorColor = Scalar(0, 0, 0);
			putText(colorColor, "No signal", Point(30, 30), CV_FONT_HERSHEY_SIMPLEX, 1, Scalar(255), 1);
		}

		cv::imshow("colorColor", colorColor);
		cv::imshow("irGray", irGray);

		if (key == 'q')
		{
			break;
		}
		else if (key == ' ') {
			Mat renderColorColor = colorColor.clone();
			Mat renderIrGray = irGray.clone();

			Size patternSize = Size(CHESS_COLS, CHESS_ROWS);
			auto test = [patternSize](Mat gray, Mat &render) {
				cvtColor(gray, gray, CV_BGR2GRAY);

				vector<Point2f> corners;
				if (findChessboardCorners(gray, patternSize, corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK)) {
					cornerSubPix(gray, corners, Size(CHESS_SIZE, CHESS_SIZE), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

					drawChessboardCorners(render, patternSize, Mat(corners), true);
				}
			};

			test(colorGray, renderColorColor);
			test(irGray, renderIrGray);

			imshow("color", renderColorColor);
			imshow("gray", renderIrGray);

			while (1) {
				char k = waitKey(1);

				if (k == ' ') {
					char colorName[32];
					sprintf_s(colorName, "color_%d.png", counter);
					imwrite(colorName, colorColor);

					char irName[32];
					sprintf_s(irName, "ir_%d.png", counter);
					imwrite(irName, irGray);

					counter++;
					break;
				}
				else if (k == 'x') {
					break;
				}
			}
		}
		else if (key == 'r') {
			realSense.initialize();
			// realSense.setLaserPower(2);
		}
	}

	cv::destroyAllWindows();
	return 0;
}
