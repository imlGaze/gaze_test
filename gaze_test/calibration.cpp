#include"calibration.h"

#include<iostream>
#include<vector>
#include"const.h"
#include"ProcessUtil.h"

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

void createCalibrationParams(char *prefix, vector<vector<Vec3f>> &object_points, vector<vector<Point2f>> &image_points, bool handCalibrate = false)
{
	vector<Mat> raw;
	vector<Mat> gray;
	load_images(prefix, raw, gray);

	namedWindow("image", WINDOW_AUTOSIZE);
	std::cout << raw.size() << " -> (" << std::flush;

	Size patternSize = Size(CHESS_COLS, CHESS_ROWS);
	int handSkip = 0, autoSkip = 0;
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
						image_points.push_back(image_corners);
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
				image_points.push_back(image_corners); // TODO: INDEX‚ÌŠK‘w‚ª•sˆÀiIMAGE_INDEX/WINDOW_INDEX‚È‚Ì‚©AWINDOW/INDEX/IMAGE_INDEX‚È‚Ì‚©j
			}
		}
		else {
			autoSkip++;
		}
	}
	cv::destroyAllWindows();

	std::cout << ") -> " << image_points.size() << "/" << handSkip << "/" << autoSkip << std::endl;

	for (int i = 0, n = image_points.size(); i < n; i++) {
		vector<Vec3f> of_image;
		for (int row = 0; row < patternSize.height; row++) {
			for (int col = 0; col < patternSize.width; col++) {
				of_image.push_back(Vec3f(col*CHESS_SIZE, row*CHESS_SIZE, 0.0));
			}
		}
		object_points.push_back(of_image);
	}

}

void do_calibration(Mat &colorCameraMatrix, Mat &colorDistCoeffs, Mat &colorR, Mat &colorT, Mat &irCameraMatrix, Mat &irDistCoeffs, Mat &irR, Mat &irT, Mat &R, Mat &T) {
	vector<vector<Vec3f>> colorOp;
	vector<vector<Point2f>> colorIp;
	vector<vector<Vec3f>> irOp;
	vector<vector<Point2f>> irIp;

	createCalibrationParams("color", colorOp, colorIp);
	createCalibrationParams("ir", irOp, irIp);

	// Mat irCameraMatrix(3, 3, CV_32FC1);
	// Mat colorCameraMatrix(3, 3, CV_32FC1);
	// Mat colorDistCoeffs(1, 4, CV_32FC1);
	// Mat irDistCoeffs(1, 4, CV_32FC1);
	const Size size = Size(IMAGE_WIDTH, IMAGE_HEIGHT);

	vector<Mat> colorRVecs, colorTVecs;
	vector<Mat> irRVecs, irTVecs;
	std::cout << "Begin calibrate color" << std::endl;
	calibrateCamera(colorOp, colorIp, size, colorCameraMatrix, colorDistCoeffs, colorRVecs, colorTVecs);
	std::cout << "Begin calibrate ir" << std::endl;
	calibrateCamera(irOp, irIp, size, irCameraMatrix, irDistCoeffs, irRVecs, irTVecs);

	Mat dR, dT, E, F;
	// irCameraMatrix, irDistCoeffs
	// colorCameraMatrix, colorDistCoeffs
	// R, T, E, F
	std::cout << "Begin stereo calibrate" << std::endl;
	stereoCalibrate(irOp, irIp, colorIp, irCameraMatrix, irDistCoeffs, colorCameraMatrix, colorDistCoeffs, Size(IMAGE_WIDTH, IMAGE_HEIGHT), dR, dT, E, F);

	R = dR;
	T = dT;

	// colorR = colorRVecs[0];
	Rodrigues(colorRVecs[0], colorR);
	colorT = colorTVecs[0];
	// irR = colorR + dR;
	// irT = colorT + dT;

	//irR = irRVecs[0];
	Rodrigues(irRVecs[0], irR);
	irT = irTVecs[0];

}