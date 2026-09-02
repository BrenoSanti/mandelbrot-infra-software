#ifndef MANDELBROT_H
#define MANDELBROT_H

#include <stddef.h>

typedef struct {
    int width;
    int height;
    int max_iterations;
    int num_threads;
} MandelbrotConfig;

void mandelbrot_serial(unsigned char *image, const MandelbrotConfig *config);
void mandelbrot_openmp(unsigned char *image, const MandelbrotConfig *config);
int mandelbrot_pthreads_blocks(unsigned char *image,
                              const MandelbrotConfig *config);
int mandelbrot_pthreads_cyclic(unsigned char *image,
                              const MandelbrotConfig *config);

#endif
