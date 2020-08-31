#include "opencv2/core/core.hpp"
#include "highgui.h"
#include <stdio.h>
#include <math.h>
#include <string.h>
#include <iostream>
#include "opencv2/imgproc.hpp"
#include "circle.h"
#include "triangle.h"
#include "rectangle.h"

using namespace cv;
using namespace std;


int main()
{
	string str;
	getline(cin, str);
	if (str == "圆形")
		circle();
	
	else if (str == "矩形")
		rectangle();

	else if(str == "三角形")
		triangle();
}
