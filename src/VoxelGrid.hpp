#pragma once

#include <memory>
#include <vector>

#include <opencv2/opencv.hpp>

#include "Voxel.hpp"

class Camera;
class LookupTable;

class VoxelGrid {
private:
	static const int GridNum;
	static const int GridSize;
	static const int VoxelStep;

public:
	VoxelGrid(int, int, std::vector<std::shared_ptr<Camera>>);

	void UpdateVoxels(std::vector<std::shared_ptr<Camera>>);

	int numViews;
	int numVoxels;
	int viewWidth;
	int viewHeight;

	std::vector<Voxel> voxels;
	std::vector<Voxel> visibleVoxels;
	std::vector<cv::Point3f> volumeCorners; // 8 corners of the acquisition space
	std::vector<std::shared_ptr<LookupTable>> LUT;
};
