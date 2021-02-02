#pragma once
#include "cv.h"
#include "highgui.h"
#include <iostream>


using namespace cv;
using namespace std;

void triangle() {
	Mat srcImage = imread("第三题图.png");
	imshow("原图", srcImage);
	Mat hsv, temp;
	cvtColor(srcImage, hsv, CV_BGR2HSV);
	imshow("1", hsv);
	inRange(hsv, Scalar(0, 43, 46), Scalar(15, 255, 255), temp);
	
	imshow("2", temp);
	
	Mat element = getStructuringElement(MORPH_RECT, Size(2, 2));
	morphologyEx(temp, temp, MORPH_OPEN, element);//开操作，去除一些噪点
	medianBlur(temp, temp, 7);//中值滤波，除噪
	Canny(temp, temp, 20, 80, 3, false);
	imshow("3", temp);

	std::vector<std::vector<Point>> contours;
	std::vector<Vec4i> hierarchy;

	findContours(temp, contours, hierarchy, RETR_TREE, CHAIN_APPROX_SIMPLE, Point(0, 0));
	for (int i = 0; i < contours.size(); i++) {
		drawContours(srcImage, contours, i, Scalar(255, 0, 0), 2, 8, hierarchy, 0, Point(0, 0));
	}
	imshow("output", srcImage);
	waitKey(0);
}
