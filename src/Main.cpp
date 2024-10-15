#include "Main.hpp"

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

#include "Camera.hpp"
#include "Renderer.hpp"
#include "VoxelGrid.hpp"

#include "Tracker.hpp"
#include "Histogram.hpp"


// control
static int mouse_x;
static int mouse_y;
static bool mouse_left_down;
static bool mouse_right_down;

// window
static int currentWindow = 0;

// switches
static bool topView = false;
static bool rotateView = false;
static bool videoEnd = false;

static bool showLines = true;
static bool showBoxes = true;

// OpenCV
static std::vector<cv::Mat> gFrames;
static std::vector<cv::Mat> gBackgrounds;
static std::vector<cv::Mat> gBackgroundsHSV;
static std::vector<cv::Mat> gForegrounds;
static std::vector<cv::Mat> gForegroundMasks;
static std::vector<cv::VideoCapture> gCaptures;

// classes
static std::shared_ptr<Tracker> gTracker;
static std::shared_ptr<Renderer> gRenderer;
static std::shared_ptr<VoxelGrid> gVoxelGrid;
static std::vector<std::shared_ptr<Camera>> gCameras;
static std::vector<std::shared_ptr<Histogram>> gHistograms;

static cv::Mat getFG(cv::Mat& fgFrame, cv::Mat& bgFrame) {
	cv::Mat bgOut;
	//// define temporary parameters
	//cv::Size size = fgFrame.size(); // size of frame

	//cv::Mat bgOut = cv::Mat(size, CV_8U, 1);
	//cv::Mat bgTemp = cv::Mat(size, CV_8U, 1);
	//cv::Mat fgTemp = cv::Mat(size, CV_8U, 1);

	//// Colour transformation from rgb to hsv for the background model
	//// -> this part could also be outside this function because now it is performed for every new
	////		frame, while it only has to be done once the background frame is captured!

	////IplImage* b_HSV = cvCreateImage( sz, 8, 3);	// definition and allocation
	////cvCvtColor(BGframe,b_HSV,CV_BGR2HSV);	// color conversion
	//std::vector<cv::Mat> bgChannels;
	//cv::split(bgFrame, bgChannels);

	//// Colour transformation from rgb to hsv for the current frame
	//cv::Mat fgHSV = fgFrame.clone();
	//cv::cvtColor(fgFrame, fgHSV, cv::COLOR_BGR2HSV); // color conversion

	//std::vector<cv::Mat> fgChannels;
	//cv::split(fgHSV, fgChannels);

	//// Definition of the thresholds
	//int thresholdH = 25;
	//int thresholdS = 40;
	//int thresholdV = 65;

	//double maxval = 255.0;

	//// BACKGROUND SUBTRACTION

	//// initialize background by h-channel
	//cv::absdiff(fgChannels[0], bgChannels[0], fgTemp);
	//cv::threshold(fgTemp, bgOut, thresholdH, maxval, cv::THRESH_BINARY);

	//// update background by s-channel
	//cv::absdiff(fgChannels[1], bgChannels[1], fgTemp);
	//cv::threshold(fgTemp, bgTemp, thresholdS, maxval, cv::THRESH_BINARY);
	//cv::bitwise_and(bgOut, bgTemp, bgOut);

	//// update background by v-channel
	//cv::absdiff(fgChannels[2], bgChannels[2], fgTemp);
	//cv::threshold(fgTemp, bgTemp, thresholdV, maxval, cv::THRESH_BINARY);
	//cv::bitwise_or(bgOut, bgTemp, bgOut);

	//// erosion kernel
	//cv::Mat element_d = cv::getStructuringElement(cv::MorphShapes::MORPH_CROSS, cv::Size2f(5, 5), cv::Point(3, 3)); // CV_SHAPE_CROSS CV_SHAPE_RECT CV_SHAPE_ELLIPSE

	//// Erosion & dilation to connect blobs and remove noisy points
	//cv::erode(bgOut, bgOut, 0);
	//cv::dilate(bgOut, bgOut, element_d, cv::Point(-1, -1), 2);
	//cv::erode(bgOut, bgOut, 0);

	return bgOut;
}

static void display() {
	// render scene
	gRenderer->Render(gVoxelGrid);
	
	// draw lines from camera into scene
	if (showLines) {
		gTracker->LabelLines();
	}

	// draw label grid in the scene
	if (showBoxes) {
		gTracker->LabelGrids();
	}
	
	// swap buffers
	gRenderer->Flush();
}

static void quit() {
	cv::destroyAllWindows();
	exit(0);
}

static void keyboard(unsigned char key, int x, int y) {
	switch (key) {
		case 'q':
			quit();
			break;

		case 't':
			topView = !topView;
			break;

		case 'v':
			gRenderer->ShowVolume = !gRenderer->ShowVolume;
			break;

		case 'g':
			gRenderer->ShowFloor = !gRenderer->ShowFloor;
			break;

		case 'c':
			gRenderer->ShowCamera = !gRenderer->ShowCamera;
			break;
		
		case 'o':
			gRenderer->ShowOrigin = !gRenderer->ShowOrigin;
			break;
		
		case 'r':
			rotateView = !rotateView;
			break;

		case '1':
			currentWindow = 0;
			break;

		case '2':
			currentWindow = 1;
			break;

		case '3':
			currentWindow = 2;
			break;

		case '4':
			currentWindow = 3;
			break;
		
		case 'l':
			showLines = !showLines;
			break;

		case 'b':
			showBoxes = !showBoxes;
			break;
	}
}

static void mouse(int button, int state, int x, int y) {
	if (state == GLUT_DOWN && button == GLUT_LEFT_BUTTON) {
		mouse_left_down = true;
	}
	else {
		mouse_left_down = false;
	}

	if (state == GLUT_DOWN && button == GLUT_RIGHT_BUTTON) {
		mouse_right_down = true;
	}
	else {
		mouse_right_down = false;
	}
}

static void motion(int x, int y) {
	if (mouse_left_down) {
		gRenderer->MoveScene(x - mouse_x, y - mouse_y);
	}

	if (mouse_right_down) {
		gRenderer->Zoom(-(y - mouse_y)/2);
	}

    mouse_x = x;
    mouse_y = y;
}

static void passive_motion(int x, int y) {
	mouse_x = x;
	mouse_y = y;
}

static void reshape(int width, int height) {
	gRenderer->Resize(width, height);
}

static void update(int value = 0) {
	// get frame from videos
	for (int i = 0; i < NumViews; ++i) {
		gCaptures[i].read(gFrames[i]);

		if (gFrames[i].empty()) {
			quit();
		}

		gForegroundMasks[i] = getFG(gFrames[i], gBackgroundsHSV[i]);
		gCameras[i]->Foreground = gForegroundMasks[i];

		gHistograms[i]->CreateColorHistogram(gFrames[i], gForegroundMasks[i]);
		gForegrounds[i] = gTracker->ExtractForeground(gFrames[i], gForegroundMasks[i]);
	}

	gRenderer->NumFrames++;
	gVoxelGrid->UpdateVoxels(gCameras);

	// find the persons in the 3D grid
	gTracker->TrackPersons(gForegrounds, NumViews);

	// label persons
	gTracker->LabelVoxels(gVoxelGrid, gTracker->PersonPosA, 350.f, 750.f, CV_RGB(0, 200, 0));
	gTracker->LabelVoxels(gVoxelGrid, gTracker->PersonPosB, 350.f, 750.f, CV_RGB(0, 0, 200));

	cv::imshow("Video Feed", gFrames[currentWindow]);
	cv::imshow("Foreground", gForegrounds[currentWindow]);
	
	cv::imshow("Camera Color Histogram", gHistograms[currentWindow]->GetRenderedImage());
	cv::imshow("Image Histogram A", gTracker->GetImageHistogramA(currentWindow).GetRenderedImage());
	cv::imshow("Image Histogram B", gTracker->GetImageHistogramB(currentWindow).GetRenderedImage());

	if (topView) {
		gRenderer->ViewIndex(currentWindow);
	}
	else if (gRenderer->CameraView) {
		gRenderer->ResetTopView();
	}

	if (!topView) {
		rotateView = false;
		gRenderer->ViewAngle = 0;
	}

	// render the scene
	if (rotateView) {
		gRenderer->RotateView();
	}

	char key = cv::waitKey(2);
	keyboard(key, 0, 0);

	glutSwapBuffers();

	// clean up
	for (int i = 0; i < NumViews; ++i) {
		gForegroundMasks[i].release();
		gForegrounds[i].release();
	}

	glutTimerFunc(18, update, 0);
}

static void my_glut_init() {
	glEnable(GL_DEPTH_TEST);
	glHint(GL_POLYGON_SMOOTH_HINT, GL_NICEST);

	glMatrixMode(GL_PROJECTION);
	glLoadIdentity();
	gluPerspective(60, 800 / 600, 1, 20000);

	glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
	glEnable(GL_COLOR_MATERIAL);
}

static void idle() {
	glutPostRedisplay();
}

static void initialize_glut(int argc, char **argv) {
	glutInit(&argc, argv);
	glutInitDisplayMode(GLUT_RGBA | GLUT_DOUBLE | GLUT_DEPTH);
	glutInitWindowSize(800, 600);
	glutInitWindowPosition(700, 10);
	glutCreateWindow("Voxel Representation");

	my_glut_init();

	glutTimerFunc(8, update, 0);
	glutIdleFunc(idle);
	glutDisplayFunc(display);
	glutKeyboardFunc(keyboard);
	glutMouseFunc(mouse);
	glutMotionFunc(motion);
	glutPassiveMotionFunc(passive_motion);
	glutReshapeFunc(reshape);
}


int main(int argc, char** argv) {
	gFrames = std::vector<cv::Mat>(NumViews);
	gBackgrounds = std::vector<cv::Mat>(NumViews);
	gBackgroundsHSV = std::vector<cv::Mat>(NumViews);
	gForegrounds = std::vector<cv::Mat>(NumViews);
	gForegroundMasks = std::vector<cv::Mat>(NumViews);
	gCaptures = std::vector<cv::VideoCapture>(NumViews);

	gCameras = std::vector<std::shared_ptr<Camera>>(NumViews);
	gHistograms = std::vector<std::shared_ptr<Histogram>>(NumViews);

	// histograms
	for (int i = 0; i < NumViews; ++i) {
		gHistograms.push_back(std::make_shared<Histogram>());
	}

	// load camera calibration files
	gCameras.push_back(std::make_shared<Camera>(0, ViewWidth, ViewHeight, "data/camparam_f.ini"));
	gCameras.push_back(std::make_shared<Camera>(1, ViewWidth, ViewHeight, "data/camparam_l.ini"));
	gCameras.push_back(std::make_shared<Camera>(2, ViewWidth, ViewHeight, "data/camparam_r.ini"));
	gCameras.push_back(std::make_shared<Camera>(3, ViewWidth, ViewHeight, "data/camparam_s.ini"));

	// background image
	gBackgrounds.push_back(cv::imread("data/background_f.bmp", 1));
	gBackgrounds.push_back(cv::imread("data/background_l.bmp", 1));
	gBackgrounds.push_back(cv::imread("data/background_r.bmp", 1));
	gBackgrounds.push_back(cv::imread("data/background_s.bmp", 1));

	// the input videos
	gCaptures.push_back(cv::VideoCapture("data/video_f.avi"));
	gCaptures.push_back(cv::VideoCapture("data/video_l.avi"));
	gCaptures.push_back(cv::VideoCapture("data/video_r.avi"));
	gCaptures.push_back(cv::VideoCapture("data/video_s.avi"));

	// background in HSV
	for (int i = 0; i < NumViews; i ++) {
		gBackgroundsHSV.push_back(cv::Mat(gBackgrounds[i].size(), 8, 3));
		cv::cvtColor(gBackgrounds[i], gBackgroundsHSV[i], cv::COLOR_BGR2HSV);
	}

	gRenderer = std::make_shared<Renderer>(NumViews);
	
	// pass init image to tracker
	gTracker = std::make_shared<Tracker>(cv::imread("data/init-person1.jpg", 1), cv::imread("data/init-person2.jpg", 1), gCameras);

	// transfer camera coordinate to 3D scene
	for (int i = 0; i < NumViews; i ++) {
		gRenderer->CamCoord(gCameras[i]->Corners);
	}

	// volumetric reconstruction
	gVoxelGrid = std::make_shared<VoxelGrid>(ViewWidth, ViewHeight, gCameras);

	initialize_glut(argc, argv);
	
	// initialize windows
	cv::namedWindow("Video Feed", 1);
	cv::namedWindow("Foreground", 1);
	cv::namedWindow("Camera Color Histogram", 1);
	cv::namedWindow("Image Histogram A", 1);
	cv::namedWindow("Image Histogram B", 1);

	// from now on it's just events
	glutMainLoop();

	return 0;
}
