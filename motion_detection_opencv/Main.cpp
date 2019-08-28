#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//global constants
int BLOCK_DIM = 20;
int MV_TRESH = 20;


//functions
VideoWriter* generateWriter(VideoCapture);
Mat* generateMovementDetectionMatrix(Mat, Mat);

int main() {
	VideoCapture capture("video/chaplin.mp4");
	VideoWriter* writer;
	Mat currentFrame;
	Mat prevFrame;

	if (!capture.isOpened()) return -1;

	writer = generateWriter(capture, "detection");

	//iterating through all of the video frames
	while (true) {
		prevFrame = currentFrame;
		capture >> currentFrame;


	}

	capture.release();
	writer->release();
	
	return 0;
}

VideoWriter* generateWriter(VideoCapture capture, string outputName) {
	int height = capture.get(CAP_PROP_FRAME_HEIGHT);
	int width = capture.get(CAP_PROP_FRAME_WIDTH);
	int fps = capture.get(CAP_PROP_FPS);
	VideoWriter* writer = new VideoWriter(outputName + "mp4", VideoWriter::fourcc('M', 'P', '4', 'V'), fps, Size(width,height));
	return writer;
}

Mat* generateMovementDetectionMatrix(Mat currentFrame, Mat prevFrame) {

}