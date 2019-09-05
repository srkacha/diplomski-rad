#ifndef MOTION_DETECTION
#define	MOTION_DETECTION

#define VIDEO_W 1920
#define VIDEO_H 1080

#define MACRO_BLOCK_DIM 16
#define SEARCH_WINDOW_P 7

//distance is based on the manhathan distance, that's the sum of x and y offsets
#define MOVEMENT_TRESH 7
#define BLOCK_SIMILARITY_TRESH 125

//modes for block matching implementation algorithm
#define SEARCH_MODE_EXHAUSTIVE 1
#define SEARCH_MODE_TSS 2
#define SEARCH_MODE_DIAMOND 3


//if set to 0 then there is no limit
#define SEARCH_MODE_DIAMOND_RECURSION_DEPTH_LIMIT 0

//modes for treshold potimization
#define BLOCK_SIMILARITY_OPTIMIZATION_ON 1
#define BLOCK_SIMILARITY_OPTIMIZATION_OFF 0

//color modes for the algorithm
#define COLOR_MODE_RGB 1
#define COLOR_MODE_GRAY 2

/*
All the functions expect a one dimensional representation of a frame for more optimal operations
*/

/*
Takes two consecutive frames as input
Returns motion vector matrix calcualted for the input frames
*/
char*** calculateMotionVectorMatrix(int video_w, int video_h, int color_mode, unsigned char* currentFrame, unsigned char* prevFrame, int mode, int treshold_optimization);

/*
Allocates space for the output motion vector matrix
*/
char*** allocateSpaceForMVM(int video_w, int video_h);

/*
Helper function for calculating mvm for 1 channel video (grayscale)
Returns 1 for success, 0 for error
*/
int calculateMotionVectorMatrixGrayscale(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm, int mode, int treshold_optimization);

/*
Helper function for calculating mvm for 3 channel video (RGB, HSV...)
Returns 1 for success, 0 for error
*/
int calculateMotionVectorMatrixRGB(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm, int mode, int treshold_optimization);

/*
Calculating the offset for the given block on the current frame based on its position on the prev frame, exshaustive method
*/
void calculateBlockOffsetExhaustive(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

/*
Calculating the offset for the given block on the current frame based on its position on the prev frame, exshaustive method, for grayscale input frames
*/
void calculateBlockOffsetExhaustiveGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

/*
Three Step Search implemenatation of the block matching algorithm
*/
void calculateBlockOffsetTSS(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

/*
Three Step Search implemenatation of the block matching algorithm, for grayscale implementation
*/
void calculateBlockOffsetTSSGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

/*
Diamond Search implemenatation of the block matching algorithm
*/
void calculateBlockOffsetDiamond(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

/*
Diamond Search implemenatation of the block matching algorithm, for grayscale implementation
*/
void calculateBlockOffsetDiamondGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y);

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

/*
Optimization method that checks if the given blocks are the same block or not 
*/
int blockDidMove(int frame_w, int frame_h, int block_row, int block_col,unsigned char* current_frame,unsigned char* prev_frame);

/*
Optimization method that checks if the given blocks are the same block or not, for grayscale color mode
*/
int blockDidMoveGray(int frame_w, int frame_h, int block_row, int block_col, unsigned char* current_frame, unsigned char* prev_frame);

/*
Visualisation method to color the frame based on the movemen of the macro blocks
*/
void colorMacroBlocks(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix);

/*
Visualisation method to color the frame based on the movemen of the macro blocks, in reed and blue
*/
void colorMacroBlocksRB(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix);

/*
Convert the color frame array representation to a one byte per pixel grayscale representation
*/
unsigned char* convertRGBtoGray(int frame_w, int frame_h, unsigned char* color_frame);

#endif
