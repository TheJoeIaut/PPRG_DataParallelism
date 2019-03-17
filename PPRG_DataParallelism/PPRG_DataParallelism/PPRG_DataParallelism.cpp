#include <omp.h>
#include <stdio.h>

#define NUM_THREADS 4

void helloWorld()
{
#pragma omp parallel num_threads(NUM_THREADS)
	{
		int ID = omp_get_thread_num();
		printf("hello(%d)", ID);
		printf(" world(%d)", ID);
	}
}

//void calcPix(int px, int py) {
//	float cx, cy = normalizeToViewRectangle(px, py, minx, miny, maxx, maxy);
//	float zx = cx;
//	float zy = cy;
//	for (int n = 0; n < maxIterations; n++) {
//		float x = (zx * zx - zy * zy) + cx;
//		float y = (zy * zx + zx * zy) + cy;
//		if ((x * x + y * y) > 4) {
//			// diverge , produce nice color
//			// Todo: paint pixel px, py in a color depending on n
//				return;
//		}
//		zx = x;
//		zy = y;
//	}
//	// Todo: paint pixel px, py black // in the set
//}

int main()
{
	helloWorld();
	return 0;
}

