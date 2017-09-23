#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"RealSenseAPI.h"
#include"ProcessUtil.h"
#include<vector>

#include "const.h"
#include "photo.h"
#include "calibration.h"

using namespace cv;
using std::vector;

struct Mouse {
	int eventType;
	int x, y;
	int flags;
};

void mouseCallback(int eventType, int x, int y, int flags, void *userData) {
	Mouse *mouse = static_cast<Mouse*>(userData);
	mouse->eventType = eventType;
	mouse->x = x;
	mouse->y = y;
	mouse->flags = flags;
}

int do_main();

int main()
{
	// return do_photo();
	// return do_calibration();
	// do_main();
	do_main2();
}

int do_main2() {
}

int do_main()
{
	Mat irGray(IR_HEIGHT, IR_WIDTH, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、表示用
	Mat irBinary(IR_HEIGHT, IR_WIDTH, CV_8UC1, Scalar(0)); // 2値、輪郭抽出用

	Mat colorColor(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC3, Scalar(0, 0, 0)); // Color受け取り用、カラー、3ch、表示用
	Mat colorGray(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、特徴抽出用
	Mat colorBinary(COLOR_HEIGHT, COLOR_WIDTH, CV_8UC1, Scalar(0)); // 2値、キャリブレーション用

	Mat intrinsic(3, 3, CV_32FC1);
	Mat rotation(1, 3, CV_32FC1);
	Mat translation(1, 3, CV_32FC1);
	Mat distortion(1, 4, CV_32FC1);

	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(IR_WIDTH, IR_HEIGHT), true); // 動画出力用（仮）

	RealSenseAPI realSense;
	if (!realSense.initialize()) {
		std::cout << "ERROR: No device was found" << std::endl;
	}

	ProcessUtil util;
	util.initialize();

	namedWindow("irGray", WINDOW_AUTOSIZE);
	namedWindow("irBinary", WINDOW_AUTOSIZE);
	namedWindow("colorColor", WINDOW_AUTOSIZE);
	namedWindow("colorBinary", WINDOW_AUTOSIZE);

	Mouse irMouse, colorMouse;
	setMouseCallback("irGray", mouseCallback, &irMouse);
	setMouseCallback("colorColor", mouseCallback, &colorMouse);

	Rect lastFace;
	Rect lastLEye, lastREye;

	Rect irClip(0, 0, IR_WIDTH, IR_HEIGHT);
	Rect colorClip(0, 0, IR_WIDTH, IR_HEIGHT);
	bool calibMode = true;
	bool mode = false;
	bool laser = true;
	bool cross = false;


	// TODO: キャリブレーション
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

		Mat irBinaryClip = irBinary(irClip);
		Mat colorBinaryClip = colorBinary(colorClip);

		if (calibMode) {
			if (irMouse.eventType == CV_EVENT_LBUTTONDOWN) {
				printf("IR(%d, %d)\n", irMouse.x, irMouse.y);
				int base = min(min(irMouse.x, IR_WIDTH - irMouse.x) / 4, min(irMouse.y, IR_HEIGHT - irMouse.y) / 3);
				irClip.x = max(0, irMouse.x - base * 4);
				irClip.y = max(0, irMouse.y - base * 3);
				irClip.width = base * 4 * 2;
				irClip.height = base * 3 * 2;

				colorClip = Rect(320 - irClip.width / 2, 240 - irClip.height / 2, irClip.width, irClip.height);
			}

			if (irClip.width != 0) {
				rectangle(irGray, irClip, Scalar(0, 255), 2);

				Point irCenter(irClip.x + irClip.width / 2, irClip.y + irClip.height / 2);
				util.renderPoint(irGray, irCenter, Scalar(255, 0, 255), 2);
				util.renderPoint(colorColor, irCenter, Scalar(255, 0, 255), 2);
			}

			if (colorClip.width != 0) {
				rectangle(colorColor, colorClip, Scalar(0, 255), 2);
			}
		}
		else {
			Mat element(3, 3, CV_8UC1); // フィルタサイズ
			erode(irBinary, irBinary, element, Point(-1, -1), 2); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
			dilate(irBinary, irBinary, element, Point(-1, -1), 3); // 膨張（強調）、対象ピクセルの近傍のうち最小

			vector<Rect> faces;
			util.getFaces(colorGray, faces);
			// util.renderRects(colorColor, faces, Scalar(255, 0, 0));

			Rect face;
			Rect lEye, rEye;

			if (faces.size() != 0) {
				std::sort(faces.begin(), faces.end(), areaIsGreater);
				face = faces[0];
			}
			else {
				face = lastFace;
			}

			if (face.width != 0) {
				Rect upperHalf(face.x, face.y, face.width, face.height / 2);

				vector<Rect> eyes;
				util.getEyes(colorGray(upperHalf), eyes);

				if (eyes.size() >= 2) {
					std::sort(eyes.begin(), eyes.end(), areaIsGreater);

					lEye = (eyes[0].x < eyes[1].x ? eyes[1] : eyes[0]) + Point(face.x, face.y);
					rEye = (eyes[0].x < eyes[1].x ? eyes[0] : eyes[1]) + Point(face.x, face.y);
				}
				else {
					lEye = lastLEye;
					rEye = lastREye;
				}

				if (lEye.width != 0) {
					line(colorColor, Point(upperHalf.x, upperHalf.y + upperHalf.height), Point(upperHalf.x + upperHalf.width, upperHalf.y + upperHalf.height), Scalar(255, 255, 255), 2);
					rectangle(colorColor, face, Scalar(255, 0, 0), 2);

					rectangle(colorColor, lEye, Scalar(255, 255, 0), 2);
					rectangle(colorColor, rEye, Scalar(0, 255, 255), 2);
					rectangle(irGray, lEye, Scalar(255, 255, 0), 2);
					rectangle(irGray, rEye, Scalar(0, 255, 255), 2);

					vector<Rect> pupils;
					util.getPupils(irBinary, pupils);
					util.renderRects(colorColor, pupils, Scalar(255, 0, 255));

					lastLEye = lEye;
					lastREye = rEye;
					lastFace = face;
				}
			}
			writer << irGray;
		}

		if (cross) {
			line(colorColor, Point(320, 0), Point(COLOR_WIDTH / 2, COLOR_HEIGHT), Scalar(255), 2);
			line(colorColor, Point(0, 240), Point(COLOR_WIDTH, COLOR_HEIGHT / 2), Scalar(255), 2);
			line(irGray, Point(320, 0), Point(IR_WIDTH / 2, IR_HEIGHT), Scalar(255), 2);
			line(irGray, Point(0, 240), Point(IR_WIDTH, IR_HEIGHT / 2), Scalar(255), 2);
		}

		cv::imshow("colorColor", colorColor);
		cv::imshow("colorBinary", colorBinary);
		cv::imshow("irGray", irGray);
		cv::imshow("irBinary", irBinary);

		if (key == 'q')
		{
			break;
		}
		else if (key == 'c')
		{
			calibMode = !calibMode;
			std::cout << "Calib: " << (calibMode ? "on" : "off") << std::endl;
		}
		else if (key == 'x')
		{
			cross = !cross;
			std::cout << "Cross: " << (cross ? "on" : "off") << std::endl;
		}
		else if (key == 'l')
		{
			laser = !laser;
			std::cout << "Laser: " << (laser ? "on" : "off") << std::endl;
			realSense.setLaserPower(laser ? 1 : 0);
		}
	}

	cv::destroyAllWindows();

	return 0;
}
