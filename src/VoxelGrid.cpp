#include "VoxelGrid.hpp"

#include <vector>
#include <iostream>

#include <opencv2/opencv.hpp>

#include "Main.hpp"
#include "Camera.hpp"
#include "LookupTable.hpp"

const int VoxelGrid::GridNum = 4;
const int VoxelGrid::GridSize = 400;
const int VoxelGrid::VoxelStep = 50;

VoxelGrid::VoxelGrid(int w, int h, std::vector<std::shared_ptr<Camera>> cameras) : 
numViews(casti(cameras.size())), viewWidth(w), viewHeight(h) {
	// Create Look Up Table for each view
	LUT = std::vector<std::shared_ptr<LookupTable>>(numViews, nullptr);
	for (int i = 0; i < numViews; i ++) {
		LUT.push_back(std::make_shared<LookupTable>(viewWidth, viewHeight));
	}

	int halfEdge = GridSize * GridNum;
	int edge = halfEdge * 2;

	int xL = -halfEdge;
	int xR = halfEdge;
	int yL = -halfEdge;
	int yR = halfEdge;
	int zL = 0;
	int zR = edge - (halfEdge / 2);

	// save the volume corners
	volumeCorners.resize(8); // 8 corners of the acquision space

	// bottom
	volumeCorners[0] = cv::Point3f(castf(xL), castf(yL), castf(zL));
	volumeCorners[1] = cv::Point3f(castf(xL), castf(yR), castf(zL));
	volumeCorners[2] = cv::Point3f(castf(xR), castf(yR), castf(zL));
	volumeCorners[3] = cv::Point3f(castf(xR), castf(yL), castf(zL));

	// top
	volumeCorners[4] = cv::Point3f(castf(xL), castf(yL), castf(zR));
	volumeCorners[5] = cv::Point3f(castf(xL), castf(yR), castf(zR));
	volumeCorners[6] = cv::Point3f(castf(xR), castf(yR), castf(zR));
	volumeCorners[7] = cv::Point3f(castf(xR), castf(yL), castf(zR));

	numVoxels = (edge / VoxelStep) * (edge / VoxelStep) * ((edge - (halfEdge / 2)) / VoxelStep);

	// the whole voxel list
	voxels.resize(numVoxels);

	int percentSign = 10;
	std::cout << "Number of voxels " << numVoxels << std::endl;
	std::cout << "Now initializing look-up table ";

	// voxel index
	int v = 0;
	for (int x = xL; x < xR; x += VoxelStep) {
		for (int y = yL; y < yR; y += VoxelStep) {
			for (int z = zL; z < zR; z += VoxelStep) {
				// initialize the voxels
				voxels[v].x = x;
				voxels[v].y = y;
				voxels[v].z = z;
				voxels[v].numVisible = 0;
				voxels[v].view = numViews;

				// project 3D voxel to each 2D view
				// attach to the corresponding pixel's LookupTable
				for (int i = 0; i < numViews; i ++) {
					cv::Point pt = cameras[i]->ProjectOnView(
						cv::Point3f(castf(x), castf(y), castf(z)));
					// if the voxel is visible in current view, save its idex to the LookupTable of the pixel it projects on
					if ((pt.x >= 0) && (pt.x < viewWidth) && (pt.y >= 0) && (pt.y < viewHeight)) {
						//	std::cout << v << " " << pt.x << " " << pt.y << std::endl;
						LUT[i]->lut[pt.x][pt.y].push_back(v);
					}
				}
				v ++;
				if ((numVoxels / v) == percentSign) {
					std::cout << ".";
					percentSign--;
				}
			}
		}
	}

	std::cout << " Done." << std::endl;
}

void VoxelGrid::UpdateVoxels(std::vector<std::shared_ptr<Camera>> cameras) {
	for (int v = 0; v < numVoxels; v ++) {
		if (voxels[v].numVisible > 0) {
			voxels[v].numVisible = 0;
		}
	}

	visibleVoxels.clear();

	// update the voxel list
	for (int i = 0; i < numViews; i ++) {
		for (int x = 0; x < viewWidth; x ++) {
			for (int y = 0; y < viewHeight; y ++) {
				// if it is a foreground pixel
				if (cameras[i]->Foreground.at<uint8_t>(y, x) == 255) {
					// if some voxel is visible at this pixel
					if (int(LUT[i]->lut[x][y].size()) > 0) {
						int size = static_cast<int>(LUT[i]->lut[x][y].size());
						for (int t = 0; t < size; t ++) {
							// visible counter plus one
							int v = LUT[i]->lut[x][y][t];
							if (v < numVoxels) {
								// reset voxel colors to grey (unlabeled)
								voxels[v].r = voxels[v].g = voxels[v].b = 150;
								voxels[v].numVisible++;

								// if the voxel is visible in all views, marked it as visible voxel
								if (voxels[v].numVisible == numViews) {
									visibleVoxels.push_back(voxels[v]);
								}
							}
						}
					}
				}
			}
		}
	}
}
