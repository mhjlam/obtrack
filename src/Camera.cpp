#include "Camera.hpp"

#include <vector>
#include <fstream>
#include <iostream>
#include <algorithm>

#include <opencv2/opencv.hpp>

Camera::Camera(int index, int viewWidth, int viewHeight, std::string camIniFile) : 
viewSize(viewWidth, viewHeight) {
	LoadParams(camIniFile);

	// colors
	color.push_back(CV_RGB(0,0,255));
	color.push_back(CV_RGB(0,255,0));
	color.push_back(CV_RGB(255,0,0));
	color.push_back(CV_RGB(0,255,255));
	color.push_back(CV_RGB(255,255,0));
	color.push_back(CV_RGB(255,0,255));

	// get the camera parameters and calculate principal matrices and points
	fx = static_cast<float>(intrinsicMatrix.at<float>(0, 0));
	fy = static_cast<float>(intrinsicMatrix.at<float>(1, 1));
	px = static_cast<float>(intrinsicMatrix.at<float>(0, 2));
	py = static_cast<float>(intrinsicMatrix.at<float>(1, 2));

	ComputeLocation();
	InvertRt();
	WorldCoord();

	std::cout << "Camera " << index + 1 << " : coordinates ready" << std::endl;
}

// load calibration files
void Camera::LoadParams(std::string iniFileName) {
	rotationVector = cv::Mat(1, 3, CV_32FC1);
	translationVector = cv::Mat(1, 3, CV_32FC1);
	intrinsicMatrix = cv::Mat(3, 3, CV_32FC1);
	distortionCoeffs = cv::Mat(4, 1, CV_32FC1);
	rotationMatrix = cv::Mat(3, 3, CV_32FC1);

	std::string line;
	std::ifstream file(iniFileName);

	// load settings
	if (file.is_open()) {
		float value;
		for (int y = 0; y < 3; y ++) {
			for (int x = 0; x < 3; x ++) {
				file >> value;
				intrinsicMatrix.at<float>(y, x) = value;
			}
		}

		for (int x = 0; x < 4; x ++) {
			file >> value;
			distortionCoeffs.at<float>(x, 0) = value;
		}
		for (int y = 0; y < 3; y ++) {
			file >> value;
			rotationVector.at<float>(0, y) = value;
		}
		for (int y = 0; y < 3; y ++) {
			file >> value;
			translationVector.at<float>(0, y) = value;
		}
	}
	file.close();
}

void Camera::ComputeLocation() {
	float rMatrix[9];
	float rVector[3];

	rVector[0] = rotationVector.at<float>(0, 0);
	rVector[1] = rotationVector.at<float>(0, 1);
	rVector[2] = rotationVector.at<float>(0, 2);

	cv::Mat rMatrixTemp = cv::Mat(3, 3, CV_32FC1, rMatrix);
	cv::Mat rVectorTemp = cv::Mat(1, 3, CV_32FC1, rVector);

	// compute rotation matrix by Rodrigues transform
	cv::Rodrigues(rVectorTemp, rMatrixTemp);

	// save rotation matrix
	// TODO: OPTIMIZE
	for (int i = 0; i < 3; i ++) {
		for (int j = 0; j < 3; j ++) {
			rotationMatrix.at<float>(i, j) = rMatrixTemp.at<float>(i, j);
		}
	}

	// Combine transformations into one matrix
	// Mind the order; rotations are not commutative
	float tmat[4][4] = { 
		{ 1.f, 0.f, 0.f, 0.f },
		{ 0.f, 1.f, 0.f, 0.f },
		{ 0.f, 0.f, 1.f, 0.f },
		{ 
			translationVector.at<float>(0, 0) * -1.f,
			translationVector.at<float>(0, 1) * -1.f,
			translationVector.at<float>(0, 2) * -1.f,
			1.f 
		}
	};

	float rmat[4][4] = { 
		{ rMatrix[0], rMatrix[1], rMatrix[2], 0.f },
		{ rMatrix[3], rMatrix[4], rMatrix[5], 0.f },
		{ rMatrix[6], rMatrix[7], rMatrix[8], 0.f },
		{ 0.f, 0.f, 0.f, 1.f }
	};
	
	// matrix multiplication
	float rm[4][4];
	for (int i = 0; i <= 3; i++) {
		for (int j = 0; j <= 3; j++) {
			rm[i][j] = 0.0;
			for (int k = 0; k <= 3; k++) {
				rm[i][j] += tmat[i][k] * rmat[k][j];
			}
		}
	}

	PosWorld = cv::Point3f(
		rm[0][0] + rm[3][0],
		rm[1][1] + rm[3][1],
		rm[2][2] + rm[3][2]
	);
}

void Camera::InvertRt() {
	// Xi = K[R/t]Xw 
	// t = -RC
	Rt = cv::Mat(4, 4, CV_64FC1);
	inverseRt = cv::Mat(4, 4, CV_64FC1);

	// [R|t] 
	Rt.at<float>(0, 0) = rotationMatrix.at<float>(0, 0);
	Rt.at<float>(0, 1) = rotationMatrix.at<float>(0, 1);
	Rt.at<float>(0, 2) = rotationMatrix.at<float>(0, 2);
	Rt.at<float>(0, 3) = translationVector.at<float>(0, 0);

	Rt.at<float>(1, 0) = rotationMatrix.at<float>(1, 0);
	Rt.at<float>(1, 1) = rotationMatrix.at<float>(1, 1);
	Rt.at<float>(1, 2) = rotationMatrix.at<float>(1, 2);
	Rt.at<float>(1, 3) = translationVector.at<float>(0, 1);

	Rt.at<float>(2, 0) = rotationMatrix.at<float>(2, 0);
	Rt.at<float>(2, 1) = rotationMatrix.at<float>(2, 1);
	Rt.at<float>(2, 2) = rotationMatrix.at<float>(2, 2);
	Rt.at<float>(2, 3) = translationVector.at<float>(0, 2);

	Rt.at<float>(3, 0) = 0.f;
	Rt.at<float>(3, 1) = 0.f;
	Rt.at<float>(3, 2) = 0.f;
	Rt.at<float>(3, 3) = 1.f;

	cv::invert(Rt, inverseRt);
}

void Camera::MultMatrix(float rm[4][4], const float m1[4][4], const float m2[4][4]) {
	for (int i = 0; i <= 3; i ++) {
		for (int j = 0; j <= 3; j ++) {
			rm[i][j] = 0.0;
			for (int k = 0; k <= 3; k ++){
				rm[i][j] += m1[i][k] * m2[k][j];
			}
		}
	}
}

cv::Point Camera::ProjectOnView(cv::Point3f obP) {
	cv::Point pt;
	cv::Mat objectPoints = cv::Mat(3, 3, CV_32FC1);
	cv::Mat imagePoints = cv::Mat(3, 2, CV_32FC1);

	objectPoints.at<float>(0, 0) = obP.x;
	objectPoints.at<float>(0, 1) = obP.y;
	objectPoints.at<float>(0, 2) = obP.z;

	cv::projectPoints(objectPoints, rotationVector, translationVector, intrinsicMatrix, distortionCoeffs, imagePoints);

	pt.x = static_cast<int>(imagePoints.at<float>(0, 0));
	pt.y = static_cast<int>(imagePoints.at<float>(0, 1));

	return pt;
}

void Camera::WorldCoord() {
	// 0 camera projection center
	Corners.push_back(PosWorld);
	cv::Point3f p1;
	cv::Point3f p2;
	cv::Point3f p3;
	cv::Point3f p4;
	cv::Point3f p5;

	// clockwise four image plane conners
	// 1 image plane's left upper conner
	p1 = camPoint3DtoWorld3D(cv::Point3f(-px, -py, (fx + fy) / 2));
	Corners.push_back(p1);

	// 2 image plane's right upper conner
	p2 = camPoint3DtoWorld3D(cv::Point3f(viewSize.width - px, -py, (fx + fy) / 2));
	Corners.push_back(p2);

	// 3 image plane's right bottom conner
	p3 = camPoint3DtoWorld3D(cv::Point3f(viewSize.width - px, viewSize.height - py, (fx + fy) / 2));
	Corners.push_back(p3);

	// 4 image plane's left bottom conner
	p4 = camPoint3DtoWorld3D(cv::Point3f(-px, viewSize.height - py, (fx + fy) / 2));
	Corners.push_back(p4);

	// principal point on the image plane
	p5 = camPoint3DtoWorld3D(cv::Point3f(px, py, (fx + fy) / 2));
	Corners.push_back(p5);
}

// convert a 2D point on view to 3D world coordinates
cv::Point3f Camera::Point2DtoWorld3D(cv::Point pt) {
	return camPoint3DtoWorld3D(cv::Point3f(float(pt.x - px), float(pt.y - py), (fx + fy) / 2));
}

cv::Point3f Camera::camPoint3DtoWorld3D(cv::Point3f camPt3D) {
	cv::Mat Xc = cv::Mat(4, 1, CV_64FC1);
	Xc.at<float>(0, 0) = camPt3D.x;
	Xc.at<float>(1, 0) = camPt3D.y;
	Xc.at<float>(2, 0) = camPt3D.z;
	Xc.at<float>(3, 0) = 1.f;

	cv::Mat Xw = inverseRt * Xc;
	return cv::Point3f(
		Xw.at<float>(0, 0),
		Xw.at<float>(1, 0),
		Xw.at<float>(2, 0)
	);
}
