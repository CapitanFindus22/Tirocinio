#include "lib.h"
#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#define ROWS 512
#define COLS 512

int main(int argc, char **argv)
{
    init();

    RGB *bfr = (RGB *)get_buff();

    int width, height, channels;
    unsigned char *img = stbi_load("image3.jpg", &width, &height, &channels, 0);

    if (img == NULL)
    {
        printf("Errore nel caricamento dell'immagine: %s\n", stbi_failure_reason());
        return 1;
    }

    int img_index;

    for (int i = 0; i < ROWS; i++)
    {
        for (int j = 0; j < COLS; j++)
        {
            img_index = (i * width + j) * channels;
            bfr[i * COLS + j].r = img[img_index + 0];
            bfr[i * COLS + j].g = img[img_index + 1];
            bfr[i * COLS + j].b = img[img_index + 2];
        }
    }

    convol(ROWS, COLS);

    for (short i = 0; i < ROWS; i++)
    {
        for (short j = 0; j < COLS; j++)
        {
            printf("\033[38;5;%dm██\033[0m", 232 + (bfr[i * COLS + j].g * 23) / 255);
        }

        printf("\n");
    }

    stbi_image_free(img);

    finish((void *)bfr);

    return 0;
}
