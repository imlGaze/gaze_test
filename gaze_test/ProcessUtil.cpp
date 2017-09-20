#include "ProcessUtil.h"

void ProcessUtil::initialize() {
	cascade_face = CascadeClassifier("haarcascade_frontalface_alt.xml"); // 正面顔特徴
	cascade_eye = CascadeClassifier("haarcascade_eye.xml"); // 目特徴
}

void ProcessUtil::getFaces(Mat graySrc, vector<Rect> &faces) {
	cascade_face.detectMultiScale(graySrc, faces);
}

void ProcessUtil::getEyes(Mat graySrc, vector<Rect> &eyes) {
	cascade_eye.detectMultiScale(graySrc, eyes);
}

bool ProcessUtil::getMaxRect(vector<Rect> rects, Rect &result) {
	int maxArea = 0;
	int maxIndex = -1;
	for (int i = 0, n = rects.size(); i < n; i++) {
		int size = rects[i].width * rects[i].height;

		if (maxArea < size) {
			maxArea = size;
			maxIndex = i;
		}
	}

	if (maxIndex != -1) {
		result = rects[maxIndex];
		return true;
	}
	return false;
}

void ProcessUtil::renderRects(Mat dst, vector<Rect> rects, Scalar color, int thickness) {
	for (int i = 0, n = rects.size(); i < n; i++) {
		rectangle(dst, rects[i], color, thickness);
	}
}

bool ProcessUtil::getPupils(Mat binSrc, vector<Rect> &pupils) { // 瞳の位置
	Mat element(3, 3, CV_8UC1); // フィルタサイズ
	erode(binSrc, binSrc, element, Point(-1, -1), 2); // 収縮(ノイズ除去)、対象ピクセルの近傍のうち最大
	dilate(binSrc, binSrc, element, Point(-1, -1), 3); // 膨張（強調）、対象ピクセルの近傍のうち最小

	vector<vector<Point>> contours;
	// 輪郭(Contour)抽出、RETR_EXTERNALで最も外側のみ、CHAIN_APPROX_NONEですべての輪郭点（輪郭を構成する点）を格納
	cv::findContours(binSrc, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // TODO: Color-IRキャリブレーション

	for (int j = 0, n = contours.size(); j < n; j++)
	{
		Rect rect = boundingRect(contours[j]); // 点の集合に外接する傾いていない矩形を求める
		pupils.push_back(rect);
	}

	return pupils.size() != 0;
}