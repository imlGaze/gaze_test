#pragma once

#include <opencv2\opencv.hpp>
#include<vector>

using namespace cv;
using std::vector;

class ProcessUtil {
public:
	void initialize();

	void getFaces(Mat graySrc, vector<Rect> &faces);
	void getEyes(Mat graySrc, vector<Rect> &eyes);

	bool getMaxRect(vector<Rect> rects, Rect &result);
	void renderRects(Mat dst, vector<Rect> rects, Scalar color = Scalar(255, 255, 255), int thickness = 2);

	bool getPupils(Mat binSrc, vector<Rect> &pupils); // “µ‚ÌˆÊ’u

private:
	CascadeClassifier cascade_face;
	CascadeClassifier cascade_eye;

};