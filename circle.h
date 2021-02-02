#pragma once
#include "opencv2/highgui/highgui.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include <iostream>
#include <stdio.h>
#include <string.h>



using namespace cv;
using namespace std;

void circle()
{
	Mat srcImage = imread("第三题图.png");
	Mat midImage;//临时变量定义

	cvtColor(srcImage, midImage, COLOR_BGR2GRAY);//转化为灰度图
	imshow("灰度图", midImage);
	GaussianBlur(midImage, midImage, Size(9, 9), 2, 2);
	// 霍夫圆检测
	vector<Vec3f> circles; // 3通道float型向量
	double dp = 1.5;//用于检测圆心的累加器图像的分辨率与输入图像之比的倒数
	double minDist = 10;//两个圆心之间的最小距离
	double param1 = 100;//Canny边缘检测的较大阈值
	double param2 = 100;//累加器阈值
	int min_radius = 20;//圆形半径的最小值
	int max_radius = 100;//圆形半径的最大值

	HoughCircles(midImage, circles, HOUGH_GRADIENT, 1.5, 10, 100, 100, 0, 0);
	//HoughCircles(midImage, circles, HOUGH_GRADIENT, dp, minDist, param1, param2, min_radius,max_radius);

	// 依次在图中绘制出圆
	for (size_t i = 0; i < circles.size(); i++)
	{
		//参数定义
		Point center(cvRound(circles[i][0]), cvRound(circles[i][1]));
		int radius = cvRound(circles[i][2]);
		//绘制圆心
		circle(srcImage, center, 3, Scalar(0, 255, 0), -1, 8, 0);
		//绘制圆轮廓
		circle(srcImage, center, radius, Scalar(0, 0, 255), 3, 8, 0);
	}
	//显示效果图
	imshow("效果图", srcImage);
	waitKey(0);

}
