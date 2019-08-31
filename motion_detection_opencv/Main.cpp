#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

//global constants
int BLOCK_DIM = 10;
int DETECTION_RECT_DIM = 6;
int MV_TRESH = 20;
float MV_DETECTION_TRESH = 0.5;

int MACRO_BLOCK_DIM = 16;
int MACRO_BLOCK_P = 7;


//functions
VideoWriter* generateWriter(VideoCapture, string);
Mat* generateMovementDetectionMatrix(Mat);
Mat* generateFrameDifference(Mat, Mat);
Mat* generateBinaryDifference(Mat, int);
Mat drawMotionRects(Mat , Mat );
void drawRectOnFrame(Mat*, int, int, int, int);
Mat generateMotionVectorMatrix(Mat, Mat);

int main() {
	VideoCapture capture("video/chaplin.mp4");
	VideoWriter* writer;
	Mat currentFrame;
	Mat prevFrame;
	Mat* difference;
	Mat* binaryDifference;
	Mat* movementMatrix;
	Mat markedFrame;

	if (!capture.isOpened()) return -1;

	writer = generateWriter(capture, "video/detection.avi");

	//iterating through all of the video frames
	while (true) {
		//we have to clone it because prevFrame and currFrame are references to objects
		prevFrame = currentFrame.clone();
		capture >> currentFrame;

		if (currentFrame.empty()) break;
		
		if (prevFrame.rows != 0) {
			difference = generateFrameDifference(currentFrame, prevFrame);
			binaryDifference = generateBinaryDifference(*difference, MV_TRESH);
			movementMatrix = generateMovementDetectionMatrix(*binaryDifference);
			markedFrame = drawMotionRects(currentFrame, *movementMatrix);
			writer->write(markedFrame);
		}
	}

	capture.release();
	writer->release();

	destroyAllWindows();
	
	return 0;
}

VideoWriter* generateWriter(VideoCapture capture, string outputName) {
	int height = capture.get(CAP_PROP_FRAME_HEIGHT);
	int width = capture.get(CAP_PROP_FRAME_WIDTH);
	int fps = capture.get(CAP_PROP_FPS);
	VideoWriter* writer = new VideoWriter(outputName, VideoWriter::fourcc('F','M', 'P', '4'), fps, Size(width,height));
	return writer;
}

Mat* generateMovementDetectionMatrix(Mat binaryDifference) {
	int rows = binaryDifference.rows/BLOCK_DIM;
	int cols = binaryDifference.cols/BLOCK_DIM;
	Mat* result = new Mat(rows, cols, 0);
	int whiteCount = 0;

	//now we calcualte the matrix values
	for (int blockRow = 0; blockRow < rows; blockRow++) {
		for (int blockCol = 0; blockCol < cols; blockCol++) {
			//now we got the blocks
			whiteCount = 0;
			for (int pixelRow = blockRow * BLOCK_DIM; pixelRow < blockRow * BLOCK_DIM + BLOCK_DIM; pixelRow++) {
				for (int pixelCol = blockCol * BLOCK_DIM * 3; pixelCol < (blockCol * BLOCK_DIM + BLOCK_DIM) * 3; pixelCol += 3) {
					if (binaryDifference.at<unsigned char>(pixelRow, pixelCol) == 255) whiteCount++;
				}
			}
			if (whiteCount / (float)(BLOCK_DIM * BLOCK_DIM) >= MV_DETECTION_TRESH) {
				result->at<unsigned char>(blockRow, blockCol) = 1;
			} else result->at<unsigned char>(blockRow, blockCol) = 0;
		}
	}

	return result;
}

Mat* generateFrameDifference(Mat currentFrame, Mat prevFrame) {
	Mat* result = new Mat();
	absdiff(currentFrame, prevFrame, *result);
	return result;
}

Mat* generateBinaryDifference(Mat frameDifference, int treshold) {
	Mat* result = new Mat();
	cv::threshold(frameDifference, *result, treshold, 255, cv::THRESH_BINARY);
	return result;
}

Mat drawMotionRects(Mat currentFrame, Mat movementMatrix) {
	Mat clone = (currentFrame.clone());
	for (int i = 0; i < movementMatrix.rows; i++) {
		for (int j = 0; j < movementMatrix.cols; j++) {
			if (movementMatrix.at<unsigned char>(i, j) == 1) {
				drawRectOnFrame(&clone, i, j, DETECTION_RECT_DIM, DETECTION_RECT_DIM);
			}
		}
	}
	return clone;
}

void drawRectOnFrame(Mat* frame, int blockRow, int blockCol, int height, int width) {
	int startRow = blockRow * BLOCK_DIM + ((BLOCK_DIM - height) / 2);
	int startCol = (blockCol * BLOCK_DIM + ((BLOCK_DIM - width) / 2)) * 3;
	for (int i = startRow; i < startRow + height; i++) {
		for (int j = startCol; j < (startCol + width*3); j += 3) {
			if (i == startRow || i == startRow + height - 1) {
				frame->at<unsigned char>(i, j) = 0;
				frame->at<unsigned char>(i, j + 1) = 255;
				frame->at<unsigned char>(i, j + 2) = 0;
			}
			else if (j == startCol || j == startCol + width*3 - 3) {
				frame->at<unsigned char>(i, j) = 0;
				frame->at<unsigned char>(i, j + 1) = 255;
				frame->at<unsigned char>(i, j + 2) = 0;
			}
		}
	}
}

Mat generateMotionVectorMatrix(Mat currentFrame, Mat prevFrame) {
	int vectorDim = 2;
	int matrixRows = currentFrame.rows;
	int matrixCols = currentFrame.cols;
	int type = 0;
	const int matSize[] = { matrixRows, matrixCols, vectorDim };
	Mat motionVectorMatrix(3, matSize, type);

	//iterating through the frame macro blocks
	int blockRows = matrixRows / MACRO_BLOCK_DIM;
	int blockCols = matrixCols / MACRO_BLOCK_DIM;
	//for(int i = 0; i < )

	return motionVectorMatrix;
}