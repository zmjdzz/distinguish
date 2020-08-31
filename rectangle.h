//“矩形检测”程序。
//它按顺序加载了几张图像，并试图找到正方形

#include "opencv2/core/core.hpp"
#include "opencv2/imgproc/imgproc.hpp"
#include "opencv2/highgui/highgui.hpp"

#include <iostream>
#include <math.h>
#include <string.h>

using namespace cv;
using namespace std;

int thresh1 = 50, N = 5;
const char* wndname1 = "4.png";


//求出向量夹角的余弦
//从pt0->pt1到pt0->pt2
static double angle(Point pt1, Point pt2, Point pt0)//angle函数用来返回（两个向量之间找到角度的余弦值）
{
	double dx1 = pt1.x - pt0.x;
	double dy1 = pt1.y - pt0.y;
	double dx2 = pt2.x - pt0.x;
	double dy2 = pt2.y - pt0.y;
	return (dx1*dx2 + dy1 * dy2) / sqrt((dx1*dx1 + dy1 * dy1)*(dx2*dx2 + dy2 * dy2) + 1e-10);
}


//返回在图像上检测到的正方形序列。
//序列存储在指定的内存中
static void findSquares(const Mat& image, vector<vector<Point> >& squares)
{
	squares.clear();//数组清空
	//按比例放大图像，滤除噪声
	//中值滤波将增强边缘检测
	Mat timg(image);
	medianBlur(image, timg, 9);//中值滤波
	Mat gray0(timg.size(), CV_8U), gray;

	vector<vector<Point> > contours;//vector容器里面放了一个vector容器，子容器里放点,绘制轮廓

	//在图像的每个颜色平面上找到正方形
	for (int c = 0; c < 3; c++)
	{
		int ch[] = { c, 0 };
		mixChannels(&timg, 1, &gray0, 1, ch, 1);//将输入数组的指定通道复制到输出数组的指定通道

		//尝试几个阈值级别
		for (int l = 0; l < N; l++)
		{
			// Canny帮助捕捉带有渐变阴影的正方形
			if (l == 0)
			{
				Canny(gray0, gray, 5, thresh, 5);
				dilate(gray, gray, Mat(), Point(-1, -1));//膨胀
			}
			else
			{
				gray = gray0 >= (l + 1) * 255 / N;
			}

			findContours(gray, contours, RETR_LIST, CHAIN_APPROX_SIMPLE);//对图像进行处理,将检测出的每个轮廓（相互独立）以点集（拐点）的形式存储到contours中

			vector<Point> approx;//储存矩形4个顶点的坐标的数组

			for (size_t i = 0; i < contours.size(); i++)
			{
				//近似轮廓与精度成正比
				//到轮廓周长
				approxPolyDP(Mat(contours[i]), approx, arcLength(Mat(contours[i]), true)*0.02, true);//输出封闭的多边形顶点点集

				//判断是否为四边形条件
				if (approx.size() == 4 &&
					fabs(contourArea(Mat(approx))) > 1000 &&//取绝对值
					isContourConvex(Mat(approx)))  //凸性检测 检测一个曲线是不是凸的
				{
					double maxCosine = 0;

					for (int j = 2; j < 5; j++)
					{
						double cosine = fabs(angle(approx[j % 4], approx[j - 2], approx[j - 1]));//计算夹角余弦值
						maxCosine = MAX(maxCosine, cosine);
					}

					if (maxCosine < 0.3)//如果三个夹角余弦值都小于0.3，则说明是矩形，并存储矩形
						squares.push_back(approx);
				}
			}
		}
	}
}


static void drawSquares(Mat& image, const vector<vector<Point> >& squares)//绘制矩形
{
	for (size_t i = 0; i < squares.size(); i++)
	{
		const Point* p = &squares[i][0];

		int n = (int)squares[i].size();
		if (p->x > 3 && p->y > 3)
			polylines(image, &p, &n, 1, true, Scalar(0, 0, 255), 3, LINE_AA);
	}
	imshow(wndname, image);
}


void rectangle()
{
	static const char* names[] = { "4.png",0 };
	namedWindow(wndname, 1);
	vector<vector<Point> > squares;

	for (int i = 0; names[i] != 0; i++)
	{
		Mat image = imread(names[i], 1);
		if (image.empty())
		{
			cout << "Couldn't load " << names[i] << endl;
			continue;
		}

		findSquares(image, squares);
		drawSquares(image, squares);
		imwrite("out.jpg", image);
		int c = waitKey();
		if ((char)c == 27)
			break;
	}

}
