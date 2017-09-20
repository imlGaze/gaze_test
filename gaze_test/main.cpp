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
	Mat irGray(480, 640, CV_8UC3, Scalar(25, 50, 200)); // グレースケール、3ch、表示用
	Mat irBinary(480, 640, CV_8UC1, Scalar(0)); // 2値、輪郭抽出用

	Mat colorColor(480, 640, CV_8UC3, Scalar(0, 0, 0)); // Color受け取り用、カラー、3ch、表示用
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

	while (1)
	{
		char key = waitKey(1);

		realSense.queryIRImage(irGray, irBinary, 100);

		Mat element(3, 3, CV_8UC1); // フィルタサイズ
		erode(irBinary, irBinary, element, Point(-1, -1), 2); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
		dilate(irBinary, irBinary, element, Point(-1, -1), 3); // 膨張（強調）、対象ピクセルの近傍のうち最小

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
