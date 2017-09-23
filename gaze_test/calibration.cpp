#include<iostream>
#include<opencv2\opencv.hpp>
#include"ProcessUtil.h"
#include<vector>
#include"const.h"

using namespace cv;
using std::vector;



void load_images(char *prefix, vector<Mat> &raw, vector<Mat> &gray) {
	for (int i = 0; i < IMAGE_COUNT; i++) {
		char name[128];
		sprintf_s(name, "calib\\%s_%d.png", prefix, i);

		Mat rawImage = imread(name);
		Mat grayImage(rawImage.size(), CV_8UC1);
		cvtColor(rawImage, grayImage, CV_BGR2GRAY);

		raw.push_back(rawImage);
		gray.push_back(grayImage);
	}
}

bool createCalibrationParams(char *prefix, vector<vector<Vec3f>> &object_points, vector<vector<Point2f>> &image_points, bool handCalibrate)
{
	vector<Mat> raw;
	vector<Mat> gray;
	load_images(prefix, raw, gray);

	namedWindow("image", WINDOW_AUTOSIZE);
	std::cout << raw.size() << " -> (" << std::flush;

	Size patternSize = Size(CHESS_COLS, CHESS_ROWS);
	int handSkip = 0, autoSkip = 0;
	vector<vector<Point2f>> corners;
	for (int i = 0; i < IMAGE_COUNT; i++) {
		vector<Point2f> image_corners;
		if (findChessboardCorners(gray[i], patternSize, image_corners, CALIB_CB_ADAPTIVE_THRESH + CALIB_CB_NORMALIZE_IMAGE + CALIB_CB_FAST_CHECK)) {
			cornerSubPix(gray[i], image_corners, Size(CHESS_SIZE, CHESS_SIZE), Size(-1, -1), TermCriteria(CV_TERMCRIT_EPS + CV_TERMCRIT_ITER, 30, 0.1));

			drawChessboardCorners(raw[i], patternSize, Mat(image_corners), true);
			if (handCalibrate) {
				while (1) {
					imshow("image", raw[i]);

					char key = waitKey(1);
					if (key == ' ') {
						corners.push_back(image_corners);
						std::cout << i << ',' << std::flush;
						break;
					}
					else if (key == 'x') {
						handSkip++;
						break;
					}
				}
			}
			else {
				corners.push_back(image_corners);
			}
		}
		else {
			autoSkip++;
		}
	}
	cv::destroyAllWindows();

	std::cout << ") -> " << corners.size() << "/" << handSkip << "/" << autoSkip << std::endl;

	vector<vector<Vec3f>> object_points;
	for (int i = 0, n = corners.size(); i < n; i++) {
		vector<Vec3f> of_image;
		for (int row = 0; row < patternSize.height; row++) {
			for (int col = 0; col < patternSize.width; col++) {
				of_image.push_back(Vec3f(col*CHESS_SIZE, row*CHESS_SIZE, 0.0));
			}
		}
		object_points.push_back(of_image);
	}

}

void do_calibration(Mat &R, Mat &T) {
	vector<vector<Vec3f>> irOp;
	vector<vector<Point2f>> irIp;
	vector<vector<Vec3f>> colorOp;
	vector<vector<Point2f>> colorIp;

	createCalibrationParams("ir", irOp, irIp, false);
	createCalibrationParams("color", colorOp, colorIp, false);

	Mat irCameraMatrix(3, 3, CV_32FC1);
	Mat irDistCoeffs(1, 4, CV_32FC1);
	Mat colorCameraMatrix(3, 3, CV_32FC1);
	Mat colorDistCoeffs(1, 4, CV_32FC1);
	Size size = Size(IMAGE_WIDTH, IMAGE_HEIGHT);
	Mat E, F;

	// vector<Mat> rvecs, tvecs;
	// calibrateCamera(object_points, corners, Size(IMAGE_WIDTH, IMAGE_HEIGHT), cameraMatrix, distCoeffs, rvecs, tvecs);

	// irCameraMatrix, irDistCoeffs
	// colorCameraMatrix, colorDistCoeffs
	// R, T, E, F
	stereoCalibrate(irOp, irIp, colorIp, irCameraMatrix, irDistCoeffs, colorCameraMatrix, colorDistCoeffs, Size(IMAGE_WIDTH, IMAGE_HEIGHT), R, T, E, F);
}