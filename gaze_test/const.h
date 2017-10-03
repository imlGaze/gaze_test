#pragma once
#include<opencv2\opencv.hpp>

const int IR_WIDTH = 640;
const int IR_HEIGHT = 480;
const int COLOR_WIDTH = 640;
const int COLOR_HEIGHT = 480;

const int IMAGE_COUNT = 30;
const int IMAGE_WIDTH = 640;
const int IMAGE_HEIGHT = 480;
const cv::Size IMAGE_SIZE = cv::Size(IMAGE_WIDTH, IMAGE_HEIGHT);
const int CHESS_SIZE = 20/2;

const int CHESS_COLS = 10;
const int CHESS_ROWS = 7;
const cv::Size CHESS_PATTERN = cv::Size(CHESS_COLS, CHESS_ROWS);

const bool CALIB_SKIP_CHECK = 1;
