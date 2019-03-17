#include "tga.h"
#include <omp.h>
#include <stdio.h>

#define NUM_THREADS 4

#define MAX_ITERATIONS 50

typedef struct {
	char r, g, b;
} Pixel;

#define WIDTH 800
#define HEIGHT 600
#define BPP 24

void helloWorld()
{
#pragma omp parallel num_threads(NUM_THREADS)
	{
		int ID = omp_get_thread_num();
		printf("hello(%d)", ID);
		printf(" world(%d)", ID);
	}
}

float normalize(int actual, int min, int max) {
	return (float)actual / (float)max;
}

float hue2rgb(float p, float q, float t) {
	if (t < 0) t += 1.0;
	if (t > 1.0) t -= 1.0;
	if (t < 1.0 / 6.0) return p + (q - p) * 6.0 * t;
	if (t < 1.0 / 2.0) return q;
	if (t < 2.0 / 3.0) return p + (q - p) * (2.0 / 3.0 - t) * 6.0;
	return p;
}

Pixel hsl2Pixel(float h, float s, float l) {
	float r, g, b;

	if (s == 0) {
		r = g = b = l; // achromatic
	}
	else {
		float q = l < 0.5 ? l * (1.0 + s) : l + s - l * s;
		float p = 2.0 * l - q;
		r = hue2rgb(p, q, h + 1.0 / 3.0);
		g = hue2rgb(p, q, h);
		b = hue2rgb(p, q, h - 1.0 / 3.0);
	}

	Pixel p;
	p.r = (char) roundf(r * 255);
	p.g = (char) roundf(g * 255);
	p.b = (char) roundf(b * 255);
	return p;
}

Pixel calcPix(int px, int py) {
	float cx = normalize(px, 0, WIDTH - 1), cy = normalize(py, 0, HEIGHT - 1);
	float zx = cx;
	float zy = cy;
	for (int n = 0; n < MAX_ITERATIONS; n++) {
		float x = (zx * zx - zy * zy) + cx;
		float y = (zy * zx + zx * zy) + cy;
		if ((x * x + y * y) > 4) {
			// diverge , produce nice color
			// Todo: paint pixel px, py in a color depending on n
			
			float hue = (float)n / (float)MAX_ITERATIONS;
			return hsl2Pixel(hue, 0.7, 0.7);
		}
		zx = x;
		zy = y;
	}
	// paint pixel px, py black
	Pixel p;
	p.r = 0;
	p.g = 0;
	p.b = 0;
	return p;
}

void renderFrame_serial()
{
	tga::TGAImage image;

	image.bpp = BPP;
	image.width = WIDTH;
	image.height = HEIGHT;

	int bytesPerPixel = (BPP / 8);

	int imageSize = (bytesPerPixel * WIDTH * HEIGHT);

	image.imageData = std::vector<unsigned char>(imageSize);

	int currentByte = 0;

	for (int py = 0; py < HEIGHT; ++py)
		for (int px = 0; px < WIDTH; ++px)
		{
			image.imageData[currentByte++] = calcPix(px, py).r;
			image.imageData[currentByte++] = calcPix(px, py).g;
			image.imageData[currentByte++] = calcPix(px, py).b;
		}

	tga::saveTGA(image, "test.tga");
}

int main()
{
	//helloWorld();
	renderFrame_serial();
	return 0;
}

