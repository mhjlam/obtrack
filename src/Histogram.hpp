#pragma once

#include <opencv2/opencv.hpp>

enum Channel { 
	CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE
};

struct Hist {
	std::vector<cv::Mat> m;
	cv::Mat r() { return m[0]; }
	cv::Mat g() { return m[1]; }
	cv::Mat b() { return m[2]; }
	cv::Mat operator()(int ch) { return m[ch]; }
};

class Histogram {
private:
	Hist hist;
	mutable cv::Mat render;

public: // constructor
	Histogram() = default;

public: // functions
	void CreateColorHistogram(cv::Mat image, cv::Mat mask = cv::Mat());
	void Normalize(double factor = 1.0);
	void Reset();
	
	cv::Mat GetRenderedImage();
	cv::Point GetMeanPeakPosition();
	
	inline float R(int bin) { return hist.r().at<float>(bin); }
	inline float G(int bin) { return hist.g().at<float>(bin); }
	inline float B(int bin) { return hist.b().at<float>(bin);}

	void AddValues(int bin, float valR, float valG, float valB);
};
