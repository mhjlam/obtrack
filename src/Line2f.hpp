#pragma once

#include <vector>

#include <opencv2/opencv.hpp>

class Line2f {
private:
	cv::Point2f p0;
	cv::Point2f p1;

public: // constructor
	Line2f();

public: // functions
	static cv::Point2f FindIntersection(Line2f& l1, Line2f& l2);
	static std::vector<cv::Point2f> FindIntersections(Line2f* lines);
	static cv::Point2f FindMeanIntersection(Line2f* lines);
	
	static Line2f Line2DFrom3D(cv::Point3f p0, cv::Point3f p1);
};
