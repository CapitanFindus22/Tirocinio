#include <x86intrin.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h>

#define STB_IMAGE_IMPLEMENTATION
#include "library/stb_image.h"

typedef struct
{

    uint8_t r, g, b;

} RGB;

void *copy(void *dest, const void *src, size_t n) {
    unsigned char *d = dest;
    const unsigned char *s = src;
    while (n--) {
        *d++ = *s++;
    }
    return dest;
}

int main()
{

    int width, height, channels;
    unsigned char *img = stbi_load("rootfs/image4.png", &width, &height, &channels, 0);

    if (img == NULL)
    {
        printf("Error loading image: %s\n", stbi_failure_reason());
        return 1;
    }

    RGB *bfr = (RGB *)malloc(width * height * sizeof(RGB));

    int img_index;

    for (int i = 0; i < height; i++)
    {
        for (int j = 0; j < width; j++)
        {
            img_index = (i * width + j) * channels;
            bfr[i * width + j].r = img[img_index + 0];
            bfr[i * width + j].g = img[img_index + 1];
            bfr[i * width + j].b = img[img_index + 2];
        }
    }

    uint64_t start = __rdtsc();

    for (size_t i = 0; i < height; i++)
    {

        for (size_t j = 0; j < width; j++)
        {

            bfr[i * width + j].r = (77 * bfr[i * width + j].r + 150 * bfr[i * width + j].g + 29 * bfr[i * width + j].b) >> 8;
            bfr[i * width + j].g = bfr[i * width + j].r;
            bfr[i * width + j].b = bfr[i * width + j].r;
        }
    }

    printf("%llu\n", __rdtsc() - start);

    int kernelX[3][3] = {
        {-1, 0, 1},
        {-2, 0, 2},
        {-1, 0, 1}};

    int kernelY[3][3] = {
        {-1, -2, -1},
        {0, 0, 0},
        {1, 2, 1}};

    RGB *mtr = (RGB *)malloc(height * width * sizeof(RGB));

    int vx, vy;

    for (size_t i = 1; i < height - 1; i++)
    {

        vx = 0;
        vy = 0;

        for (size_t j = 1; j < width - 1; j++)
        {

            vx = bfr[(i - 1) * width + j - 1].g * kernelX[0][0] +
                 bfr[(i - 1) * width + j].g * kernelX[0][1] +
                 bfr[(i - 1) * width + j + 1].g * kernelX[0][2] +
                 bfr[i * width + j - 1].g * kernelX[1][0] +
                 bfr[i * width + j].g * kernelX[1][1] +
                 bfr[i * width + j + 1].g * kernelX[1][2] +
                 bfr[(i + 1) * width + j - 1].g * kernelX[2][0] +
                 bfr[(i + 1) * width + j].g * kernelX[2][1] +
                 bfr[(i + 1) * width + j + 1].g * kernelX[2][2];

            vy = bfr[(i - 1) * width + j - 1].g * kernelY[0][0] +
                 bfr[(i - 1) * width + j].g * kernelY[0][1] +
                 bfr[(i - 1) * width + j + 1].g * kernelY[0][2] +
                 bfr[i * width + j - 1].g * kernelY[1][0] +
                 bfr[i * width + j].g * kernelY[1][1] +
                 bfr[i * width + j + 1].g * kernelY[1][2] +
                 bfr[(i + 1) * width + j - 1].g * kernelY[2][0] +
                 bfr[(i + 1) * width + j].g * kernelY[2][1] +
                 bfr[(i + 1) * width + j + 1].g * kernelY[2][2];

            mtr[i * width + j].g = (uint8_t)sqrt(vx * vx + vy * vy);

            mtr[i * width + j].b = mtr[i * width + j].g;
            mtr[i * width + j].r = mtr[i * width + j].g;
        }
    }

    copy(bfr, mtr, height * width * sizeof(RGB));

    free(mtr);

    printf("%llu\n", __rdtsc() - start);

    stbi_image_free(img);

    return 0;
}
