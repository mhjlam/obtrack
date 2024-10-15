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
 * 		VoxelGrid:	Changes to the size of the voxel grid (faster startup) and voxel color reset.
 * 		
 * 		Other changes to the original code include name changes, fixes, and memory management.
 * 	
 */

#include <memory>
#include <vector>
#include <cstdlib>
#include <fstream>
#include <iostream>

#define casti static_cast<int>
#define castf static_cast<float>
#define castd static_cast<double>

constexpr int NumViews = 4;
constexpr int ViewWidth = 644;
constexpr int ViewHeight = 484;

class Camera;
class Tracker;
class Renderer;
class Histogram;
class VoxelGrid;


constexpr int HIST_SCALE    = 4;	// scale of the histogram
constexpr int NUM_BINS      = 64;	// number of bins of a histogram (should be a power of two)
constexpr int COLOR_DEPTH   = 256;	// number of bits of a color channel
constexpr int SCREEN_WIDTH  = 644;	// screen width in pixels
