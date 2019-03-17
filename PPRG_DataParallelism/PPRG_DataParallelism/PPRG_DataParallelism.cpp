#include "tga.h"
#include <omp.h>
#include <stdio.h>
#include <chrono>
#include <iostream>

#define NUM_THREADS 5

#define MAX_ITERATIONS 1000

typedef struct {
	char r, g, b;
} Pixel;

#define WIDTH 1024
#define HEIGHT 1024
#define BPP 24

#define MIN_X -2
#define MAX_X  1
#define MIN_Y -1
#define MAX_Y  1

float normalize(int actual, int min, int max, int numberofpixel) {

	int range = max - min;
	return min + (float)range / (float)numberofpixel * actual;
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
	float cx = normalize(px, MIN_X, MAX_X,WIDTH), cy = normalize(py, MIN_Y, MAX_Y, HEIGHT);
	float zx = cx;
	float zy = cy;
	for (int n = 0; n < MAX_ITERATIONS; n++) {
		float x = (zx * zx - zy * zy) + cx;
		float y = (zy * zx + zx * zy) + cy;
		if ((x * x + y * y) > 4) {
			// diverge , produce nice color
			// paint pixel px, py in a color depending on n
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

int renderFrame_serial()
{
	auto start_time = std::chrono::high_resolution_clock::now();

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
			Pixel p = calcPix(px, py);
			image.imageData[currentByte++] = p.r;
			image.imageData[currentByte++] = p.g;
			image.imageData[currentByte++] = p.b;
		}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

	std::cout << "serial took" << time / std::chrono::milliseconds(1) << "ms to run.\n" << std::endl;

	tga::saveTGA(image, "test.tga");

	return time / std::chrono::milliseconds(1);
}

int renderFrame_parallel()
{
	auto start_time = std::chrono::high_resolution_clock::now();
	tga::TGAImage image;

	image.bpp = BPP;
	image.width = WIDTH;
	image.height = HEIGHT;

	int bytesPerPixel = (BPP / 8);

	int imageSize = (bytesPerPixel * WIDTH * HEIGHT);

	image.imageData = std::vector<unsigned char>(imageSize);

	int currentByte = 0;

	omp_set_num_threads(NUM_THREADS);

	#pragma omp parallel for
	for (int i = 0; i < HEIGHT * WIDTH; i++)
	{
		int px = i % WIDTH;
		int py = i / HEIGHT;

		Pixel p = calcPix(px, py);
		image.imageData[i*3] = p.r;
		image.imageData[i * 3 +1] = p.g;
		image.imageData[i * 3 +2] = p.b;
	}

	auto end_time = std::chrono::high_resolution_clock::now();
	auto time = end_time - start_time;

	std::cout << "parallel took" << time / std::chrono::milliseconds(1) << "ms to run.\n" << std::endl;

	tga::saveTGA(image, "test.tga");

	return time / std::chrono::milliseconds(1);
}

int main()
{
	int serialtimes =0, paralleltimes =0, runs =10;

	for (int i = 0; i < runs;i++) {

		serialtimes += renderFrame_serial();
		paralleltimes +=renderFrame_parallel();

	}

	std::cout << "serial took on average " << serialtimes / runs << "ms to run.\n" << std::endl;
	std::cout << "parallel took on average " << paralleltimes / runs << "ms to run.\n" << std::endl;

	return 0;
}

