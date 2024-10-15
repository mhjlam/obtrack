#include "Histogram.hpp"

#include "Main.hpp"

// Generates RGB histograms based on an image.
void Histogram::CreateColorHistogram(cv::Mat image, cv::Mat mask) {
	// split image into RGB channels
	std::vector<cv::Mat> channels;
	cv::split(image, channels);

	// clear previous values
	Reset();

	// calculate new values
	int histSize = NUM_BINS;
	float range[] = { 5, 250 }; // filter realy dark and really bright values
	const float* histRange = { range };

	cv::calcHist(&channels[0], 1, 0, cv::Mat(), hist(0), 1, &histSize, &histRange);
	cv::calcHist(&channels[1], 1, 0, cv::Mat(), hist(1), 1, &histSize, &histRange);
	cv::calcHist(&channels[2], 1, 0, cv::Mat(), hist(2), 1, &histSize, &histRange);

	// normalize histogram
	Normalize();
}

// Creates an image representation of a histogram.
cv::Mat Histogram::GetRenderedImage() {
	cv::Size renderSize = cv::Size(
		NUM_BINS * HIST_SCALE, 
		NUM_BINS * HIST_SCALE
	);

	// initialize render image
	render = (render.empty()) ? cv::Mat(renderSize, CV_8U) : cv::Mat::zeros(renderSize, CV_8U);

	// process each channel individually
	for (int ch = 0; ch < 3; ++ch) {
		// determine max
		double max;
		cv::minMaxLoc(hist(ch), nullptr, &max, nullptr, nullptr);

		// draw lines between bin values
		for (int i = 0; i < NUM_BINS - 1; ++i) {
			// compute line between two values
			float currentValue = hist(ch).at<float>(i);
			float nextValue = hist(ch).at<float>(i+1);

			// compute line locations in the image (requires some scaling)
			double y = (NUM_BINS - ((currentValue * NUM_BINS) / max)) * HIST_SCALE;
			cv::Point pt1 = cv::Point(i * HIST_SCALE, static_cast<int>(y));
			cv::Point pt2 = cv::Point((i+1) * HIST_SCALE, static_cast<int>(y));

			// color to draw with is based on the current channel
			cv::Scalar color = CV_RGB(0, 0, 0);
			color[ch] = 255;

			// draw the line
			cv::line(render, pt1, pt2, color, 1);
		}
	}

	return render;
}

// Normalizes the histograms for all three channels.
void Histogram::Normalize(double factor) {
	cv::normalize(hist(0), hist(0), factor, cv::NORM_L1);
	cv::normalize(hist(1), hist(1), factor, cv::NORM_L1);
	cv::normalize(hist(2), hist(2), factor, cv::NORM_L1);
}

// Resets all the histograms for all three channels
void Histogram::Reset() {
	hist(0).zeros(hist(0).size(), CV_8U);
	hist(1).zeros(hist(1).size(), CV_8U);
	hist(2).zeros(hist(2).size(), CV_8U);
}

// Returns the mean of the channel histogram peaks.
cv::Point Histogram::GetMeanPeakPosition() {
	cv::Point maxR, maxG, maxB;
	
	// retrieve individual channel peaks
	cv::minMaxLoc(hist.r(), nullptr, nullptr, nullptr, &maxR);
	cv::minMaxLoc(hist.g(), nullptr, nullptr, nullptr, &maxG);
	cv::minMaxLoc(hist.b(), nullptr, nullptr, nullptr, &maxB);
	
	// calculate mean of x
	return cv::Point(static_cast<int>(maxR.x + maxG.x + maxB.x / 3.0), 0);
}

// Adds values to the selected bin for each channel.
void Histogram::AddValues(int bin, float valR, float valG, float valB) {
	hist.r().at<float>(bin) = hist.r().at<float>(bin) + valR;
	hist.g().at<float>(bin) = hist.g().at<float>(bin) + valG;
	hist.b().at<float>(bin) = hist.b().at<float>(bin) + valB;
}
