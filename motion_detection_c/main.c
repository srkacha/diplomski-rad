#include "motion_detection.h"
#include <stdio.h>
#include <string.h>

void printMat(unsigned char*** mat);

int main(int argc, char** argv) {
	unsigned char* currentFrame = allocateSpaceForFrame(VIDEO_W, VIDEO_H, 3);
	unsigned char* prevFrame = allocateSpaceForFrame(VIDEO_W, VIDEO_H, 3);
	int firstFrameReading = 1;
	long count;

	FILE* pipein = _popen("ffmpeg -i video/input.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "rb");
	FILE* pipeout = _popen("ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt rgb24 -s 640x360 -r 25 -i - -f mp4 -q:v 5 -an -vcodec mpeg4 video/output.mp4", "wb");
	
	unsigned char*** motionVectorMatrix;

	// Process video frames
	while (1)
	{
		//make the current frame the prev one
		memcpy(prevFrame, currentFrame, VIDEO_H * VIDEO_W * 3);

		// Read a frame from the input pipe into the buffer
		count = fread(currentFrame, 1, VIDEO_H * VIDEO_W * 3, pipein);

		// If we didn't get a frame of video, we're probably at the end
		if (count != VIDEO_H * VIDEO_W * 3) break;

		if (!firstFrameReading) {

			// Process this frame
			motionVectorMatrix = calculateMotionVectorMatrix(VIDEO_W, VIDEO_H, 3, currentFrame, prevFrame);
			drawRectangles(VIDEO_W, VIDEO_H, currentFrame, motionVectorMatrix, MOVEMENT_TRESH);
			//printMat(motionVectorMatrix);

			// Write this frame to the output pipe
			fwrite(currentFrame, 1, VIDEO_H * VIDEO_W * 3, pipeout);
		}
		else {
			firstFrameReading = 0;
		}
	}

	// Flush and close input and output pipes
	fflush(pipein);
	_pclose(pipein);
	fflush(pipeout);
	_pclose(pipeout);

	return 0;
}

void printMat(unsigned char*** mat) {
	for (int i = 0; i < VIDEO_H / MACRO_BLOCK_DIM; i++) {
		for (int j = 0; j<VIDEO_W / MACRO_BLOCK_DIM; j++) {
			if (mat[i][j][0] + mat[i][j][1] > MOVEMENT_TRESH) {
				printf("1");
			}
			else printf("0");
		}
		printf("\n");
	}
	printf("\n");
}