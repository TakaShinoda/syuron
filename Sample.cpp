
#pragma comment(linker, "/SUBSYSTEM:WINDOWS /ENTRY:mainCRTStartup")


#include "stdafx.h"

#include<stdio.h>
#include <OpenNI.h> // OpenNI2 Header file
#include <NiTE.h> // NITE2 Header file
#include <opencv2/opencv.hpp>
#include <opencv/highgui.h>

#include <GL/gl.h>
#include <GL/glu.h>
#include <GL/glut.h>

#include <string.h>

#include <mmsystem.h>
#pragma comment(lib,"winmm.lib") //"winmm.libというライブラリをリンかでリンクする

#define RANGE 180


//画面のサイズ
#define WIDTH 640 
#define HEIGHT 480

//物理演算
#define g 9.8
#define v0 


#if 1

using namespace cv;
using namespace std;

//ball 平行移動用
float x = 0.0f;
float trans_x = 0.0f; //ボールのx軸の動き
float trans_y = 0.0f; //ボールのy軸の動き


float scale_x = 0.2; //ボールの大きさ
float scale_y = 0.2;
float scale_z = 0.2;

bool flag = false;


//ボール
GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 }; //緑
GLfloat lightpos[] = { 200.0, 150.0, -500.0, 1.0 }; //ライトの位置

bool baseballFlag = false; //ボールを出さない
bool soccerFlag = false;
bool soccerFlag_up = false;
bool baseballFlag_re = false; //restart
bool soccerFlag_re = false;

//物理演算
double F, a;//力と加速度
double y = 0;//y座標
double t = 0;//時間

//背景
cv::Mat img, img2;




void printString(float x, float y, char* str, int length) {
	float z = -1.0f;
	glRasterPos3f(x, y, z);

	for (int i = 0; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
	}
}

//文字列描画
static void DrawString(String str, int w, int h, int x0, int y0)
{
	glColor3d(0, 250, 0); // 画面上にテキスト描画
	glRasterPos2f(x0, y0); //文字列の位置(左上原点のスクリーン座標系, 文字列の左下がこの位置になる)

	int size = (int)str.size(); //str:文字列
	for (int i = 0; i < size; ++i) {
		char ic = str[i];
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ic);
	}
}

//OpenNI2/NITE2を使ってKinectのColor,Depth,User,Skeleton,Combination,Combination_PC,Ballを表示する
void display(void)
{


	//背景画像
	img = cv::imread("baseball_back.jpg", 1);
	img2 = cv::imread("soccer_back.jpg", 1);

	//エラー処理
	if (img.data == NULL) {
		printf("img read error!\n");
	}

	cv::setUseOptimized(true);

	// Initialize OpenNI2
	openni::OpenNI::initialize();

	// Initialize NITE2 
	nite::NiTE::initialize();



	// Device Open
	openni::Status statusOpenNI = openni::STATUS_OK;
	openni::Device device;
	statusOpenNI = device.open(openni::ANY_DEVICE);
	if (statusOpenNI != openni::STATUS_OK) {
		cerr << "Error : openni::Device::open" << endl;
		return;
	}

	// Color Stream Create and Open
	openni::VideoStream colorStream;
	colorStream.create(device, openni::SENSOR_COLOR);
	statusOpenNI = colorStream.start();
	if (statusOpenNI != openni::STATUS_OK) {
		cerr << "Error : openni::VideoStream::start( COLOR )" << endl;
		return;
	}


	// Depth Stream Create and Open
	openni::VideoStream depthStream;
	depthStream.create(device, openni::SENSOR_DEPTH);
	statusOpenNI = depthStream.start();
	if (statusOpenNI != openni::STATUS_OK) {
		cerr << "Error : openni::VideoStream::start( DEPTH )" << endl;
		return;
	}

	// User Tracker Create and Open
	nite::Status statusNITE = nite::STATUS_OK;
	nite::UserTracker userTracker;
	statusNITE = userTracker.create(&device);
	if (statusNITE != nite::STATUS_OK) {
		cerr << "Error : nite::UserTracker::create" << endl;
		return;
	}

	// Setting enable Synchronize
	device.setDepthColorSyncEnabled(true);

	// Set Registration Mode Depth to Color
	// But Kinect doesn't support this Function!
	device.setImageRegistrationMode(openni::IMAGE_REGISTRATION_DEPTH_TO_COLOR);


	cv::Mat grayMat;
	cv::Mat colorMat;
	cv::Mat depthMat;
	cv::Mat userMat;
	cv::Mat skeletonMat;

	//Windowのサイズ
	cv::namedWindow("Color", cv::WINDOW_NORMAL);
	cv::namedWindow("Grey", cv::WINDOW_NORMAL);
	cv::namedWindow("Depth", cv::WINDOW_NORMAL);
	cv::namedWindow("User", cv::WINDOW_NORMAL);
	cv::namedWindow("Skeleton", cv::WINDOW_NORMAL);

	// Table of Colors
	cv::Vec3b color[7];
	color[0] = cv::Vec3b(255, 255, 255);
	color[1] = cv::Vec3b(255, 0, 0);
	color[2] = cv::Vec3b(0, 255, 0);
	color[3] = cv::Vec3b(0, 0, 255);
	color[4] = cv::Vec3b(255, 255, 0);
	color[5] = cv::Vec3b(255, 0, 255);
	color[6] = cv::Vec3b(0, 255, 255);

	float tmp_x[15];
	float tmp_y[15];
	float tmp_z[15];

	int baseball_status = -1;
	int soccer_status = -1;

	while (1) {
		// Retrieve Frame from Color Stream (8bit 3channel)
		openni::VideoFrameRef colorFrame;
		colorStream.readFrame(&colorFrame); // Retrieve a Frame from Stream
		//cv::cvtColor(入力画像, 出力画像, 色空間を変換する値);
		if (colorFrame.isValid()) {
			colorMat = cv::Mat(colorStream.getVideoMode().getResolutionY(), colorStream.getVideoMode().getResolutionX(), CV_8UC3, reinterpret_cast<uchar*>(const_cast<void*>(colorFrame.getData()))); // Retrieve a Data from Frame 
			cv::cvtColor(colorMat, colorMat, CV_RGB2BGR); // Change the order of the pixel RGB to BGR 
			
			cv::cvtColor(colorMat, grayMat, CV_BGR2GRAY); // Change the order of the pixel RGB to GRAY

		}


		// Retrieve Frame from Depth Stream (16bit 1channel)
		openni::VideoFrameRef depthFrame;
		depthStream.readFrame(&depthFrame); // Retrieve a Frame from Stream
		if (depthFrame.isValid()) {
			depthMat = cv::Mat(depthStream.getVideoMode().getResolutionY(), depthStream.getVideoMode().getResolutionX(), CV_16UC1, reinterpret_cast<ushort*>(const_cast<void*>(depthFrame.getData()))); // Retrieve a Data from Frame
			depthMat.convertTo(depthMat, CV_8UC1, -255.0f / 10000, 255.0);
			// Convert the pixel 0~10000 to 0~255
		}

		// Retrieve User Frame from UserTracker
		nite::UserTrackerFrameRef userFrame;
		userTracker.readFrame(&userFrame); // Retrive a Frame form Tracker
		const nite::UserId* pUserId = userFrame.getUserMap().getPixels();
		// Retrive UserId from Frame
		int width = userFrame.getUserMap().getWidth();
		int height = userFrame.getUserMap().getHeight();

		userMat = cv::Mat(height, width, CV_8UC3, cv::Scalar(255, 255, 255));//画像を格納

		if (userFrame.isValid()) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					userMat.at<cv::Vec3b>(y, x) = color[*pUserId]; //色を付ける
					pUserId++;
				}
			}
		}

		// Retrieve Skeleton Frame from UserTracker
		skeletonMat = cv::Mat(height, width, CV_8UC3, cv::Scalar(255, 255, 255));
		const nite::Array<nite::UserData>& users = userFrame.getUsers(); // Retrieve User from User Frame
		for (int count = 0; count < users.getSize(); count++) {
			// Start Skeleton Tracking a new User
			if (users[count].isNew()) {
				userTracker.startSkeletonTracking(users[count].getId());
			}
			// Retrieve Skeleton from Tracking User ( who is Not Lost and Visible User )
			else if (!users[count].isLost() && users[count].isVisible()) {
				const nite::Skeleton& skeleton = users[count].getSkeleton(); // Retrieve Skeleton form User
				if (skeleton.getState() == nite::SkeletonState::SKELETON_TRACKED) {
					for (int position = 0; position < 15; position++) {
						const nite::SkeletonJoint& joint = skeleton.getJoint((nite::JointType)position);
						// Retrieve Joint from Skeleton ( Total 14 joint )
						const nite::Point3f& point = joint.getPosition();
						// Retrieve three-dimensional position of the Joint
						cv::Point2f registPoint;
						userTracker.convertJointCoordinatesToDepth(point.x, point.y, point.z, &registPoint.x, &registPoint.y); // Registration Joint Position to Depth
						cv::circle(skeletonMat, registPoint, 10, static_cast<cv::Scalar>(color[count + 1]), -1, CV_AA);

						tmp_x[position] = point.x;
						tmp_y[position] = point.y;
						tmp_z[position] = point.z;


						//動きの判別
						if (position == 8 && (baseball_status == -1 || soccer_status == -1)) { //初期化
							if ((abs(tmp_y[2] - tmp_y[3]) < 100) && ((abs(tmp_y[4] - tmp_y[5]) < 150))) {
								//気を付けの姿勢
								trans_x = 0;
								trans_y = 0;
								baseball_status = 0;
								soccer_status = 0;
								printf("Junbi\n");
							}
						}

						printf("xxyyyyyy:%d %d %d\n", position, (int)point.x, (int)point.y);

						//野球（右手）//RANGE180 1=首 4=左ひじ 5=右ひじ 6=左手 7=右手 8=胴体の中心
						if (position == 8 && baseball_status == 0 && soccerFlag == false) { //野球モード
							if (((abs(tmp_x[4] - tmp_x[8]) < RANGE) && (abs(tmp_y[4] - tmp_y[8]) < RANGE) && ((abs(tmp_x[5] - tmp_x[8]) < RANGE) && (abs(tmp_y[5] - tmp_y[8]) < RANGE)) && ((abs(tmp_y[1] - tmp_y[6]) < RANGE) && ((abs(tmp_y[1] - tmp_y[7]) < RANGE))))) {  //両手がお腹あたりにくる
								baseball_status = 1;
								printf("Kamae\n");
							}
						}

						if (position == 7 && baseball_status == 1 && soccer_status == 0) {
							if (point.y > tmp_y[0]) { //右手が頭より上にくる
								if (position == 7 && tmp_y[6] < tmp_y[7]) {
									baseballFlag = true;
									baseball_status = 2;
								}
								printf("Tewoageru\n");
							}
						}

						//サッカー（右脚）
						if (position == 12 && soccer_status == 0 && baseballFlag == false) {
							if (tmp_y[12] > tmp_y[10] - 300) { //右膝が右腰より上 <-ココを分類器を用いて認識する
								soccerFlag = true;
								soccer_status = 1;
								printf("Hizawoageru\n");
							}
						}

						printf("(10)%f,  (11)%f,  (13)%f,  (14)%f\n", tmp_y[10], tmp_y[11], tmp_y[13], tmp_y[14]);
						printf("(9)%f,  (12)%f,\n", tmp_y[9], tmp_y[12]);

						//最初に戻る
						if (trans_x > 100 || trans_y < -50) {
							baseballFlag = false;
							soccerFlag = false;
							baseball_status = 0;
							soccer_status = 0;
							trans_x = 0;
							trans_y = 0;

							scale_x = 0.2; //元のボールの大きさを戻す
							scale_y = 0.2;
							scale_z = 0.2;

							t = 0; //初期化
							soccerFlag_up = false;
							printf("Modoru\n");
						}
					}
				}
			}
		}


		//ボールの描画
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WIDTH, HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();


		//視野角,アスペクト比(ウィンドウの幅/高さ),描画する範囲(最も近い距離,最も遠い距離)
		gluPerspective(30.0, (double)WIDTH / (double)HEIGHT, 1.0, 1000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//視点の設定
		gluLookAt(150.0, 100.0, -200.0, //カメラの座標
			0.0, 0.0, 0.0, // 注視点の座標
			0.0, 1.0, 0.0); // 画面の上方向を指すベクトル

							//ライトの設定
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

		//マテリアルの設定
		glMaterialfv(GL_FRONT, GL_DIFFUSE, green);

		//ボールの表示
		if (baseballFlag || soccerFlag) {
			//野球
			if (baseballFlag == true && soccerFlag == false) {

				glTranslatef(trans_x - 30, trans_y + 55, 0.0f); //ボールが出てくる位置
				glScalef(scale_x, scale_y - 0.05, scale_z); //ボールの拡大縮小 glScalef(0.2, 0.15, 0.2)
				glutSolidSphere(40.0, 16, 16);

				scale_x = scale_x * 0.88; //ボール縮小
				scale_y = scale_y * 0.88;
				scale_z = scale_z * 0.88;

				trans_x += 12.f; //ボールがx軸に沿って動く
								 //oto
				PlaySound(L"baseball.wav", NULL, SND_FILENAME | SND_SYNC | SND_ASYNC); //サウンド
				printf("Baseball\n");

			}

			//サッカー
			if (soccerFlag == true && baseballFlag == false) {
				glTranslatef(trans_x - 35, trans_y, 0.0f); //ボールが出てくる位置
				glScalef(0.4, 0.3, 0.4); //ボールの拡大
				glutSolidSphere(40.0, 16, 16);

				if (soccerFlag_up == true && trans_y > 0) { //2回目以降（振り出しに戻す）
					t = 0; //初期化
					soccerFlag_up = false;
				}

				if (trans_y >= 0) { //ボールが膝以上の高さ
					trans_y = 25 * t - 0.5 * g * t * t;
				}
				else if (trans_y < 0) { //ボールが膝より下の高さ
					if (soccer_status == 1) { //ボールが落ちた一瞬
						soccerFlag_up = true;
						t = 0; //初期化
					}

					if (soccerFlag_up == true) { //脚を上げる
						trans_y = 35.4 * t - 0.5 * g * t * t; //35.4 = 25 * 1.414
						trans_y += 5.0; //ボールが上がる
										//oto
						PlaySound(L"soccer.wav", NULL, SND_FILENAME | SND_SYNC | SND_ASYNC); //サウンド
					}
					else {
						trans_y = -5 * 1.2 * t; //ボールが落ち続ける
					}

					printf("Soccer\n");
				}
				t += 0.8;
				soccer_status = 0;
			}

			int bits = glutGet(GLUT_WINDOW_BUFFER_SIZE);

		}

		//背景の切り替え
		GLint oldMatrixMode;

		glGetIntegerv(GL_MATRIX_MODE, &oldMatrixMode);
		glMatrixMode(GL_PROJECTION);
		glPushMatrix();
		glLoadIdentity();
		glRasterPos2i(-1, -1);
		glPixelZoom(1, 1);

		glDrawPixels(img.cols, img.rows, GL_RGB, GL_UNSIGNED_BYTE, img.data);
		glPopMatrix();
		glMatrixMode(oldMatrixMode);
		glClear(GL_DEPTH_BUFFER_BIT);

		char str[20];

		sprintf(str, "%d", baseball_status);


		/*
		glPushMatrix(); //status表示
		printString(80, 65, str, strlen(str));
		glPopMatrix();

		*/

		//OpenGLで描画された画面をOpenCVのcv::Mat形式の画像として取得する
		//Ball (Combination)
		int type = CV_8UC3;   //３チャンネルのデータ
		int format = GL_BGR_EXT;

		glReadBuffer(GL_FRONT);
		cv::Mat out_img(cv::Size(WIDTH, HEIGHT), type);
		glReadPixels(0, 0, WIDTH, HEIGHT, format, GL_UNSIGNED_BYTE, out_img.data);
		cv::flip(out_img, out_img, 0);
		for (int i = 0; i < 3 * WIDTH; i++) {
			for (int j = 0; j < HEIGHT; j++) {
				if (userMat.at<uchar>(j, i) != 0 && out_img.at<uchar>(j, i) == 0) {
					if (baseball_status >= 1) {
						out_img.at<uchar>(j, i) = img.at<uchar>(j, i);
					}
					else if (soccerFlag == true) {
						out_img.at<uchar>(j, i) = img2.at<uchar>(j, i);
					}
				}//Matの要素を取得
				else if (out_img.at<uchar>(j, i) == 0) {
					out_img.at<uchar>(j, i) = userMat.at<uchar>(j, i);
					//Matの要素を取得
				}
			}
		}

		cv::transpose(out_img, out_img); //画像を90度回転

		resize(out_img, out_img, Size(), 3, 1.5); //画面のサイズの拡大縮小
		glutSwapBuffers();



		//画面表示 //imshow(ウィンドウ名, 表示される画像);
		cv::imshow("Color", colorMat); //カラー表示
		cv::imshow("Gray", grayMat); //グレースケール表示
		cv::imshow("Depth", depthMat);
		cv::imshow("User", userMat);
		cv::imshow("Skeleton", skeletonMat);
		cv::imshow("Combination", out_img); //openGLとopenCVの組み合わせ
		cv::imshow("Combination_PC", out_img); //PC用Combination
											   
											   
		
		//分類器の為のグレースケール背景
		cv::Mat bg = cv::imread("haikei.png");
		//張り付ける位置
		int x = 300, y = 150;
		//背景からはみ出しいないかチェック
		CV_Assert((x >= 0) && (x + colorMat.cols <= bg.cols));
		CV_Assert((y >= 0) && (y + colorMat.rows <= bg.rows));
		//colorMatのサイズ変更
		cv::resize(colorMat, colorMat, cv::Size(), 0.3, 0.3);
		//前面画像の画素を背景にコピー
		cv::Mat roi = bg(cv::Rect(x, y, colorMat.cols, colorMat.rows));
		colorMat.copyTo(roi);
		//colorMatをグレースケール化
		cv::cvtColor(bg, bg, CV_BGR2GRAY);
		cv::imshow("cascade", bg);

		// Press the Escape key to Exit
		if (cv::waitKey(30) == VK_ESCAPE) {
			break;
		}
	}

	// Shutdown Application
	cv::destroyAllWindows();
	colorStream.stop();
	depthStream.stop();
	colorStream.destroy();
	depthStream.destroy();
	userTracker.destroy();
	device.close();
	openni::OpenNI::shutdown();
	nite::NiTE::shutdown();
}

void idle(void)
{
	if (flag) { x -= 0.5f; }
	else { x += 0.5f; }
	if (x > 50.0f)flag = true;
	if (x < -50.0f)flag = false;
	Sleep(1);
	glutPostRedisplay();
}

void Init() {
	glClearColor(0.3f, 0.3f, 0.3f, 1.0f);
	glEnable(GL_DEPTH_TEST);
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
}

int main(int argc, char *argv[])
{
	glutInitWindowPosition(100, 100);
	glutInitWindowSize(WIDTH, HEIGHT);
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE);
	glutCreateWindow("Ball");
	glutDisplayFunc(display);

	glutIdleFunc(idle);
	Init();
	glClearColor(0, 0, 0, 0);
	glutMainLoop();

	return 0;
}

#endif