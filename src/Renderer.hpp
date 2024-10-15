#pragma once

#include <string>
#include <vector>

#include <GL/freeglut.h>
#include <opencv2/opencv.hpp>

#include "VoxelGrid.hpp"

class Renderer {
public:
	Renderer(int);
	~Renderer() = default;

	// function to get the N camera coordinates
	void CamCoord(std::vector<cv::Point3f>);
	void ViewIndex(int);
	void ResetTopView();

	void Render(std::shared_ptr<VoxelGrid>);
	void Flush();

    void MoveScene(int x, int y);
    void Zoom(int amount);

	// functions for demo
	void RotateView();
    void Resize(int width, int height);

private:
	// function to calculate and display the ground floor grid and volume
	void initGrid();

	void drawGrid();
	void drawVolume(std::shared_ptr<VoxelGrid>);
	void drawOrigin();

	// function to draw voxels
	void drawVoxels(std::shared_ptr<VoxelGrid>);
	void drawCamCoord();

public:
	int NumFrames;
	float ViewAngle;

	bool CameraView;
	bool ShowVolume;
	bool ShowFloor;
	bool ShowCamera;
	bool ShowOrigin;

private:
	static const int gridNum;
	static const int gridSize;
	static const int offsetZ;

	// camera view point
	static float eyeX;
	static float eyeY;
	static float eyeZ;

	// view dimensions
	static unsigned int width;
	static unsigned int height;

private:
	int numViews;
	int currentView;

	// camera and corner coordinates 
	std::vector<std::vector<cv::Point3f>> camCorners;

	// edge points of the virtual ground floor grid
	std::vector<std::vector<cv::Point3f>> gridEdges;
};
