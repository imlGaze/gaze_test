#include "ProcessUtil.h"

void ProcessUtil::initialize() {
	cascade_face = CascadeClassifier("haarcascade_frontalface_alt.xml"); // ���ʊ����
	cascade_eye = CascadeClassifier("haarcascade_eye.xml"); // �ړ���
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

bool ProcessUtil::getPupils(Mat binSrc, vector<Rect> &pupils) { // ���̈ʒu
	Mat element(3, 3, CV_8UC1); // �t�B���^�T�C�Y
	erode(binSrc, binSrc, element, Point(-1, -1), 2); // ���k(�m�C�Y����)�A�Ώۃs�N�Z���̋ߖT�̂����ő�
	dilate(binSrc, binSrc, element, Point(-1, -1), 3); // �c���i�����j�A�Ώۃs�N�Z���̋ߖT�̂����ŏ�

	vector<vector<Point>> contours;
	// �֊s(Contour)���o�ARETR_EXTERNAL�ōł��O���̂݁ACHAIN_APPROX_NONE�ł��ׂĂ̗֊s�_�i�֊s���\������_�j���i�[
	cv::findContours(binSrc, contours, RETR_EXTERNAL, CV_CHAIN_APPROX_NONE); // TODO: Color-IR�L�����u���[�V����

	for (int j = 0, n = contours.size(); j < n; j++)
	{
		Rect rect = boundingRect(contours[j]); // �_�̏W���ɊO�ڂ���X���Ă��Ȃ���`�����߂�
		pupils.push_back(rect);
	}

	return pupils.size() != 0;
}