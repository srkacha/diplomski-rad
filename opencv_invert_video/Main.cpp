#include <iostream>
#include <opencv2/opencv.hpp>

using namespace std;
using namespace cv;

Mat* invertFrame(Mat);

int main(int argc, char** argv) {

	VideoCapture capture("video/chaplin.mp4");
	Mat frame; //temp frame where we will load all of the framews on by one
	VideoWriter* videoWriter;

	if (capture.isOpened() == false) {
		return -1;
	}

	//writer to write the video object
	videoWriter = new VideoWriter("video/output.mp4", VideoWriter::fourcc('M', 'P', '4', 'V'), capture.get(CAP_PROP_FPS), Size(capture.get(CAP_PROP_FRAME_WIDTH), capture.get(CAP_PROP_FRAME_HEIGHT)));
	cout << capture.get(CAP_PROP_FPS) << endl;
	cout << capture.get(CAP_PROP_FRAME_WIDTH) << endl;
	cout << capture.get(CAP_PROP_FRAME_HEIGHT) << endl;
	while (true) {
		capture >> frame;

		if (frame.empty()) {
			break;
		}

		Mat* invertedFrame = invertFrame(frame);
		videoWriter->write(*invertedFrame);
		//cout << (unsigned int)frame.at<unsigned char>(359, 1080) << "  " << (unsigned int)(*invertedFrame).at<unsigned char>(333, 639) << endl;
		//imshow("Frame", *invertedFrame);

	}

	//releasing the video object
	capture.release();
	videoWriter->release();

	//closing all the frames
	destroyAllWindows();

	return 0;
}

Mat* invertFrame(Mat originalFrame) {
	Mat* invertedFrame = new Mat(originalFrame.rows, originalFrame.cols, originalFrame.type());
	//cout << originalFrame.cols + originalFrame.rows << endl;
	for (int i = 0; i < originalFrame.rows; i++) {
		for (int j = 0; j < originalFrame.cols * 3; j++) {
			(invertedFrame)->at<unsigned char>(i, j) = 255 - (unsigned int)originalFrame.at<unsigned char>(i, j);
		}
	}
	return invertedFrame;
}