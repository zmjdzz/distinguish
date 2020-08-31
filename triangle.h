#include "cv.h"
#include "highgui.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include "opencv2/imgproc.hpp"

using namespace cv;
using namespace std;

int thresh = 50;
IplImage* img = NULL;
IplImage* img0 = NULL;
CvMemStorage* storage = NULL;
const char * wndname = "4.png";


// 两个向量之间找到角度的余弦值
// from pt0->pt1 and from pt0->pt2
double angle(CvPoint* pt1, CvPoint* pt2, CvPoint* pt0)
{
	double dx1 = pt1->x - pt0->x;
	double dy1 = pt1->y - pt0->y;
	double dx2 = pt2->x - pt0->x;
	double dy2 = pt2->y - pt0->y;
	return (dx1*dx2 + dy1 * dy2) / sqrt((dx1*dx1 + dy1 * dy1)*(dx2*dx2 + dy2 * dy2) + 1e-10);
}

// 返回图像中找到的轮廓序列
// 序列存储在内存存储器中
CvSeq* findSquares4(IplImage* img, CvMemStorage* storage)
{
	CvSeq* contours;//创建序列
	int i, c, l, N = 11;
	CvSize sz = cvSize(img->width & -2, img->height & -2);//定义了矩形框大小,以像素为精度

	IplImage* timg = cvCloneImage(img);//拷贝了原图用来处理(函数内部会额外分配内存，将源数据全部复制过来，包括ROI区域，使用前无需申请，在for循环中使用会消耗内存，每次用完需要用cvRelease来释放)
	IplImage* gray = cvCreateImage(sz, 8, 1);// 创建灰度图,创建头并分配数据,宽高,位深度,通道
	IplImage* pyr = cvCreateImage(cvSize(sz.width / 2, sz.height / 2), 8, 3);//创建1/4图大小的三通道图像
	IplImage* tgray;
	CvSeq* result;
	//double s, t;
	// 创建一个空序列用于存储轮廓和角点
	CvSeq* squares = cvCreateSeq(0, sizeof(CvSeq), sizeof(CvPoint), storage);

	cvSetImageROI(timg, cvRect(0, 0, sz.width, sz.height));
	// 过滤噪音
	cvPyrDown(timg, pyr, 7);//使用Gaussian金字塔分解对输入图像向下采样 ,为了过滤噪音
	cvPyrUp(pyr, timg, 7);//使用Gaussian金字塔分解对输入图像向上采样 ,为了过滤噪音
	tgray = cvCreateImage(sz, 8, 1);

	// find squares in every color plane of the image
	for (c = 0; c < 3; c++)//红绿蓝三色分别尝试提取
	{
		// 提取 the c-th color plane
		cvSetImageCOI(timg, c + 1);//设置图像的COI的函数，coi为0时表示选择所有通道,1第一通道以此类推
		cvCopy(timg, tgray, 0);//对单通道数组应用固定阈值操作。该函数的典型应用是对灰度图像进行阈值操作得到二值图像。(cvCmpS 也可以达到此目的) 或者是去掉噪声，例如过滤很小或很大象素值的图像 

		// try several threshold levels
		for (l = 0; l < N; l++)
		{
			// hack: use Canny instead of zero threshold level.
			// Canny helps to catch squares with gradient shading  
			if (l == 0)
			{
				// apply Canny. Take the upper threshold from slider
				// and set the lower to 0 (which forces edges merging)
				cvCanny(tgray, gray, 0, thresh, 5);
				// dilate canny output to remove potential
				// holes between edge segments
				cvDilate(gray, gray, 0, 1);
			}
			else
			{
				// apply threshold if l!=0:
				//     tgray(x,y) = gray(x,y) < (l+1)*255/N ? 255 : 0
				cvThreshold(tgray, gray, (l + 1) * 255 / N, 255, CV_THRESH_BINARY);
			}

			// 找到所有轮廓并且存储在序列中
			cvFindContours(gray, storage, &contours, sizeof(CvContour),
				CV_RETR_LIST, CV_CHAIN_APPROX_SIMPLE, cvPoint(0, 0));

			// test each contour
			while (contours)
			{
				// approximate contour with accuracy proportional
				// to the contour perimeter
				result = cvApproxPoly(contours, sizeof(CvContour), storage,
					CV_POLY_APPROX_DP, cvContourPerimeter(contours)*0.02, 0);
				// 三角形轮廓应当有3个顶点,3条边
				// relatively large area (过滤干扰的轮廓)
				// cvCheckContourConvexity保证是凸面的
				// cvContourArea计算三角形区域面积，去掉一些不相干的区域
				if (result->total == 3 &&
					fabs(cvContourArea(result, CV_WHOLE_SEQ)) > 1000 &&
					fabs(cvContourArea(result, CV_WHOLE_SEQ)) < 100000 &&
					cvCheckContourConvexity(result))
				{


					for (i = 0; i < 3; i++)
						cvSeqPush(squares,
						(CvPoint*)cvGetSeqElem(result, i));
				}

				// 继续查找下一个轮廓
				contours = contours->h_next;
			}
		}
	}

	// 释放所有临时图像
	cvReleaseImage(&gray);
	cvReleaseImage(&pyr);
	cvReleaseImage(&tgray);
	cvReleaseImage(&timg);

	return squares;
}


// 绘制三角形
void drawSquares(IplImage* img, CvSeq* squares)
{
	CvSeqReader reader;
	IplImage* cpy = cvCloneImage(img);
	int i;

	// initialize reader of the sequence
	cvStartReadSeq(squares, &reader, 0);

	// read 4 sequence elements at a time (all vertices of a square)
	for (i = 0; i < squares->total; i += 3)
	{
		CvPoint pt[3], *rect = pt;
		int count = 3;

		// 读取三个顶点
		CV_READ_SEQ_ELEM(pt[0], reader);
		CV_READ_SEQ_ELEM(pt[1], reader);
		CV_READ_SEQ_ELEM(pt[2], reader);

		// cvPolyLine函数画出三角形轮廓
		cvPolyLine(cpy, &rect, &count, 1, 1, CV_RGB(0, 0, 255), 3, CV_AA, 0);
	}

	// show the resultant image
	cvShowImage(wndname, cpy);
	cvReleaseImage(&cpy);
}




void triangle()
{

	char* names[] = { (char*)"4.png", (char*)"pic2.png", (char*)"pic3.png",
	 (char*)"pic4.png", (char*)"pic5.png", (char*)"pic6.png",(char*)"pic7.png", (char*)"pic8.png",
	 (char*)"pic9.png",(char*)"pic10.png", (char*)"pic11.png", (char*)"pic12.png",0 };
	int i, c;
	// create memory storage that will contain all the dynamic data
	storage = cvCreateMemStorage(0);

	for (i = 0; names[i] != 0; i++)
	{
		img0 = cvLoadImage(names[i], 1);
		if (!img0)
		{
			cout << "不能载入" << names[i] << "继续下一张图片" << endl;
			continue;
		}
		img = cvCloneImage(img0);

		cvNamedWindow(wndname, 1);

		drawSquares(img, findSquares4(img, storage));
		c = cvWaitKey(0);
		// 释放图像
		cvReleaseImage(&img);
		cvReleaseImage(&img0);
		// 清空图像释放空间
		cvClearMemStorage(storage);
		if ((char)c == 27)
			break;
	}

	cvDestroyWindow(wndname);

}


