#pragma once

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include "Histogram.hpp"

class Camera;
class VoxelGrid;

class Tracker {
private:
	// color histograms for both persons
	Histogram colorHistA;
	Histogram colorHistB;
	
	// image histograms for both persons, for each view
	Histogram imageHistA[4];
	Histogram imageHistB[4];

	// back-projected positions of person's pixel position
	cv::Point3f linePosWorldA[4];
	cv::Point3f linePosWorldB[4];

	// local storage of the cameras
	std::vector<std::shared_ptr<Camera>> const cameras;

public: // variables
	// final location of person (at intersection of lines)
	cv::Point3f PersonPosA;
	cv::Point3f PersonPosB;
	
public: // constructor
	Tracker(cv::Mat const, cv::Mat const, std::vector<std::shared_ptr<Camera>> const);
	
public: // functions
	cv::Mat ExtractForeground(cv::Mat frame, cv::Mat mask) const;

	void TrackPersons(std::vector<cv::Mat> foregrounds, int views);
	void LabelForeground(cv::Point posA, cv::Point posB, cv::Mat foreground) const;
	void LabelVoxels(std::shared_ptr<VoxelGrid> vr, cv::Point3f center, float sizeX, float sizeY, cv::Scalar color) const;
	
	void LabelLines();
	void LabelGrids();
	
	void DrawLabelLines(cv::Point3f* positions, cv::Scalar color, float length) const;
	void DrawLabelGrids(cv::Point3f personLocation, float sizeX, float sizeY, float height, cv::Scalar color) const;

	inline Histogram GetImageHistogramA(int view) {
		return imageHistA[view];
	}

	inline Histogram GetImageHistogramB(int view) {
		return imageHistB[view];
	}
};
