#pragma once

#include <string>
#include <vector>

#include <opencv2/opencv.hpp>

class Camera {
public:
	Camera(int, int, int, std::string);

public:
	void LoadParams(std::string);	// calib data by loading intrinsic and extrinsic matrices
	void ComputeLocation();
	void MultMatrix(float rm[4][4], 
					const float m1[4][4], 
					const float m2[4][4]);
	void InvertRt();
	void WorldCoord();				// camera corners in world coordinate
	cv::Point ProjectOnView(cv::Point3f);

	// functions for 2D <=> 3D calculation
	cv::Point3f Point2DtoWorld3D(cv::Point);

private:
	cv::Point3f camPoint3DtoWorld3D(cv::Point3f camPt3D);

public:
	cv::Mat Foreground;
	std::vector<cv::Point3f> Corners;

	// camera location
	cv::Point3f PosWorld; // 3D coordinates in world frame


private:
	cv::Size viewSize; 						// size of the view
	std::vector<cv::Point3f> camOnGrdFlr; 	// three points that are the projection of the camera itself to the ground floor view

	int framesProcessed; //  number of frames been processed

	// colors 	
	std::vector<cv::Scalar> color;

	// calibration matrices
	cv::Mat rotationVector;
	cv::Mat translationVector;
	cv::Mat intrinsicMatrix;
	cv::Mat distortionCoeffs;
	cv::Mat rotationMatrix;

	// Rt matrix and its inverse
	cv::Mat Rt;
	cv::Mat inverseRt;

	// camera focal length and principal points
	float fx;
	float fy;
	float px;
	float py;
};
