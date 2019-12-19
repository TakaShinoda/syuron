
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")


#include "stdafx.h"
#include <opencv2/opencv.hpp>
#include <opencv2/highgui.hpp>
#include <vector>

using namespace cv;
using namespace std;

int main()
{
	CascadeClassifier cascade;
	cascade.load("C:/Users/hm140/Desktop/project_m/sample/Sample/Sample/cascade_lbp_rgb.xml"); //正面顔情報が入っているカスケードファイル読み込み
	Mat img = imread("C:/Users/hm140/Desktop/project_m/sample/Sample/Sample/demo.png", IMREAD_UNCHANGED); //入力画像読み込み
	vector<Rect> faces; //輪郭情報を格納場所
	cascade.detectMultiScale(img, faces, 1.1, 1, 0, Size(40, 40)); //カスケードファイルに基づいて顔を検知する．検知した顔情報をベクトルfacesに格納

	for (int i = 0; i<faces.size(); i++) //検出した顔の個数"faces.size()"分ループを行う
	{
		rectangle(img, Point(faces[i].x, faces[i].y), Point(faces[i].x + faces[i].width, faces[i].y + faces[i].height), Scalar(0, 0, 255), 3, CV_AA); //検出した顔を赤色矩形で囲む
	}

	imshow("detect face", img);
	waitKey(0);
}
