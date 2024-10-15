#include "Renderer.hpp"

#include <vector>
#include <fstream>
#include <iostream>

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

const int Renderer::gridNum = 4;   	// 4 * 4
const int Renderer::gridSize = 480; // 500 pixels
const int Renderer::offsetZ = 5;

float Renderer::eyeX = 0;
float Renderer::eyeY = 0;
float Renderer::eyeZ = 10000;

unsigned int Renderer::width = 644;
unsigned int Renderer::height = 484;

Renderer::Renderer(int vN) : 
numViews(vN), currentView(-1), NumFrames(0), ViewAngle(0), CameraView(false), 
ShowVolume(true), ShowFloor(true), ShowCamera(true), ShowOrigin(true) {
	initGrid();
}

void Renderer::CamCoord(std::vector<cv::Point3f> camIndex) {
	camCorners.push_back(camIndex);
}

void Renderer::ViewIndex(int viewIndex) {
	if (!CameraView) {
		CameraView = true;
	}

	if (currentView != viewIndex) {
		currentView = viewIndex;
		eyeX = camCorners[currentView][0].x;
		eyeY = camCorners[currentView][0].y;
		eyeZ = camCorners[currentView][0].z;

		std::cout << "change view index" << std::endl;
	}
}

void Renderer::RotateView() {
	ViewAngle += 1;
}

void Renderer::ResetTopView() {
	eyeX = 0;
	eyeY = 0;
	eyeZ = 10000;
	CameraView = false;
	currentView = -1;
}

// draw grid, GRID_SIZE*GRID_SIZE pixels/square
// four edges of the ground floor grid, with points
void Renderer::initGrid() {
	// edge 1
	std::vector<cv::Point3f> edge1;
	for (int y = -gridSize * gridNum; y <= gridSize * gridNum; y += gridSize) {
		edge1.push_back(cv::Point3f(
			static_cast<float>(-gridSize * gridNum), 
			static_cast<float>(y), 
			static_cast<float>(offsetZ)));
	}

	// edge 2
	std::vector<cv::Point3f> edge2;
	for (int x = -gridSize * gridNum; x <= gridSize * gridNum; x += gridSize) {
		edge2.push_back(cv::Point3f(
			static_cast<float>(x), 
			static_cast<float>(gridSize * gridNum),
			static_cast<float>(offsetZ)));
	}

	// edge 3
	std::vector<cv::Point3f> edge3;
	for (int y = -gridSize * gridNum; y <= gridSize * gridNum; y += gridSize) {
		edge3.push_back(cv::Point3f(
			static_cast<float>(gridSize * gridNum),
			static_cast<float>(y),
			static_cast<float>(offsetZ)));
	}

	// edge 4
	std::vector<cv::Point3f> edge4;
	for (int x = -gridSize * gridNum; x <= gridSize * gridNum; x += gridSize) {
		edge4.push_back(cv::Point3f(
			static_cast<float>(x),
			static_cast<float>(-gridSize * gridNum),
			static_cast<float>(offsetZ)));
	}

	gridEdges.push_back(edge1);
	gridEdges.push_back(edge2);
	gridEdges.push_back(edge3);
	gridEdges.push_back(edge4);
}

void Renderer::drawGrid() {
	glLineWidth(1.0f);
	glPushMatrix();
	//glTranslatef(0, 4, 0);
	glBegin(GL_LINES);

	int gSize = gridNum * 2 + 1;
	for (int g = 0; g < gSize; g++) {
		// y lines
		glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
		glVertex3f(gridEdges[0][g].x, gridEdges[0][g].y, gridEdges[0][g].z);
		glVertex3f(gridEdges[2][g].x, gridEdges[2][g].y, gridEdges[2][g].z);
		// x lines
		glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
		glVertex3f(gridEdges[1][g].x, gridEdges[1][g].y, gridEdges[1][g].z);
		glVertex3f(gridEdges[3][g].x, gridEdges[3][g].y, gridEdges[3][g].z);
	}

	glEnd();
	glPopMatrix();
}

void Renderer::drawVolume(std::shared_ptr<VoxelGrid> vr) {
	glLineWidth(1.0f);
	glPushMatrix();
	glBegin(GL_LINES);

	// bottom
	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[0].x, vr->volumeCorners[0].y, vr->volumeCorners[0].z);
	glVertex3f(vr->volumeCorners[1].x, vr->volumeCorners[1].y, vr->volumeCorners[1].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[1].x, vr->volumeCorners[1].y, vr->volumeCorners[1].z);
	glVertex3f(vr->volumeCorners[2].x, vr->volumeCorners[2].y, vr->volumeCorners[2].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[2].x, vr->volumeCorners[2].y, vr->volumeCorners[2].z);
	glVertex3f(vr->volumeCorners[3].x, vr->volumeCorners[3].y, vr->volumeCorners[3].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[3].x, vr->volumeCorners[3].y, vr->volumeCorners[3].z);
	glVertex3f(vr->volumeCorners[0].x, vr->volumeCorners[0].y, vr->volumeCorners[0].z);

	// top
	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[4].x, vr->volumeCorners[4].y, vr->volumeCorners[4].z);
	glVertex3f(vr->volumeCorners[5].x, vr->volumeCorners[5].y, vr->volumeCorners[5].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[5].x, vr->volumeCorners[5].y, vr->volumeCorners[5].z);
	glVertex3f(vr->volumeCorners[6].x, vr->volumeCorners[6].y, vr->volumeCorners[6].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[6].x, vr->volumeCorners[6].y, vr->volumeCorners[6].z);
	glVertex3f(vr->volumeCorners[7].x, vr->volumeCorners[7].y, vr->volumeCorners[7].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[7].x, vr->volumeCorners[7].y, vr->volumeCorners[7].z);
	glVertex3f(vr->volumeCorners[4].x, vr->volumeCorners[4].y, vr->volumeCorners[4].z);

	// connection
	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[0].x, vr->volumeCorners[0].y, vr->volumeCorners[0].z);
	glVertex3f(vr->volumeCorners[4].x, vr->volumeCorners[4].y, vr->volumeCorners[4].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[1].x, vr->volumeCorners[1].y, vr->volumeCorners[1].z);
	glVertex3f(vr->volumeCorners[5].x, vr->volumeCorners[5].y, vr->volumeCorners[5].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[2].x, vr->volumeCorners[2].y, vr->volumeCorners[2].z);
	glVertex3f(vr->volumeCorners[6].x, vr->volumeCorners[6].y, vr->volumeCorners[6].z);

	glColor4f(0.9f, 0.9f, 0.9f, 0.5f);
	glVertex3f(vr->volumeCorners[3].x, vr->volumeCorners[3].y, vr->volumeCorners[3].z);
	glVertex3f(vr->volumeCorners[7].x, vr->volumeCorners[7].y, vr->volumeCorners[7].z);

	glEnd();
	glPopMatrix();
}

void Renderer::drawVoxels(std::shared_ptr<VoxelGrid> vr) {
	glPushMatrix();

	// apply default translation
	glTranslatef(0, 0, 0);
	glPointSize(2.0f);
	glBegin(GL_POINTS);

	int vSize = int(vr->visibleVoxels.size());

	for (int v = 0; v < vSize; v++) {
		glColor4f(
			vr->visibleVoxels[v].r / 256, 
			vr->visibleVoxels[v].g / 256, 
			vr->visibleVoxels[v].b / 256, 
			1.0f
		);
		glVertex3f(
			static_cast<float>(vr->visibleVoxels[v].x),
			static_cast<float>(vr->visibleVoxels[v].y),
			static_cast<float>(vr->visibleVoxels[v].z)
		);
	}

	glEnd();
	glPopMatrix();
}

void Renderer::drawCamCoord() {
	glLineWidth(1.0f);
	glPushMatrix();
	glBegin(GL_LINES);

	for (int i = 0; i < numViews; i++) {
		// 0 - 1
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(camCorners[i][0].x, camCorners[i][0].y, camCorners[i][0].z);
		glVertex3f(camCorners[i][1].x, camCorners[i][1].y, camCorners[i][1].z);

		// 0 - 2
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(camCorners[i][0].x, camCorners[i][0].y, camCorners[i][0].z);
		glVertex3f(camCorners[i][2].x, camCorners[i][2].y, camCorners[i][2].z);

		// 0 - 3
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(camCorners[i][0].x, camCorners[i][0].y, camCorners[i][0].z);
		glVertex3f(camCorners[i][3].x, camCorners[i][3].y, camCorners[i][3].z);

		// 0 - 4
		glColor4f(0.8f, 0.8f, 0.8f, 0.5f);
		glVertex3f(camCorners[i][0].x, camCorners[i][0].y, camCorners[i][0].z);
		glVertex3f(camCorners[i][4].x, camCorners[i][4].y, camCorners[i][4].z);

		// 1 - 2
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(camCorners[i][1].x, camCorners[i][1].y, camCorners[i][1].z);
		glVertex3f(camCorners[i][2].x, camCorners[i][2].y, camCorners[i][2].z);

		// 2 - 3
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(camCorners[i][2].x, camCorners[i][2].y, camCorners[i][2].z);
		glVertex3f(camCorners[i][3].x, camCorners[i][3].y, camCorners[i][3].z);

		// 3 - 4
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(camCorners[i][3].x, camCorners[i][3].y, camCorners[i][3].z);
		glVertex3f(camCorners[i][4].x, camCorners[i][4].y, camCorners[i][4].z);

		// 4 - 1
		glColor4f(0.5f, 0.5f, 0.5f, 0.5f);
		glVertex3f(camCorners[i][4].x, camCorners[i][4].y, camCorners[i][4].z);
		glVertex3f(camCorners[i][1].x, camCorners[i][1].y, camCorners[i][1].z);
	}

	glEnd();
	glPopMatrix();
}

void Renderer::drawOrigin() {
	glLineWidth(1.5f);
	glPushMatrix();
	glBegin(GL_LINES);

	// draw x-axis
	glColor4f(0.0f, 0.0f, 1.0f, 0.5f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(500.0f, 0.0f, 0.0f);

	// draw y-axis
	glColor4f(0.0f, 1.0f, 0.0f, 0.5f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 500.0f, 0.0f);

	// draw z-axis
	glColor4f(1.0f, 0.0f, 0.0f, 0.5f);
	glVertex3f(0.0f, 0.0f, 0.0f);
	glVertex3f(0.0f, 0.0f, 500.0f);

	glEnd();
	glPopMatrix();
}

void Renderer::Render(std::shared_ptr<VoxelGrid> vr) {
	glEnable(GL_DEPTH_TEST);

	// Here's our rendering. Clears the screen to black, clear the color and
	// depth buffers, and reset our modelview matrix.
	glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
	glLoadIdentity();

	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity(); // reset modelview matrix

	if (!CameraView) {
		gluLookAt(
			eyeX, eyeY, eyeZ,
			 0.0,  0.0,  0.0,
		    -1.0,  0.0,  0.0
		);
	}
	else {
		gluLookAt(
			eyeX, eyeY, eyeZ,
			 0.0,  0.0,  0.0,
			 0.0,  0.0,  1.0
		);
	}

	// rotate view
	glRotatef(ViewAngle, 0.0f, 0.0f, 1.0f);
	glPopMatrix();

	if (ShowCamera) {
		drawCamCoord();
	}
	if (ShowFloor) {
		drawGrid();
	}
	if (ShowVolume) {
		drawVolume(vr);
	}

	drawVoxels(vr);

	if (ShowOrigin) {
		drawOrigin();
	}
}

void Renderer::Flush() {
	glFlush();
	glutSwapBuffers();
}

void Renderer::Resize(int width, int height) {
	// Reset the viewport to new dimensions
	glViewport(0, 0, width, height);

	// Set current Matrix to projection
	glMatrixMode(GL_PROJECTION);
	glLoadIdentity(); // reset projection matrix

	// Calculate aspect ratio of the window.
	gluPerspective(54.0, static_cast<double>(width) / static_cast<double>(height), 1.0, 20000.0);

	// Model View Matrix
	glMatrixMode(GL_MODELVIEW);
	glLoadIdentity();
	gluLookAt(eyeX, eyeY, eyeZ, 
			   0.0,  0.0,  0.0,
			   0.0,  1.0,  0.0);
}

void Renderer::MoveScene(int x, int y) {
	float r = sqrt(eyeX * eyeX + eyeY * eyeY + eyeZ * eyeZ);
	float phi = atan2(float(eyeY), float(eyeX)); //acos(viewPoint.x/r);
	float theta = acos(eyeZ / r);

	float mult = (r > 1000) ? 2.f : 1.f;

	//float radiansX = -10 * float(x) / r; // atan2(float(y), float(x));
	//float radiansY = -10 * float(y) / r;

	phi   -= float(x) / (100.f * mult);
	theta -= float(y) / (100.f * mult);

	eyeX = r * cos(phi) * sin(theta);
	eyeY = r * sin(phi) * sin(theta);
	eyeZ = r * cos(theta);
}

void Renderer::Zoom(int zDelta) {
	float n = std::sqrtf(eyeX*eyeX + eyeY*eyeY + eyeZ*eyeZ);

	if (zDelta > 0) {
		eyeX += (eyeX / n) * 500;
		eyeY += (eyeY / n) * 500;
		eyeZ += (eyeZ / n) * 500;
	}
	else if (zDelta < 0) {
		eyeX -= (eyeX / n) * 500;
		eyeY -= (eyeY / n) * 500;
		eyeZ -= (eyeZ / n) * 500;
	}
}
