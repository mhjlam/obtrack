#pragma once

struct Voxel {
	int x;
	int y;
	int z;

	float r;
	float g;
	float b;

	int view;
	int numVisible; // counter in how many views the voxel is visible
};
