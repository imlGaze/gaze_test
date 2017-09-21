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

	void renderRects(Mat dst, vector<Rect> rects, Scalar color = Scalar(255, 255, 255), int thickness = 2);
	void renderPoint(Mat dst, Point point, Scalar color, int thickness);
	void renderPoints(Mat dst, vector<Point> points, Scalar color, int thickness);

	bool getPupils(Mat binSrc, vector<Rect> &pupils); // “µ‚ÌˆÊ’u

private:
	CascadeClassifier cascade_face;
	CascadeClassifier cascade_eye;

};


inline bool areaIsGreater(Rect a, Rect b) {
	return a.area() > b.area();
}