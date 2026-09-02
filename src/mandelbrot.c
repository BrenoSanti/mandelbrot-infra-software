#include "mandelbrot.h"

#include <pthread.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>

typedef enum {
    PARTITION_BLOCKS,
    PARTITION_CYCLIC
} Partition;

typedef struct {
    unsigned char *image;
    const MandelbrotConfig *config;
    int thread_id;
    Partition partition;
} WorkerArgs;

static unsigned char pixel_intensity(int x, int y,
                                     const MandelbrotConfig *config)
{
    const double real = -2.0 + 3.0 * (double)x / (double)config->width;
    const double imaginary = -1.5 + 3.0 * (double)y / (double)config->height;
    double zr = 0.0;
    double zi = 0.0;
    int iterations = 0;

    while (iterations < config->max_iterations && zr * zr + zi * zi <= 4.0) {
        const double next_zr = zr * zr - zi * zi + real;
        zi = 2.0 * zr * zi + imaginary;
        zr = next_zr;
        ++iterations;
    }

    if (iterations == config->max_iterations) {
        return 0;
    }
    return (unsigned char)(((unsigned long long)iterations * 255ULL) /
                           (unsigned int)config->max_iterations);
}

static void calculate_row(unsigned char *image, int y,
                          const MandelbrotConfig *config)
{
    int x;
    for (x = 0; x < config->width; ++x) {
        image[(size_t)y * (size_t)config->width + (size_t)x] =
            pixel_intensity(x, y, config);
    }
}

void mandelbrot_serial(unsigned char *image, const MandelbrotConfig *config)
{
    int y;
    for (y = 0; y < config->height; ++y) {
        calculate_row(image, y, config);
    }
}

void mandelbrot_openmp(unsigned char *image, const MandelbrotConfig *config)
{
    int y;
#pragma omp parallel for schedule(static) num_threads(config->num_threads)
    for (y = 0; y < config->height; ++y) {
        calculate_row(image, y, config);
    }
}

static void *worker(void *argument)
{
    WorkerArgs *args = argument;
    const MandelbrotConfig *config = args->config;
    int y;

    if (args->partition == PARTITION_BLOCKS) {
        const int base = config->height / config->num_threads;
        const int remainder = config->height % config->num_threads;
        const int extra = args->thread_id < remainder ? 1 : 0;
        const int start = args->thread_id * base +
                          (args->thread_id < remainder ? args->thread_id : remainder);
        const int end = start + base + extra;
        for (y = start; y < end; ++y) {
            calculate_row(args->image, y, config);
        }
    } else {
        for (y = args->thread_id; y < config->height; y += config->num_threads) {
            calculate_row(args->image, y, config);
        }
    }
    return NULL;
}

static int run_pthreads(unsigned char *image, const MandelbrotConfig *config,
                        Partition partition)
{
    pthread_t *threads;
    WorkerArgs *args;
    int created = 0;
    int failed = 0;
    int i;

    if ((size_t)config->num_threads > SIZE_MAX / sizeof(*threads) ||
        (size_t)config->num_threads > SIZE_MAX / sizeof(*args)) {
        fprintf(stderr, "Erro: overflow na alocacao das threads.\n");
        return -1;
    }
    threads = malloc((size_t)config->num_threads * sizeof(*threads));
    args = malloc((size_t)config->num_threads * sizeof(*args));
    if (threads == NULL || args == NULL) {
        fprintf(stderr, "Erro: nao foi possivel alocar estruturas das threads.\n");
        free(args);
        free(threads);
        return -1;
    }

    for (i = 0; i < config->num_threads; ++i) {
        int error;
        args[i].image = image;
        args[i].config = config;
        args[i].thread_id = i;
        args[i].partition = partition;
        error = pthread_create(&threads[i], NULL, worker, &args[i]);
        if (error != 0) {
            fprintf(stderr, "Erro: pthread_create falhou para a thread %d (codigo %d).\n",
                    i, error);
            failed = 1;
            break;
        }
        ++created;
    }

    for (i = 0; i < created; ++i) {
        const int error = pthread_join(threads[i], NULL);
        if (error != 0) {
            fprintf(stderr, "Erro: pthread_join falhou para a thread %d (codigo %d).\n",
                    i, error);
            failed = 1;
        }
    }

    free(args);
    free(threads);
    return failed ? -1 : 0;
}

int mandelbrot_pthreads_blocks(unsigned char *image,
                              const MandelbrotConfig *config)
{
    return run_pthreads(image, config, PARTITION_BLOCKS);
}

int mandelbrot_pthreads_cyclic(unsigned char *image,
                              const MandelbrotConfig *config)
{
    return run_pthreads(image, config, PARTITION_CYCLIC);
}
