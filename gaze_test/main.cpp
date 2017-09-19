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

	Mat irImage16U(480, 640, CV_16UC1, Scalar(0)); // IR受け取り用、グレースケール、1ch
	Mat irImage8U(480, 640, CV_8UC1, Scalar(0)); // 8bit変換用、グレースケール、1ch
	Mat irImage8UC3(480, 640, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、表示用
	Mat binImage(480, 640, CV_8UC1, Scalar(0)); // 2値、輪郭抽出用

	Mat colorImage8U(480, 640, CV_8UC3, Scalar(0, 0, 0)); // Color受け取り用、カラー、3ch、表示用
	Mat colorImage8UG(480, 640, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、特徴抽出用

	VideoWriter writer("input.avi", CV_FOURCC_DEFAULT, 30, cv::Size(640,480), true); // 動画出力用（仮）
	
	// Load haar-like cascade classifier
	CascadeClassifier casc_reye("haarcascade_lefteye_2splits.xml"); // 左目特徴
	CascadeClassifier casc_leye("haarcascade_righteye_2splits.xml"); // 右目特徴

	RealSenseAPI realSense;
	realSense.initialize();

												 /* ウインドウを作成 */
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
		Mat element(3, 3, CV_8UC1); // フィルタサイズ
		erode(binImage, binImage, element); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
		erode(binImage, binImage, element);
		dilate(binImage, binImage, element); // 膨張（強調）、対象ピクセルの近傍のうち最小
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
			// 輪郭(Contour)抽出、RETR_EXTERNALで最も外側のみ、CHAIN_APPROX_NONEですべての輪郭点（輪郭を構成する点）を格納
			cv::findContours(binImage(leye), contours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // TODO: Color-IRキャリブレーション
			for (int j = 0, n = contours.size(); j < n; j++)
			{
				if (cv::contourArea(contours[j])>maxArea || cv::contourArea(contours[j])<minArea) // 輪郭サイズフィルタ（仮）
				{
					continue;
				}
				Moments moment = cv::moments(contours[j]); // 輪郭重心フィルタ（仮）
				Point point = Point(moment.m10 / moment.m00, moment.m01 / moment.m00);
				if (point.x < 48 || 640 - 48 * 2 < point.x)
				{
					continue;
				}

				if (point.y < 50 || 480 - 50 * 2 < point.y)
				{
					continue;
				}

				Rect rect = boundingRect(contours[j]) + Point(leye.x, leye.y); // 点の集合に外接する傾いていない矩形を求める
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
