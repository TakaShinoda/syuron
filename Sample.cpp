
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
#pragma comment(lib,"winmm.lib") //"winmm.lib�Ƃ������C�u�������������Ń����N����

#define RANGE 180


//��ʂ̃T�C�Y
#define WIDTH 640 
#define HEIGHT 480

//�������Z
#define g 9.8
#define v0 


#if 1

using namespace cv;
using namespace std;

//ball ���s�ړ��p
float x = 0.0f;
float trans_x = 0.0f; //�{�[����x���̓���
float trans_y = 0.0f; //�{�[����y���̓���


float scale_x = 0.2; //�{�[���̑傫��
float scale_y = 0.2;
float scale_z = 0.2;

bool flag = false;


//�{�[��
GLfloat green[] = { 0.0, 1.0, 0.0, 1.0 }; //��
GLfloat lightpos[] = { 200.0, 150.0, -500.0, 1.0 }; //���C�g�̈ʒu

bool baseballFlag = false; //�{�[�����o���Ȃ�
bool soccerFlag = false;
bool soccerFlag_up = false;
bool baseballFlag_re = false; //restart
bool soccerFlag_re = false;

//�������Z
double F, a;//�͂Ɖ����x
double y = 0;//y���W
double t = 0;//����

//�w�i
cv::Mat img, img2;




void printString(float x, float y, char* str, int length) {
	float z = -1.0f;
	glRasterPos3f(x, y, z);

	for (int i = 0; i < length; i++) {
		glutBitmapCharacter(GLUT_BITMAP_HELVETICA_18, str[i]);
	}
}

//������`��
static void DrawString(String str, int w, int h, int x0, int y0)
{
	glColor3d(0, 250, 0); // ��ʏ�Ƀe�L�X�g�`��
	glRasterPos2f(x0, y0); //������̈ʒu(���㌴�_�̃X�N���[�����W�n, ������̍��������̈ʒu�ɂȂ�)

	int size = (int)str.size(); //str:������
	for (int i = 0; i < size; ++i) {
		char ic = str[i];
		glutBitmapCharacter(GLUT_BITMAP_9_BY_15, ic);
	}
}

//OpenNI2/NITE2���g����Kinect��Color,Depth,User,Skeleton,Combination,Combination_PC,Ball��\������
void display(void)
{


	//�w�i�摜
	img = cv::imread("baseball_back.jpg", 1);
	img2 = cv::imread("soccer_back.jpg", 1);

	//�G���[����
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

	//Window�̃T�C�Y
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
		//cv::cvtColor(���͉摜, �o�͉摜, �F��Ԃ�ϊ�����l);
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

		userMat = cv::Mat(height, width, CV_8UC3, cv::Scalar(255, 255, 255));//�摜���i�[

		if (userFrame.isValid()) {
			for (int y = 0; y < height; y++) {
				for (int x = 0; x < width; x++) {
					userMat.at<cv::Vec3b>(y, x) = color[*pUserId]; //�F��t����
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


						//�����̔���
						if (position == 8 && (baseball_status == -1 || soccer_status == -1)) { //������
							if ((abs(tmp_y[2] - tmp_y[3]) < 100) && ((abs(tmp_y[4] - tmp_y[5]) < 150))) {
								//�C��t���̎p��
								trans_x = 0;
								trans_y = 0;
								baseball_status = 0;
								soccer_status = 0;
								printf("Junbi\n");
							}
						}

						printf("xxyyyyyy:%d %d %d\n", position, (int)point.x, (int)point.y);

						//�싅�i�E��j//RANGE180 1=�� 4=���Ђ� 5=�E�Ђ� 6=���� 7=�E�� 8=���̂̒��S
						if (position == 8 && baseball_status == 0 && soccerFlag == false) { //�싅���[�h
							if (((abs(tmp_x[4] - tmp_x[8]) < RANGE) && (abs(tmp_y[4] - tmp_y[8]) < RANGE) && ((abs(tmp_x[5] - tmp_x[8]) < RANGE) && (abs(tmp_y[5] - tmp_y[8]) < RANGE)) && ((abs(tmp_y[1] - tmp_y[6]) < RANGE) && ((abs(tmp_y[1] - tmp_y[7]) < RANGE))))) {  //���肪����������ɂ���
								baseball_status = 1;
								printf("Kamae\n");
							}
						}

						if (position == 7 && baseball_status == 1 && soccer_status == 0) {
							if (point.y > tmp_y[0]) { //�E�肪������ɂ���
								if (position == 7 && tmp_y[6] < tmp_y[7]) {
									baseballFlag = true;
									baseball_status = 2;
								}
								printf("Tewoageru\n");
							}
						}

						//�T�b�J�[�i�E�r�j
						if (position == 12 && soccer_status == 0 && baseballFlag == false) {
							if (tmp_y[12] > tmp_y[10] - 300) { //�E�G���E������ <-�R�R�𕪗ފ��p���ĔF������
								soccerFlag = true;
								soccer_status = 1;
								printf("Hizawoageru\n");
							}
						}

						printf("(10)%f,  (11)%f,  (13)%f,  (14)%f\n", tmp_y[10], tmp_y[11], tmp_y[13], tmp_y[14]);
						printf("(9)%f,  (12)%f,\n", tmp_y[9], tmp_y[12]);

						//�ŏ��ɖ߂�
						if (trans_x > 100 || trans_y < -50) {
							baseballFlag = false;
							soccerFlag = false;
							baseball_status = 0;
							soccer_status = 0;
							trans_x = 0;
							trans_y = 0;

							scale_x = 0.2; //���̃{�[���̑傫����߂�
							scale_y = 0.2;
							scale_z = 0.2;

							t = 0; //������
							soccerFlag_up = false;
							printf("Modoru\n");
						}
					}
				}
			}
		}


		//�{�[���̕`��
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glViewport(0, 0, WIDTH, HEIGHT);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();


		//����p,�A�X�y�N�g��(�E�B���h�E�̕�/����),�`�悷��͈�(�ł��߂�����,�ł���������)
		gluPerspective(30.0, (double)WIDTH / (double)HEIGHT, 1.0, 1000.0);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();

		//���_�̐ݒ�
		gluLookAt(150.0, 100.0, -200.0, //�J�����̍��W
			0.0, 0.0, 0.0, // �����_�̍��W
			0.0, 1.0, 0.0); // ��ʂ̏�������w���x�N�g��

							//���C�g�̐ݒ�
		glLightfv(GL_LIGHT0, GL_POSITION, lightpos);

		//�}�e���A���̐ݒ�
		glMaterialfv(GL_FRONT, GL_DIFFUSE, green);

		//�{�[���̕\��
		if (baseballFlag || soccerFlag) {
			//�싅
			if (baseballFlag == true && soccerFlag == false) {

				glTranslatef(trans_x - 30, trans_y + 55, 0.0f); //�{�[�����o�Ă���ʒu
				glScalef(scale_x, scale_y - 0.05, scale_z); //�{�[���̊g��k�� glScalef(0.2, 0.15, 0.2)
				glutSolidSphere(40.0, 16, 16);

				scale_x = scale_x * 0.88; //�{�[���k��
				scale_y = scale_y * 0.88;
				scale_z = scale_z * 0.88;

				trans_x += 12.f; //�{�[����x���ɉ����ē���
								 //oto
				PlaySound(L"baseball.wav", NULL, SND_FILENAME | SND_SYNC | SND_ASYNC); //�T�E���h
				printf("Baseball\n");

			}

			//�T�b�J�[
			if (soccerFlag == true && baseballFlag == false) {
				glTranslatef(trans_x - 35, trans_y, 0.0f); //�{�[�����o�Ă���ʒu
				glScalef(0.4, 0.3, 0.4); //�{�[���̊g��
				glutSolidSphere(40.0, 16, 16);

				if (soccerFlag_up == true && trans_y > 0) { //2��ڈȍ~�i�U��o���ɖ߂��j
					t = 0; //������
					soccerFlag_up = false;
				}

				if (trans_y >= 0) { //�{�[�����G�ȏ�̍���
					trans_y = 25 * t - 0.5 * g * t * t;
				}
				else if (trans_y < 0) { //�{�[�����G��艺�̍���
					if (soccer_status == 1) { //�{�[������������u
						soccerFlag_up = true;
						t = 0; //������
					}

					if (soccerFlag_up == true) { //�r���グ��
						trans_y = 35.4 * t - 0.5 * g * t * t; //35.4 = 25 * 1.414
						trans_y += 5.0; //�{�[�����オ��
										//oto
						PlaySound(L"soccer.wav", NULL, SND_FILENAME | SND_SYNC | SND_ASYNC); //�T�E���h
					}
					else {
						trans_y = -5 * 1.2 * t; //�{�[��������������
					}

					printf("Soccer\n");
				}
				t += 0.8;
				soccer_status = 0;
			}

			int bits = glutGet(GLUT_WINDOW_BUFFER_SIZE);

		}

		//�w�i�̐؂�ւ�
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
		glPushMatrix(); //status�\��
		printString(80, 65, str, strlen(str));
		glPopMatrix();

		*/

		//OpenGL�ŕ`�悳�ꂽ��ʂ�OpenCV��cv::Mat�`���̉摜�Ƃ��Ď擾����
		//Ball (Combination)
		int type = CV_8UC3;   //�R�`�����l���̃f�[�^
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
				}//Mat�̗v�f���擾
				else if (out_img.at<uchar>(j, i) == 0) {
					out_img.at<uchar>(j, i) = userMat.at<uchar>(j, i);
					//Mat�̗v�f���擾
				}
			}
		}

		cv::transpose(out_img, out_img); //�摜��90�x��]

		resize(out_img, out_img, Size(), 3, 1.5); //��ʂ̃T�C�Y�̊g��k��
		glutSwapBuffers();



		//��ʕ\�� //imshow(�E�B���h�E��, �\�������摜);
		cv::imshow("Color", colorMat); //�J���[�\��
		cv::imshow("Gray", grayMat); //�O���[�X�P�[���\��
		cv::imshow("Depth", depthMat);
		cv::imshow("User", userMat);
		cv::imshow("Skeleton", skeletonMat);
		cv::imshow("Combination", out_img); //openGL��openCV�̑g�ݍ��킹
		cv::imshow("Combination_PC", out_img); //PC�pCombination
											   
											   
		
		//���ފ�ׂ̈̃O���[�X�P�[���w�i
		cv::Mat bg = cv::imread("haikei.png");
		//����t����ʒu
		int x = 300, y = 150;
		//�w�i����͂ݏo�����Ȃ����`�F�b�N
		CV_Assert((x >= 0) && (x + colorMat.cols <= bg.cols));
		CV_Assert((y >= 0) && (y + colorMat.rows <= bg.rows));
		//colorMat�̃T�C�Y�ύX
		cv::resize(colorMat, colorMat, cv::Size(), 0.3, 0.3);
		//�O�ʉ摜�̉�f��w�i�ɃR�s�[
		cv::Mat roi = bg(cv::Rect(x, y, colorMat.cols, colorMat.rows));
		colorMat.copyTo(roi);
		//colorMat���O���[�X�P�[����
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