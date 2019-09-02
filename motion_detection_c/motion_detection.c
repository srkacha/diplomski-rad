#include "motion_detection.h"
#include <stdlib.h>

unsigned char*** calculateMotionVectorMatrix(int video_w, int video_h, int channels, unsigned char*** currentFrame, unsigned char*** prevFrame){
	unsigned char*** mvm = allocateSpaceForMVM(video_w, video_h);
	int result = 0;

	//if the video frames are grayscale
	if (channels == 1) {
		result = calculateMotionVectorMatrixGrayscale(video_h, video_w, currentFrame, prevFrame, mvm);
	//if the video frames are RGB/HSV/something other
	}
	else if (channels == 3) {
		result = calculateMotionVectorMatrixRGB(video_h, video_w, currentFrame, prevFrame, mvm);
	}

	//checking for errors
	if (result == 0) return 0;

	return mvm;
}



unsigned char*** allocateSpaceForMVM(int video_w, int video_h) {
	//alocating rows
	unsigned char*** mvm = (unsigned char***)calloc((video_h / MACRO_BLOCK_DIM), sizeof(unsigned char**));
	
	//alocating cols
	for (int i = 0; i < video_h; i++) {
		mvm[i] = (unsigned char**)calloc((video_w / MACRO_BLOCK_DIM), sizeof(unsigned char*));
		//alocating channels
		for (int j = 0; j < video_w; j++) {
			mvm[i][j] = (unsigned char*)calloc(2, sizeof(unsigned char));
		}
	}

	return mvm;
}

int calculateMotionVectorMatrixGrayscale(int video_w, int video_h, unsigned char*** currentFrame, unsigned char*** prevFrame, char*** mvm) {
	return 0;
}

void calculateBlockOffset(unsigned char*** currentFrame, unsigned char*** prevFrame, int block_row, int block_col, int* offset_x, int* offset_y) {

}

int calculateMotionVectorMatrixRGB(int video_w, int video_h, unsigned char*** currentFrame, unsigned char*** prevFrame, char*** mvm) {
	
	//i and j are iterating through macro blocks
	for (int i = 0; i < video_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < video_w / MACRO_BLOCK_DIM; j++) {
			int offset_x = 0;
			int offset_y = 0;
			calculateBlockOffset(currentFrame, prevFrame, i, j, &offset_x, &offset_y);
			mvm[i][j][0] = offset_x;
			mvm[i][j][1] = offset_y;
		}
	}

	return 1;
}

