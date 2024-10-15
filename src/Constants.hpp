#pragma once

/**
 * 		Original code written by Xinghan Luo. Modifications by Maurits Lam and Marco van Laar.
 * 
 * 		The additional required input images for this code are "init-person1.jpg" and 
 * 		"init-person2.jpg", and should be placed with the rest of the input files 
 * 		in the ROOT/data/ directory, where ROOT contains the executable file.
 * 		
 * 		Additional keyboard controls:
 * 			L		show/hide tracking lines in the scene
 * 			B		show/hide tracking boxes in the scene
 *
 * 		Mouse controls:
 * 			LMB		hold and drag to rotate around origin (when not in topview)
 * 			RMB		hold and drag forwards/downwards to zoom in/out
 * 	
 * 	MODIFICATIONS TO ORIGINAL CODE
 * 		Main:		Tweaks to the background subtraction thresholds for less noise.
 * 					Added line and box calls in display function.
 * 					Modifications and additions to mouse and keyboard controls.
 * 		VoxelGrid:			Changes to the size of the voxel grid (faster startup) and voxel color reset.
 * 		
 * 		Other changes to the original code include name changes, fixes, and memory management.
 * 	
 */

// constexpr uint HIST_SCALE = 4;	    // scale of the histogram
// constexpr uint NUM_BINS = 64;	    // number of bins of a histogram (should be a power of two)
// constexpr uint COLOR_DEPTH = 256;	// number of bits of a color channel
// constexpr uint SCREEN_WIDTH = 644;	// screen width in pixels

// enum Channel { 
// 	CHANNEL_RED, CHANNEL_GREEN, CHANNEL_BLUE
// };
