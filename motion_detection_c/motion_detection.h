#ifndef MOTION_DETECTION
#define	MOTION_DETECTION

#define VIDEO_W 1280
#define VIDEO_H 720

#define MACRO_BLOCK_DIM 16
#define SEARCH_WINDOW_P 7

/*
Takes two consecutive frames as input
Returns motion vector matrix calcualted for the input frames
*/
unsigned char*** calculateMotionVectorMatrix(int video_w, int video_h, int channels, unsigned char*** inputFrame, unsigned char*** outputFrame);

/*
Allocates space for the output motion vector matrix
*/
unsigned char*** allocateSpaceForMVM(int video_w, int video_h, int channels);

#endif
