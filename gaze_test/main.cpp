#include<RealSense\SampleReader.h>
#include<RealSense\SenseManager.h>
#include<iostream>
#include<opencv2\opencv.hpp>
#include"RealSenseAPI.h"
#include"ProcessUtil.h"
#include<vector>

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

int main()
{
	Mat irGrayB(480, 640, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、表示用
	Mat irBinaryB(480, 640, CV_8UC1, Scalar(0)); // 2値、輪郭抽出用

	Mat colorColorB(480, 640, CV_8UC3, Scalar(0, 0, 0)); // Color受け取り用、カラー、3ch、表示用
	Mat colorGray(480, 640, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、特徴抽出用
	Mat colorBinary(480, 640, CV_8UC1, Scalar(0)); // 2値、キャリブレーション用

	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(640,480), true); // 動画出力用（仮）
	
	// Load haar-like cascade classifier

	RealSenseAPI realSense;
	realSense.initialize();

	ProcessUtil util;
	util.initialize();

	namedWindow("bin");
	namedWindow("ir", WINDOW_AUTOSIZE);
	namedWindow("color", WINDOW_AUTOSIZE);

	Mouse irMouse, colorMouse;
	setMouseCallback("ir", mouseCallback, &irMouse);
	setMouseCallback("color", mouseCallback, &colorMouse);

	Rect lastFace;
	Rect lastLEye, lastREye;

	int left, top, right, bottom;
	left = top = right = bottom = 0;
	bool calibMode = true;
	Rect colorR, irR;
	bool mode = false;

	while (1)
	{
		char key = waitKey(1);

		realSense.queryIRImage(irGrayB, irBinaryB, 100);
		realSense.queryColorImage(colorColorB, colorGray, colorBinary, 100);
		
		Mat irGray, irBinary;
		Mat colorColor;

		if (!calibMode) {
			/*
			Rect clip = Rect(max(0, irX.x - 320), max(0, irX.y - 320), min(640, irX.x + 320), min(480, irX.y + 240));
			*/
			resize(irGrayB(irR), irGray, Size(640, 480));
			resize(irBinaryB(irR), irBinary, Size(640, 480));
			resize(colorColorB(colorR), colorColor, Size(640, 480));
		}
		else {
			irGray = irGrayB;
			irBinary = irBinaryB;
			colorColor = colorColorB;
		}

		if (calibMode) {
			if (irMouse.eventType == CV_EVENT_LBUTTONDOWN) {
				std::cout << irMouse.x << ',' << irMouse.y;

				if (!mode) {
					irR.x = irMouse.x;
					irR.y = irMouse.y;
					std::cout << 'L' << std::endl;
				}
				else {
					irR.width = irMouse.x - irR.x;
					irR.height = irMouse.y - irR.y;
					std::cout << 'R' << std::endl;
				}

				mode = !mode;
			}
			if (colorMouse.eventType == CV_EVENT_LBUTTONDOWN) {
				std::cout << colorMouse.x << ',' << colorMouse.y;

				if (!mode) {
					colorR.x = colorMouse.x;
					colorR.y = colorMouse.y;
					std::cout << 'L' << std::endl;
				}
				else {
					colorR.width = colorMouse.x - colorR.x;
					colorR.height = colorMouse.y - colorR.y;
					std::cout << 'R' << std::endl;
				}

				mode = !mode;
			}

			if (colorR.width != 0) {
				rectangle(colorColor, Rect(colorR.x, colorR.y, colorR.width, colorR.height), Scalar(255, 0, 0), 2);
			}
			if (irR.width != 0) {
				rectangle(irGray, Rect(irR.x, irR.y, irR.width, irR.height), Scalar(255, 0, 0), 2);
			}

			line(colorColor, Point(320, 0), Point(320, 480), Scalar(255, 255, 255), 2);
			line(colorColor, Point(0, 240), Point(640, 240), Scalar(255, 255, 255), 2);
			line(irGray, Point(320, 0), Point(320, 480), Scalar(255, 255, 255), 2);
			line(irGray, Point(0, 240), Point(640, 240), Scalar(255, 255, 255), 2);
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
		
		// TODO: キャリブレーション

		cv::imshow("color", colorColor);
		cv::imshow("ir", irGray);
		cv::imshow("bin", irBinary);

		if (key == 'q')
		{
			break;
		}
		else if (key == 'c')
		{
			calibMode = !calibMode;
			std::cout << "Calib: " << (calibMode ? "on" : "off") << std::endl;

			// realSense.setLaserPower(calibMode ? 0 : 1);
		}
		else if (key == 'x') {
			char str[64];
			// sprintf_s(str, "(%d, %d), (%d, %d)", colorX.x, colorX.y, irX.x, irX.y);
			std::cout << str << std::endl;
		}
		else if (key == 'r') {
			left = top = right = bottom = 0;
		}
		else if (key == 'd') {
			char str[64];
			sprintf_s(str, "%d, %d, %d, %d", left, top, right, bottom);
			std::cout << str << std::endl;
		}
	}

	cv::destroyAllWindows();

	return 0;
}
