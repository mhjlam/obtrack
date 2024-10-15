#include "Tracker.hpp"

#include <GL/freeglut.h>

#include "Main.hpp"
#include "Voxel.hpp"
#include "Camera.hpp"
#include "Line2f.hpp"
#include "Constants.hpp"
#include "VoxelGrid.hpp"

Tracker::Tracker(cv::Mat const fgA, cv::Mat const fgB, std::vector<std::shared_ptr<Camera>> const cams) : cameras(cams) {
	colorHistA.CreateColorHistogram(fgA);
	colorHistB.CreateColorHistogram(fgB);
}

// Retrieves the foreground image based on a frame image and a mask.
cv::Mat Tracker::ExtractForeground(cv::Mat frame, cv::Mat mask) const {
	cv::Mat fg = cv::Mat(frame.size(), frame.depth(), frame.channels());
	frame.copyTo(fg, mask);
	return fg;
}

void Tracker::TrackPersons(std::vector<cv::Mat> foregrounds, int views) {
	// back-projected lines
	std::vector<Line2f> linesA;
	std::vector<Line2f> linesB;
	
	// iterate over views
	for (int v = 0; v < views; ++v) {
		// reset histograms
		imageHistA[v].Reset();
		imageHistB[v].Reset();

		// acquire foreground of the current view
		cv::Mat foreground = foregrounds[v];

		int imgwidth = foreground.size().width;
		int imgheight = foreground.size().height;

		// Iterate over foreground pixels with respect to the x-axis of the image (columns, then rows).
		for (int x = 0; x < imgwidth; ++x) {
			for (int y = 0; y < imgheight; ++y) {
				// retrieve color of this pixel
				//cv::Scalar pixelColor = cvGet2D(foreground, y, x);
				cv::Scalar pixelColor = foreground.at<cv::Scalar>(y, x);

				// ignore very dark pixels
				if (pixelColor.val[0] < 5 && pixelColor.val[1] < 5 && pixelColor.val[2] < 5) {
					continue;
				}
				
				// ignore very bright pixels
				if (pixelColor.val[0] >= 250 && pixelColor.val[1] >= 250 && pixelColor.val[2] >= 250) {
					continue;
				}

				// determine the color and image bins
				int colorBin = static_cast<int>(COLOR_DEPTH / NUM_BINS);
				int imageBin = static_cast<int>(floorf((static_cast<float>(NUM_BINS) / imgwidth) * x));
				
				// find the appropriate bins for this pixel
				uint8_t indexR = static_cast<uint8_t>(std::floor(pixelColor.val[0] / colorBin));
				uint8_t indexG = static_cast<uint8_t>(std::floor(pixelColor.val[1] / colorBin));
				uint8_t indexB = static_cast<uint8_t>(std::floor(pixelColor.val[2] / colorBin));

				// find occurrences of the color values in both color histograms
				float valueR[2] = { colorHistA.R(indexR), colorHistB.R(indexR) };
				float valueG[2] = { colorHistA.G(indexG), colorHistB.G(indexG) };
				float valueB[2] = { colorHistA.B(indexB), colorHistB.B(indexB) };
				
				// fill image histograms based on the occurrences of pixel color values
				imageHistA[v].AddValues(imageBin, valueR[0], valueG[0], valueB[0]);
				imageHistB[v].AddValues(imageBin, valueR[1], valueG[1], valueB[1]);
			}
		}
		
		// normalize image histograms
		imageHistA[v].Normalize();
		imageHistB[v].Normalize();
		
		// retrieve the (mean) peak of the histogram to find the 2D person position
		cv::Point linePosHistA = imageHistA[v].GetMeanPeakPosition();
		cv::Point linePosHistB = imageHistB[v].GetMeanPeakPosition();
		
		// label the persons in the foreground
		LabelForeground(linePosHistA, linePosHistB, foreground);

		// scale the x-coordinate by the ratio of the scene screen size to the size of the histogram
		linePosHistA.x *= SCREEN_WIDTH / NUM_BINS;
		linePosHistB.x *= SCREEN_WIDTH / NUM_BINS;
		
		// back-project these 2D positions into 3D space
		linePosWorldA[v] = cameras[v]->Point2DtoWorld3D(linePosHistA);
		linePosWorldB[v] = cameras[v]->Point2DtoWorld3D(linePosHistB);

		// construct line equation from the camera location to the person's pixel position in 3D
		linesA[v] = Line2f::Line2DFrom3D(cameras[v]->PosWorld, linePosWorldA[v]);
		linesB[v] = Line2f::Line2DFrom3D(cameras[v]->PosWorld, linePosWorldB[v]);
	}
	
	// find mean intersection of the lines to retrieve the persons' positions on the plane
	cv::Point2f meanPlanePosA = Line2f::FindMeanIntersection(&linesA[0]);
	cv::Point2f meanPlanePosB = Line2f::FindMeanIntersection(&linesB[0]);

	// define the final person locations in 3D space
	PersonPosA = cv::Point3f(meanPlanePosA.x, meanPlanePosA.y, 0.0);
	PersonPosB = cv::Point3f(meanPlanePosB.x, meanPlanePosB.y, 0.0);
}

// Indicates both persons with a colored line in the foreground image.
void Tracker::LabelForeground(cv::Point posA, cv::Point posB, cv::Mat fgImage) const {
	// must scale bin position back to image coordinate
	int scaler = fgImage.size().width / NUM_BINS;
	posA.x *= scaler;
	posB.x *= scaler;

	// draw vertical line
	cv::line(fgImage, posA, cv::Point(posA.x, fgImage.size().height), CV_RGB(0, 255, 0));
	cv::line(fgImage, posB, cv::Point(posB.x, fgImage.size().height), CV_RGB(0, 0, 255));
}

// Colors the voxels in a box of the given dimensions around the given center, with the given color.
void Tracker::LabelVoxels(std::shared_ptr<VoxelGrid> vr, cv::Point3f center, float sizeX, float sizeY, cv::Scalar color) const {
	// bounding box parameters
	float xstart = center.x - sizeX;
	float xend = center.x + sizeX;
	float ystart = center.y - sizeY;
	float yend = center.y + sizeY;

	// only label visible voxels
	for (unsigned i = 0; i < vr->visibleVoxels.size(); ++i) {
		Voxel voxel = vr->visibleVoxels[i];

		// check if this voxel is within the bounding box
		if (voxel.x > xstart && voxel.x < xend) {
			if (voxel.y > ystart && voxel.y < yend) {
				// color conflicting voxels red
				if ((voxel.g == 200 && color.val[1] != 200) || (voxel.b == 200 && color.val[0] != 200)) {
					voxel.r = 255;
					voxel.g = 0;
					voxel.b = 0;
				}
				else {
					voxel.r = static_cast<float>(color.val[2]);
					voxel.g = static_cast<float>(color.val[1]);
					voxel.b = static_cast<float>(color.val[0]);
				}
			}
		}
	}
}

// Draws lines for both persons
void Tracker::LabelLines() {
	DrawLabelLines(linePosWorldA, CV_RGB(0, 200, 0), 8000.f);
	DrawLabelLines(linePosWorldB, CV_RGB(0, 0, 200), 8000.f);
}

// Draws grids for both persons
void Tracker::LabelGrids() {
	DrawLabelGrids(PersonPosA, 350.f, 750.f, 2000.0f, CV_RGB(0, 200, 0));
	DrawLabelGrids(PersonPosB, 350.f, 750.f, 2000.0f, CV_RGB(0, 0, 200));
}

// Draws lines from the origin of each camera towards the person in the scene with the given color and length
void Tracker::DrawLabelLines(cv::Point3f* positions, cv::Scalar color, float length) const {
	glPushMatrix();
	
	glColor4d(color.val[2] / 256.0, color.val[1] / 256.0, color.val[0] / 256.0, 1.0);
	glLineWidth(0.5f);
	
	glBegin(GL_LINES);

	for (int v = 0; v < 4; ++v) {
		// need camera and pixel positions on the plane
		cv::Point3f cameraOrigin = cv::Point3f(cameras[v]->PosWorld.x, cameras[v]->PosWorld.y, 0);
		cv::Point3f linePosWorld = positions[v];

		// construct vector between these points
		cv::Point3f vector = cv::Point3f(linePosWorld.x - cameraOrigin.x, linePosWorld.y - cameraOrigin.y, 0);

		// calculate length for normalization
		float vectorLength = sqrt(vector.x*vector.x + vector.y*vector.y);

		// normalize std::vector
		cv::Point3f vectorNorm = cv::Point3f(vector.x/vectorLength, vector.y/vectorLength, 0);

		// multiply std::vector with arbitrary scalar value to retrieve the end point and add to cam location
		cv::Point3f endPosCam = cv::Point3f(cameraOrigin.x + (vectorNorm.x*length), cameraOrigin.y + (vectorNorm.y*length), 0);

		// draw line
		glVertex3f(cameraOrigin.x, cameraOrigin.y, 0);
		glVertex3f(endPosCam.x, endPosCam.y, 0);
	}
	glEnd();
	
	glPopMatrix();
}

// Draws boxes around a given location, with the given size and color.
void Tracker::DrawLabelGrids(cv::Point3f personLocation, float sizeX, float sizeY, float height, cv::Scalar color) const {
	glPushMatrix();

	glColor4d(color.val[2] / 256.0, color.val[1] / 256.0, color.val[0] / 256.0, 1.0);
	glLineWidth(2.f);

	// top and bottom
	for (float h = 0.0f; h <= height; h += height) {
		glBegin(GL_LINE_STRIP);
			glVertex3f(personLocation.x - sizeX, personLocation.y - sizeY, h);
			glVertex3f(personLocation.x - sizeX, personLocation.y + sizeY, h);
			glVertex3f(personLocation.x + sizeX, personLocation.y + sizeY, h);
			glVertex3f(personLocation.x + sizeX, personLocation.y - sizeY, h);
			glVertex3f(personLocation.x - sizeX, personLocation.y - sizeY, h);
		glEnd();
	}

	// vertical
	glBegin(GL_LINES);
		glVertex3f(personLocation.x - sizeX, personLocation.y - sizeY, 0.0f);
		glVertex3f(personLocation.x - sizeX, personLocation.y - sizeY, height);

		glVertex3f(personLocation.x - sizeX, personLocation.y + sizeY, 0.0f);
		glVertex3f(personLocation.x - sizeX, personLocation.y + sizeY, height);

		glVertex3f(personLocation.x + sizeX, personLocation.y - sizeY, 0.0f);
		glVertex3f(personLocation.x + sizeX, personLocation.y - sizeY, height);

		glVertex3f(personLocation.x + sizeX, personLocation.y + sizeY, 0.0f);
		glVertex3f(personLocation.x + sizeX, personLocation.y + sizeY, height);
	glEnd();

	glPopMatrix();
}
