#include "Line2f.hpp"

Line2f::Line2f() {
	p0 = cv::Point2f(0, 0);
	p1 = cv::Point2f(0, 0);
}

// Finds intersection between two lines.
cv::Point2f Line2f::FindIntersection(Line2f& l1, Line2f& l2) {
	// extract points
	float x1 = l1.p0.x;
	float y1 = l1.p0.y;
	
	float x2 = l1.p1.x;
	float y2 = l1.p1.y;
	
	float x3 = l2.p0.x;
	float y3 = l2.p0.y;
	
	float x4 = l2.p1.x;
	float y4 = l2.p1.y;
	
	// calculation of intersection using determinants (http://en.wikipedia.org/wiki/Line-line_intersection)
	float numerX = ((x1*y2 - y1*x2) * (x3-x4)) - ((x1 - x2) * (x3*y4 - y3*x4));
	float numerY = ((x1*y2 - y1*x2) * (y3-y4)) - ((y1 - y2) * (x3*y4 - y3*x4));
	float denom  = ((x1 - x2) * (y3 - y4)) - ((y1 - y2) * (x3 - x4));
	
	return cv::Point2f(numerX/denom, numerY/denom);
}

// Finds all intersections between non-repeating combinations of the given array of lines.
std::vector<cv::Point2f> Line2f::FindIntersections(Line2f* lines) {
	std::vector<cv::Point2f> intersections;
	
	// find all intersection points (no repetition), we explicitly leave out the 4th view because it is so noisy.
	for (int i = 1; i < 4; ++i) {
		for (int j = i; j < 3; ++j) {
			intersections.push_back(FindIntersection(lines[i-1], lines[j]));
		}
	}
	
	return intersections;
}

// Finds the mean of intersections between several lines.
cv::Point2f Line2f::FindMeanIntersection(Line2f* lines) {
	// retrieve non-repeating combinations of intersections between the lines
	std::vector<cv::Point2f> intersections = FindIntersections(lines);
	
	if (intersections.empty()) {
		return cv::Point2f(0, 0);
	}
	
	float meanPosX = 0;
	float meanPosY = 0;
	
	// iterate over intersections
	for (auto it = intersections.begin(); it != intersections.end(); ++it) {
		meanPosX += it->x;
		meanPosY += it->y;
	}
	
	// calculate mean intersection position
	meanPosX /= intersections.size();
	meanPosY /= intersections.size();
	
	return cv::Point2f(meanPosX, meanPosY);
}

// Constructs a 2D line from two positions (camera and pixel coordinates).
Line2f Line2f::Line2DFrom3D(cv::Point3f p0, cv::Point3f p1) {
	Line2f line;
	line.p0 = cv::Point2f(p0.x, p0.y);
	line.p1 = cv::Point2f(p1.x, p1.y);
	return line;
}
