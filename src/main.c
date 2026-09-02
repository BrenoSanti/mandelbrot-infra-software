#define _POSIX_C_SOURCE 200809L

#include "mandelbrot.h"

#include <errno.h>
#include <limits.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <time.h>

typedef void (*ComputeFunction)(unsigned char *, const MandelbrotConfig *);
typedef int (*PthreadFunction)(unsigned char *, const MandelbrotConfig *);

static int parse_positive_int(const char *text, const char *name, int *value)
{
    char *end = NULL;
    long parsed;

    errno = 0;
    parsed = strtol(text, &end, 10);
    if (errno == ERANGE || end == text || *end != '\0' || parsed <= 0 ||
        parsed > INT_MAX) {
        fprintf(stderr, "Erro: %s deve ser um inteiro positivo valido.\n", name);
        return -1;
    }
    *value = (int)parsed;
    return 0;
}

static double elapsed_seconds(const struct timespec *start,
                              const struct timespec *end)
{
    return (double)(end->tv_sec - start->tv_sec) +
           (double)(end->tv_nsec - start->tv_nsec) / 1000000000.0;
}

static int measure_compute(ComputeFunction compute, unsigned char *image,
                           const MandelbrotConfig *config, double *elapsed)
{
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("Erro ao iniciar medicao de tempo");
        return -1;
    }
    compute(image, config);
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("Erro ao finalizar medicao de tempo");
        return -1;
    }
    *elapsed = elapsed_seconds(&start, &end);
    return 0;
}

static int measure_pthreads(PthreadFunction compute, unsigned char *image,
                            const MandelbrotConfig *config, double *elapsed)
{
    struct timespec start;
    struct timespec end;
    if (clock_gettime(CLOCK_MONOTONIC, &start) != 0) {
        perror("Erro ao iniciar medicao de tempo");
        return -1;
    }
    if (compute(image, config) != 0) {
        return -1;
    }
    if (clock_gettime(CLOCK_MONOTONIC, &end) != 0) {
        perror("Erro ao finalizar medicao de tempo");
        return -1;
    }
    *elapsed = elapsed_seconds(&start, &end);
    return 0;
}

static int write_image(const char *filename, const unsigned char *image,
                       const MandelbrotConfig *config)
{
    FILE *file = fopen(filename, "w");
    int y;
    if (file == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar %s.\n", filename);
        return -1;
    }
    for (y = 0; y < config->height; ++y) {
        int x;
        for (x = 0; x < config->width; ++x) {
            if (x > 0 && fputc(' ', file) == EOF) {
                goto write_error;
            }
            if (fprintf(file, "%u", (unsigned int)image[(size_t)y *
                        (size_t)config->width + (size_t)x]) < 0) {
                goto write_error;
            }
        }
        if (fputc('\n', file) == EOF) {
            goto write_error;
        }
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Erro ao fechar %s.\n", filename);
        return -1;
    }
    return 0;

write_error:
    fprintf(stderr, "Erro ao escrever %s.\n", filename);
    (void)fclose(file);
    return -1;
}

static int write_times(const double times[4])
{
    static const char *const names[4] = {
        "serial", "openmp", "pthreads1", "pthreads2"
    };
    FILE *file = fopen("times.txt", "w");
    int i;
    if (file == NULL) {
        fprintf(stderr, "Erro: nao foi possivel criar times.txt.\n");
        return -1;
    }
    for (i = 0; i < 4; ++i) {
        if (fprintf(file, "%s %.9f\n", names[i], times[i]) < 0) {
            fprintf(stderr, "Erro ao escrever times.txt.\n");
            (void)fclose(file);
            return -1;
        }
    }
    if (fclose(file) != 0) {
        fprintf(stderr, "Erro ao fechar times.txt.\n");
        return -1;
    }
    return 0;
}

int main(int argc, char **argv)
{
    static const char *const filenames[4] = {
        "mandelbrot_bgs_serial.pgm",
        "mandelbrot_bgs_openmp.pgm",
        "mandelbrot_bgs_pthreads1.pgm",
        "mandelbrot_bgs_pthreads2.pgm"
    };
    MandelbrotConfig config;
    unsigned char *image;
    size_t pixel_count;
    double times[4];
    int status = EXIT_FAILURE;

    if (argc != 5) {
        fprintf(stderr, "Uso: %s largura altura max_iteracoes num_threads\n", argv[0]);
        return EXIT_FAILURE;
    }
    if (parse_positive_int(argv[1], "largura", &config.width) != 0 ||
        parse_positive_int(argv[2], "altura", &config.height) != 0 ||
        parse_positive_int(argv[3], "max_iteracoes", &config.max_iterations) != 0 ||
        parse_positive_int(argv[4], "num_threads", &config.num_threads) != 0) {
        return EXIT_FAILURE;
    }
    if ((size_t)config.width > SIZE_MAX / (size_t)config.height) {
        fprintf(stderr, "Erro: overflow no tamanho da imagem.\n");
        return EXIT_FAILURE;
    }
    pixel_count = (size_t)config.width * (size_t)config.height;
    image = malloc(pixel_count);
    if (image == NULL) {
        fprintf(stderr, "Erro: nao foi possivel alocar a imagem.\n");
        return EXIT_FAILURE;
    }

    if (measure_compute(mandelbrot_serial, image, &config, &times[0]) != 0 ||
        write_image(filenames[0], image, &config) != 0 ||
        measure_compute(mandelbrot_openmp, image, &config, &times[1]) != 0 ||
        write_image(filenames[1], image, &config) != 0 ||
        measure_pthreads(mandelbrot_pthreads_blocks, image, &config, &times[2]) != 0 ||
        write_image(filenames[2], image, &config) != 0 ||
        measure_pthreads(mandelbrot_pthreads_cyclic, image, &config, &times[3]) != 0 ||
        write_image(filenames[3], image, &config) != 0 ||
        write_times(times) != 0) {
        goto cleanup;
    }
    status = EXIT_SUCCESS;

cleanup:
    free(image);
    return status;
}
