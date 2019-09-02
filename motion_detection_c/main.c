#include "motion_detection.h"
#include <stdio.h>


int main(int argc, char** argv) {
	unsigned char*** result = calculateMotionVectorMatrix(1280, 720, 3, 0, 0);
	printf("%d", result[0][0][1]);
	return 0;
}