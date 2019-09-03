#ifndef MOTION_DETECTION
#define	MOTION_DETECTION

#define VIDEO_W 640
#define VIDEO_H 360

#define MACRO_BLOCK_DIM 16
#define SEARCH_WINDOW_P 7

//distance is based on the manhathan distance, that's the sum of x and y offsets
#define MOVEMENT_TRESH 5
#define BLOCK_DIFF_TRESH 10

/*
All the functions expect a one dimensional representation of a frame for more optimal operations
*/

/*
Takes two consecutive frames as input
Returns motion vector matrix calcualted for the input frames
*/
char* calculateMotionVectorMatrix(int video_w, int video_h, int channels, unsigned char* currentFrame, unsigned char* prevFrame);

/*
Allocates space for the output motion vector matrix
*/
char*** allocateSpaceForMVM(int video_w, int video_h);

/*
Helper function for calculating mvm for 1 channel video (grayscale)
Returns 1 for success, 0 for error
*/
int calculateMotionVectorMatrixGrayscale(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm);

/*
Helper function for calculating mvm for 3 channel video (RGB, HSV...)
Returns 1 for success, 0 for error
*/
int calculateMotionVectorMatrixRGB(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm);

/*
Calculating the offset for the given block on the current frame based on its position on the prev frame
*/
void calculateBlockOffsetExhaustive(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, int* offset_x, int* offset_y);

/*
Allocate space for a frame
*/
unsigned char* allocateSpaceForFrame(int frame_w, int frame_h, int channels);

/*
Draw squares over macroblocks that we suppose are moving
*/
void drawRectangles(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix, float movement_tresh);

/*
Helper function for drawing rects
*/
void drawRectOnBlock(int frame_w, int frame_h, int block_row, int block_col, unsigned char* frame);

int blockDidMove(int frame_w, int frame_h, int block_row, int block_col, char* current_frame, char* prev_frame);

#endif
