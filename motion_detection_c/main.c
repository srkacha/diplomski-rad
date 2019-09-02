#include "motion_detection.h"
#include <stdio.h>
#include <string.h>

int main(int argc, char** argv) {
	unsigned char* currentFrame = allocateSpaceForFrame(1280, 720, 3);
	unsigned char* prevFrame = allocateSpaceForFrame(1280, 720, 3);
	int firstFrameReading = 1;
	long count;

	FILE* pipein = _popen("ffmpeg -i video/teapot.mp4 -f image2pipe -vcodec rawvideo -pix_fmt rgb24 -", "rb");
	FILE* pipeout = _popen("ffmpeg -y -f rawvideo -vcodec rawvideo -pix_fmt rgb24 -s 1280x720 -r 25 -i - -f mp4 -q:v 5 -an -vcodec mpeg4 video/output.mp4", "wb");
	
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
			printf("\n%d %d\n", motionVectorMatrix[0][0][0],motionVectorMatrix[0][0][1]);

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