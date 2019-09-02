#include "motion_detection.h"
#include <stdlib.h>

unsigned char*** calculateMotionVectorMatrix(int video_w, int video_h, int channels, unsigned char*** inputFrame, unsigned char*** outputFrame){
	unsigned char*** mvm = allocateSpaceForMVM(video_w, video_h, channels);
	mvm[0][0][1] = 33;
	return mvm;
}

unsigned char*** allocateSpaceForMVM(int video_w, int video_h, int channels) {
	//alocating rows
	unsigned char*** mvm = (unsigned char***)malloc(video_h * sizeof(unsigned char**));
	
	//alocating cols
	for (int i = 0; i < video_h; i++) {
		mvm[i] = (unsigned char**)malloc(video_w * sizeof(unsigned char*));
		//alocating channels
		for (int j = 0; j < video_w; j++) {
			mvm[i][j] = (unsigned char*)malloc(channels * sizeof(unsigned char));
		}
	}

	return mvm;
}