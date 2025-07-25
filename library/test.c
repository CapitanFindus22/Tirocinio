#include <x86intrin.h>
#include "lib.h"

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

int main(int argc, char **argv)
{
    init();

    uint64_t start = __rdtsc();

    RGB *bfr = (RGB *)get_buff();

    int width, height, channels;
    unsigned char *img = stbi_load("image.png", &width, &height, &channels, 0);

    if (img == NULL)
    {
        printf("Error loading image: %s\n", stbi_failure_reason());
        return 1;
    }

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

    togrey(height, width);

    for (short i = 0; i < height; i++)
    {
        for (short j = 0; j < width; j++)
        {
            //printf("\033[38;5;%dm██\033[0m", 232 + (bfr[i * width + j].g * 23) / 255);
        }

        //printf("\n");
    }

    stbi_image_free(img);

    printf("%llu\n", __rdtsc() - start);

    finish();

    return 0;
}
