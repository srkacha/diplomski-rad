#include "motion_detection.h"
#include <stdlib.h>
#include <stdio.h>
#include <math.h>


char*** calculateMotionVectorMatrix(int video_w, int video_h, int color_mode, unsigned char* currentFrame, unsigned char* prevFrame, int mode, int treshold_optimization){
	char*** mvm = allocateSpaceForMVM(video_w, video_h);
	int result = 0;

	//if the video frames are grayscale
	if (color_mode == COLOR_MODE_GRAY) {
		result = calculateMotionVectorMatrixGrayscale(video_w, video_h, currentFrame, prevFrame, mvm, mode, treshold_optimization);
	//if the video frames are RGB/HSV/something other
	}
	else if (color_mode == COLOR_MODE_RGB) {
		result = calculateMotionVectorMatrixRGB(video_w, video_h, currentFrame, prevFrame, mvm, mode, treshold_optimization);
	}

	//checking for errors
	if (result == 0) return 0;

	return mvm;
}

unsigned char* allocateSpaceForFrame(int frame_w, int frame_h, int channels) {
	unsigned char* frame = (unsigned char*)calloc(frame_h * frame_w * channels, sizeof(unsigned char));
	return frame;
}


char*** allocateSpaceForMVM(int video_w, int video_h) {
	//alocating rows
	 char*** mvm = ( char***)calloc((video_h / MACRO_BLOCK_DIM), sizeof( char**));
	
	//alocating cols
	for (int i = 0; i < video_h/MACRO_BLOCK_DIM; i++) {
		mvm[i] = ( char**)calloc((video_w / MACRO_BLOCK_DIM), sizeof( char*));
		//alocating channels
		for (int j = 0; j < video_w / MACRO_BLOCK_DIM; j++) {
			mvm[i][j] = ( char*)calloc(2, sizeof( char));
		}
	}

	return mvm;
}

unsigned char* convertRGBtoGray(int frame_w, int frame_h, unsigned char* color_frame) {
	unsigned char* gray_frame = allocateSpaceForFrame(frame_w, frame_h, 1);
	int red_value = 0;
	int green_value = 0;
	int blue_value = 0;
	int gray_value = 0;

	for (int i = 0; i < frame_h * frame_w; i++) {
			red_value = color_frame[i*3 + 0];
			green_value = color_frame[i * 3 + 1];
			blue_value = color_frame[i * 3 + 2];
			gray_value = (red_value + green_value + blue_value) / 3;
			gray_frame[i] = gray_value;
	}

	return gray_frame;
}

int calculateMotionVectorMatrixGrayscale(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm, int mode, int treshold_optimization) {
	//first we convert the frames to grayscale
	unsigned char* current_frame_gray = convertRGBtoGray(video_w, video_h, currentFrame);
	unsigned char* prev_frame_gray = convertRGBtoGray(video_w, video_h, prevFrame);

	//i and j are iterating through macro blocks
	for (int i = 0; i < video_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < video_w / MACRO_BLOCK_DIM; j++) {
			char offset_x = 0;
			char offset_y = 0;

			//first we check if the block did move based on the movement treshold
			//this makes the algorithm run a lot faster because it's skipping a lot of unnecessary block matching
			if ((treshold_optimization && blockDidMoveGray(video_w, video_h, i, j, current_frame_gray, prev_frame_gray)) || !treshold_optimization) {
				if (mode == SEARCH_MODE_EXHAUSTIVE) {
					calculateBlockOffsetExhaustiveGray(video_w, video_h, current_frame_gray, prev_frame_gray, i, j, &offset_x, &offset_y);
				}
				else if (mode == SEARCH_MODE_TSS) {
					calculateBlockOffsetTSSGray(video_w, video_h, current_frame_gray, prev_frame_gray, i, j, &offset_x, &offset_y);
				}
				else if (mode == SEARCH_MODE_DIAMOND) {
					calculateBlockOffsetDiamondGray(video_w, video_h, current_frame_gray, prev_frame_gray, i, j, &offset_x, &offset_y);
				}
				else {
					//mode not found, so we return 0 for error
					return 0;
				}
			}

			mvm[i][j][0] = offset_y;
			mvm[i][j][1] = offset_x;
		}

	}

	return 1;
}

int calculateMotionVectorMatrixRGB(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, char*** mvm, int mode, int treshold_optimization) {

	//i and j are iterating through macro blocks
	for (int i = 0; i < video_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < video_w / MACRO_BLOCK_DIM; j++) {
			char offset_x = 0;
			char offset_y = 0;

			//first we check if the block did move based on the movement treshold
			//this makes the algorithm run a lot faster because it's skipping a lot of unnecessary block matching
			if ((treshold_optimization && blockDidMove(video_w, video_h, i, j, currentFrame, prevFrame)) || !treshold_optimization) {
				if (mode == SEARCH_MODE_EXHAUSTIVE) {
					calculateBlockOffsetExhaustive(video_w, video_h, currentFrame, prevFrame, i, j, &offset_x, &offset_y);
					
				}
				else if (mode == SEARCH_MODE_TSS) {
					calculateBlockOffsetTSS(video_w, video_h, currentFrame, prevFrame, i, j, &offset_x, &offset_y);
				}
				else if (mode == SEARCH_MODE_DIAMOND) {
					calculateBlockOffsetDiamond(video_w, video_h, currentFrame, prevFrame, i, j, &offset_x, &offset_y);
				}
				else {
					//mode not found, so we return 0 for error
					return 0;
				}
			}
			
			mvm[i][j][0] = offset_y;
			mvm[i][j][1] = offset_x;
		}

	}

	return 1;
}


void calculateBlockOffsetExhaustive(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	//first we need to calculate the window size
	int start_px_row = ((block_row * MACRO_BLOCK_DIM )- SEARCH_WINDOW_P <=0 ) ? 0 : (block_row * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P;
	int start_px_col = ((block_col * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P <= 0) ? 0 : (block_col * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P;
	int end_px_row = ((block_row * MACRO_BLOCK_DIM) + MACRO_BLOCK_DIM + SEARCH_WINDOW_P >= video_h) ? video_h - MACRO_BLOCK_DIM : (block_row * MACRO_BLOCK_DIM) + SEARCH_WINDOW_P;
	int end_px_col = ((block_col * MACRO_BLOCK_DIM) + MACRO_BLOCK_DIM + SEARCH_WINDOW_P >= video_w) ? video_w - MACRO_BLOCK_DIM : (block_col * MACRO_BLOCK_DIM) + SEARCH_WINDOW_P;
	int min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	int temp_difference = 0;

	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	//now comes the block matching part
	for (int i = start_px_row; i < end_px_row; i++) {
		for (int j = start_px_col; j < end_px_col; j++) {
			//here we calculate the difference of the blocks
			temp_difference = 0;
			for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
				for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
					temp_difference += (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 0] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 0])
						* (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 0] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 0]);
					temp_difference += (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 1] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 1])
						 * (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 1] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 1]);
					temp_difference += (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 2] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 2])
						 * (currentFrame[video_w * 3 * (current_frame_row + k) + (current_frame_col + p) * 3 + 2] - prevFrame[video_w * 3 * (i + k) + (j + p) * 3 + 2]);
				}
			}
			temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM * 3;

			if (temp_difference < min_difference) {
				min_diff_row_offset = (block_row * MACRO_BLOCK_DIM - i);
				min_diff_col_offset = (block_col * MACRO_BLOCK_DIM - j);
				min_difference = temp_difference;
			}
		}
	}

	//we set the output values
	*offset_x = min_diff_col_offset;
	*offset_y = min_diff_row_offset;
}


void calculateBlockOffsetExhaustiveGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	//first we need to calculate the window size
	int start_px_row = ((block_row * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P <= 0) ? 0 : (block_row * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P;
	int start_px_col = ((block_col * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P <= 0) ? 0 : (block_col * MACRO_BLOCK_DIM) - SEARCH_WINDOW_P;
	int end_px_row = ((block_row * MACRO_BLOCK_DIM) + MACRO_BLOCK_DIM + SEARCH_WINDOW_P >= video_h) ? video_h - MACRO_BLOCK_DIM : (block_row * MACRO_BLOCK_DIM) + SEARCH_WINDOW_P;
	int end_px_col = ((block_col * MACRO_BLOCK_DIM) + MACRO_BLOCK_DIM + SEARCH_WINDOW_P >= video_w) ? video_w - MACRO_BLOCK_DIM : (block_col * MACRO_BLOCK_DIM) + SEARCH_WINDOW_P;
	int min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	int temp_difference = 0;

	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	//now comes the block matching part
	for (int i = start_px_row; i < end_px_row; i++) {
		for (int j = start_px_col; j < end_px_col; j++) {
			//here we calculate the difference of the blocks
			temp_difference = 0;
			for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
				for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
					temp_difference += (currentFrame[video_w * (current_frame_row + k) + (current_frame_col + p) + 0] - prevFrame[video_w * (i + k) + (j + p) + 0])
						* (currentFrame[video_w * (current_frame_row + k) + (current_frame_col + p) + 0] - prevFrame[video_w  * (i + k) + (j + p)  + 0]);
				}
			}
			temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM;

			if (temp_difference < min_difference) {
				min_diff_row_offset = (block_row * MACRO_BLOCK_DIM - i);
				min_diff_col_offset = (block_col * MACRO_BLOCK_DIM - j);
				min_difference = temp_difference;
			}
		}
	}

	//we set the output values
	*offset_x = min_diff_col_offset;
	*offset_y = min_diff_row_offset;
}

void calculateBlockOffsetTSS(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	int min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	int temp_difference = 0;
	
	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	int current_frame_fixed_row = current_frame_row;
	int current_frame_fixed_col = current_frame_col;

	for (int step_size = 4; step_size > 0; step_size /= 2) {
		for (int i = -step_size; i <= step_size; i += step_size) {
			for (int j = -step_size; j <= step_size; j += step_size) {

				//now we check if the position is valid, if it is then we do the comparation
				if ((current_frame_row + i >= 0) && (current_frame_row + i + MACRO_BLOCK_DIM < video_h) && (current_frame_col + j >= 0) && (current_frame_row + j + MACRO_BLOCK_DIM < video_w)) {
					temp_difference = 0;
					for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
						for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
							temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 0] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 0])
								* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 0] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 0]);
							temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 1] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 1])
								* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 1] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 1]);
							temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 2] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 2])
								* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 2] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 2]);
						}
					}

					temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM * 3;

					if (temp_difference < min_difference) {
						min_diff_row_offset = i;
						min_diff_col_offset = j;
						min_difference = temp_difference;
					}
				}
			}
		}
		//now we can change the current frame position
		current_frame_row += min_diff_row_offset;
		current_frame_col += min_diff_col_offset;

		min_diff_col_offset = 0;
		min_diff_row_offset = 0;
		min_difference = INT_MAX;
	}

	//we set the output values
	int old_current_row = block_row * MACRO_BLOCK_DIM;
	int old_current_col = block_col * MACRO_BLOCK_DIM;
	*offset_x = old_current_col - current_frame_col;
	*offset_y = old_current_row - current_frame_row;
}

void calculateBlockOffsetTSSGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	int min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	int temp_difference = 0;

	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	int current_frame_fixed_row = current_frame_row;
	int current_frame_fixed_col = current_frame_col;

	for (int step_size = 4; step_size > 0; step_size /= 2) {
		for (int i = -step_size; i <= step_size; i += step_size) {
			for (int j = -step_size; j <= step_size; j += step_size) {

				//now we check if the position is valid, if it is then we do the comparation
				if ((current_frame_row + i >= 0) && (current_frame_row + i + MACRO_BLOCK_DIM < video_h) && (current_frame_col + j >= 0) && (current_frame_row + j + MACRO_BLOCK_DIM < video_w)) {
					temp_difference = 0;
					for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
						for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
							temp_difference = (currentFrame[video_w  * (current_frame_fixed_row + k) + (current_frame_fixed_col + p)  + 0] - prevFrame[video_w  * (current_frame_row + i + k) + (current_frame_col + j + p)  + 0])
								* (currentFrame[video_w * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) + 0] - prevFrame[video_w * (current_frame_row + i + k) + (current_frame_col + j + p)  + 0]);
						}
					}

					temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM;

					if (temp_difference < min_difference) {
						min_diff_row_offset = i;
						min_diff_col_offset = j;
						min_difference = temp_difference;
					}
				}
			}
		}
		//now we can change the current frame position
		current_frame_row += min_diff_row_offset;
		current_frame_col += min_diff_col_offset;

		min_diff_col_offset = 0;
		min_diff_row_offset = 0;
		min_difference = INT_MAX;
	}

	//we set the output values
	int old_current_row = block_row * MACRO_BLOCK_DIM;
	int old_current_col = block_col * MACRO_BLOCK_DIM;
	*offset_x = old_current_col - current_frame_col;
	*offset_y = old_current_row - current_frame_row;
}


void calculateBlockOffsetDiamond(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	float min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	float temp_difference = 0;

	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	int current_frame_fixed_row = current_frame_row;
	int current_frame_fixed_col = current_frame_col;

	int step_size = 2;
	int diamond_recursion_done = 0;

	//needed for the recursion depth limit
	int steps = 0;

	while (diamond_recursion_done == 0) {
		//we run this block of code until we find the match in the center on one of the recursion diamonds

		for (int i = -step_size; i <= step_size; i++) {
			for (int j = -step_size; j <= step_size; j++) {
				//check if the coordinates match the diamond pattern
				if (abs(i) + abs(j) == step_size || (i == 0 && j == 0)) {
					// now we check if the position is valid, if it is then we do the comparation
					if ((current_frame_row + i >= 0) && (current_frame_row + i + MACRO_BLOCK_DIM < video_h) && (current_frame_col + j >= 0) && (current_frame_row + j + MACRO_BLOCK_DIM < video_w)) {
						temp_difference = 0;
						for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
							for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
								temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 0] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 0])
									* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 0] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 0]);
								temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 1] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 1])
									* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 1] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 1]);
								temp_difference = (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 2] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 2])
									* (currentFrame[video_w * 3 * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) * 3 + 2] - prevFrame[video_w * 3 * (current_frame_row + i + k) + (current_frame_col + j + p) * 3 + 2]);
							}
						}

						temp_difference /= (float)(MACRO_BLOCK_DIM * MACRO_BLOCK_DIM * 3);

						if (temp_difference < min_difference) {
							min_diff_row_offset = i;
							min_diff_col_offset = j;
							min_difference = temp_difference;
						}
					}
				}
			}
		}
		
		//now we can change the current frame position
		current_frame_row += min_diff_row_offset;
		current_frame_col += min_diff_col_offset;

		//if this was the big diamond pattern, we make the next one small
		if (step_size == 2) {
			step_size = 1;
		}
		else if(min_diff_col_offset == 0 && min_diff_row_offset == 0) { // this means we found the best match in the center of the diamond so we stop the algorithm
			diamond_recursion_done = 1;
		}

		min_diff_col_offset = 0;
		min_diff_row_offset = 0;
		min_difference = INT_MAX;

		//increase the step counter after a full cycle
		steps += 1;

		//now we check if the recursion depth limit is reached, if there is a limit
		//0 value means there is no limit
		if (SEARCH_MODE_DIAMOND_RECURSION_DEPTH_LIMIT && steps == SEARCH_MODE_DIAMOND_RECURSION_DEPTH_LIMIT) {
			diamond_recursion_done = 1;
		}
	}

	//we set the output values
	int old_current_row = block_row * MACRO_BLOCK_DIM;
	int old_current_col = block_col * MACRO_BLOCK_DIM;
	*offset_x = old_current_col - current_frame_col;
	*offset_y = old_current_row - current_frame_row;
}

void calculateBlockOffsetDiamondGray(int video_w, int video_h, unsigned char* currentFrame, unsigned char* prevFrame, int block_row, int block_col, char* offset_x, char* offset_y) {
	float min_difference = INT_MAX;
	int min_diff_row_offset = 0;
	int min_diff_col_offset = 0;
	float temp_difference = 0;

	int current_frame_row = block_row * MACRO_BLOCK_DIM;
	int current_frame_col = block_col * MACRO_BLOCK_DIM;

	int current_frame_fixed_row = current_frame_row;
	int current_frame_fixed_col = current_frame_col;

	int step_size = 2;
	int diamond_recursion_done = 0;

	//needed for the recursion depth limit
	int steps = 0;

	while (diamond_recursion_done == 0) {
		//we run this block of code until we find the match in the center on one of the recursion diamonds

		for (int i = -step_size; i <= step_size; i++) {
			for (int j = -step_size; j <= step_size; j++) {
				//check if the coordinates match the diamond pattern
				if (abs(i) + abs(j) == step_size || (i == 0 && j == 0)) {
					// now we check if the position is valid, if it is then we do the comparation
					if ((current_frame_row + i >= 0) && (current_frame_row + i + MACRO_BLOCK_DIM < video_h) && (current_frame_col + j >= 0) && (current_frame_row + j + MACRO_BLOCK_DIM < video_w)) {
						temp_difference = 0;
						for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
							for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
								temp_difference = (currentFrame[video_w  * (current_frame_fixed_row + k) + (current_frame_fixed_col + p)  + 0] - prevFrame[video_w  * (current_frame_row + i + k) + (current_frame_col + j + p) + 0])
									* (currentFrame[video_w  * (current_frame_fixed_row + k) + (current_frame_fixed_col + p) + 0] - prevFrame[video_w  * (current_frame_row + i + k) + (current_frame_col + j + p)  + 0]);
							}
						}

						temp_difference /= (float)(MACRO_BLOCK_DIM * MACRO_BLOCK_DIM );

						if (temp_difference < min_difference) {
							min_diff_row_offset = i;
							min_diff_col_offset = j;
							min_difference = temp_difference;
						}
					}
				}
			}
		}

		//now we can change the current frame position
		current_frame_row += min_diff_row_offset;
		current_frame_col += min_diff_col_offset;

		//if this was the big diamond pattern, we make the next one small
		if (step_size == 2) {
			step_size = 1;
		}
		else if (min_diff_col_offset == 0 && min_diff_row_offset == 0) { // this means we found the best match in the center of the diamond so we stop the algorithm
			diamond_recursion_done = 1;
		}

		min_diff_col_offset = 0;
		min_diff_row_offset = 0;
		min_difference = INT_MAX;

		//increase the step counter after a full cycle
		steps += 1;

		//now we check if the recursion depth limit is reached, if there is a limit
		//0 value means there is no limit
		if (SEARCH_MODE_DIAMOND_RECURSION_DEPTH_LIMIT && steps == SEARCH_MODE_DIAMOND_RECURSION_DEPTH_LIMIT) {
			diamond_recursion_done = 1;
		}
	}

	//we set the output values
	int old_current_row = block_row * MACRO_BLOCK_DIM;
	int old_current_col = block_col * MACRO_BLOCK_DIM;
	*offset_x = old_current_col - current_frame_col;
	*offset_y = old_current_row - current_frame_row;
}

void drawRectangles(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix, float movement_tresh) {
	for (int i = 0; i < frame_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < frame_w / MACRO_BLOCK_DIM; j++) {
			int distance = abs(motionMatrix[i][j][0]) + abs(motionMatrix[i][j][1]);
			if (distance >= MOVEMENT_TRESH) {
				drawRectOnBlock(frame_w, frame_h, i, j, frame);
			}
		}
	}
}

void drawRectOnBlock(int frame_w, int frame_h, int block_row, int block_col, unsigned char* frame) {
	int rect_dim = MACRO_BLOCK_DIM / 2;
	int start_row = block_row * MACRO_BLOCK_DIM + (MACRO_BLOCK_DIM - rect_dim) / 2;
	int start_col = block_col * MACRO_BLOCK_DIM + (MACRO_BLOCK_DIM - rect_dim) / 2;
	for (int i = start_row; i < start_row + rect_dim; i++) {
		for (int j = start_col; j < start_col + rect_dim ; j++) {
			if (i == start_row || i == start_row + rect_dim - 1) {
				frame[i * frame_w * 3 + j * 3 + 0] = 0;
				frame[i * frame_w * 3 + j * 3 + 1] = 255;
				frame[i * frame_w * 3 + j * 3 + 2] = 0;
			}
			else if (j == start_col || j == start_col + rect_dim - 1) {
				frame[i * frame_w * 3 + j * 3 + 0] = 0;
				frame[i * frame_w * 3 + j * 3 + 1] = 255;
				frame[i * frame_w * 3 + j * 3 + 2] = 0;
			}
		}
	}
}

int blockDidMove(int frame_w, int frame_h, int block_row, int block_col, unsigned char* current_frame,unsigned char* prev_frame) {
	int temp_difference = 0;
	for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
		for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
			temp_difference += (current_frame[(block_row*MACRO_BLOCK_DIM + k)*frame_w*3 + (block_col)*MACRO_BLOCK_DIM*3 + p*3 + 0] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 0])
				* (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 0] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 0]);
			temp_difference += (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 1] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 1])
				* (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 1] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 1]);
			temp_difference += (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 2] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 2])
				* (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 2] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w * 3 + (block_col)* MACRO_BLOCK_DIM * 3 + p * 3 + 2]);
			
		}
	}
	
	temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM * 3;

	return temp_difference > BLOCK_SIMILARITY_TRESH;
}

int blockDidMoveGray(int frame_w, int frame_h, int block_row, int block_col, unsigned char* current_frame, unsigned char* prev_frame) {
	int temp_difference = 0;
	for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
		for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
			temp_difference += (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w + (block_col)* MACRO_BLOCK_DIM  + p  + 0] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w  + (block_col)* MACRO_BLOCK_DIM  + p  + 0])
				* (current_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w + (block_col)* MACRO_BLOCK_DIM + p + 0] - prev_frame[(block_row * MACRO_BLOCK_DIM + k) * frame_w  + (block_col)* MACRO_BLOCK_DIM + p  + 0]);
		}
	}

	temp_difference /= MACRO_BLOCK_DIM * MACRO_BLOCK_DIM;

	return temp_difference > BLOCK_SIMILARITY_TRESH;
}

void colorMacroBlocks(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix) {
	int distance = 0;
	unsigned char color = 0;
	for (int i = 0; i < frame_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < frame_w / MACRO_BLOCK_DIM; j++) {
			distance = abs(motionMatrix[i][j][0]) + abs(motionMatrix[i][j][1]);
			color = distance * (255 / (SEARCH_WINDOW_P * 2));
			for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
				for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 0] = color;
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 1] = color;
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 2] = color;
				}
			}
		}
	}
}

void colorMacroBlocksRB(int frame_w, int frame_h, unsigned char* frame, char*** motionMatrix) {
	unsigned char color_r = 0;
	unsigned char color_b = 0;
	for (int i = 0; i < frame_h / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j < frame_w / MACRO_BLOCK_DIM; j++) {
			color_r = abs(motionMatrix[i][j][0]) * (255 / (SEARCH_WINDOW_P * 2));
			color_b = abs(motionMatrix[i][j][1]) * (255 / (SEARCH_WINDOW_P * 2));
			for (int k = 0; k < MACRO_BLOCK_DIM; k++) {
				for (int p = 0; p < MACRO_BLOCK_DIM; p++) {
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 0] = color_r;
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 1] = 0;
					frame[frame_w * 3 * (i * MACRO_BLOCK_DIM + k) + j * MACRO_BLOCK_DIM * 3 + p * 3 + 2] = color_b;
				}
			}
		}
	}
}

