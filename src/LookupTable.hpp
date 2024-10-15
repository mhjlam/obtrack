#pragma once

#include <vector>

class LookupTable {
public:
	LookupTable(int w, int h) : width(w), height(h) {
		for (int x = 0; x < width; x ++) {
			std::vector<std::vector<int>> ltx;
			lut.push_back(ltx);

			for (int y = 0; y < height; y ++) {
				std::vector<int> lty;
				lut[x].push_back(lty);
			}
		}
	}

	int width;
	int height;

	// Look Up Table for each pixel
	std::vector<std::vector<std::vector<int>>> lut;
};
