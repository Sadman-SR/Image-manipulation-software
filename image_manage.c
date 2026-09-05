#include <stdio.h>
#include <stdlib.h>
#include "image_manage.h"

#pragma pack(push, 1)
typedef struct { uint16_t type; uint32_t filesize; uint32_t reserved; uint32_t dataoffset; } BMPFileHeader;
typedef struct { uint32_t size; int32_t width; int32_t height; uint16_t planes; uint16_t bitsperpixel; uint32_t compression; char xyz[20]; } BMPInfoHeader;
#pragma pack(pop)

Image* create_image(int width, int height) {
    Image *img = (Image*) malloc(sizeof(Image)); // memory allocation for image struct...
    if (!img) return NULL;
    img->width = width;
    img->height = height;
    img->data = (Pixel*) malloc(width * height * sizeof(Pixel));// memory allocation for all pixels...
    return img;
}

void destroy_image(Image *img) {
    if (img) {
        if (img->data) free(img->data);
        free(img);
    }
}

Image* copy_image(Image *src) {
    if (!src || !src->data) return NULL;
    Image *dst = create_image(src->width, src->height);
    if (!dst) return NULL;
    for (int i = 0; i < src->width * src->height; i++) dst->data[i] = src->data[i];
    return dst;
}

Image* open_image(const char *filename) //const because we dont want to change the file.....
{
    FILE *fp = fopen(filename, "rb");
    if (!fp) return NULL;//test if file exists or not....

    BMPFileHeader fileheader;
    BMPInfoHeader infoheader;

    if (fread(&fileheader, sizeof(BMPFileHeader), 1, fp) != 1 ||
        fread(&infoheader, sizeof(BMPInfoHeader), 1, fp) != 1) {
        fclose(fp);  // tests if reading succesful or failed........
        return NULL;
    }

    if (fileheader.type != 0x4D42 || infoheader.bitsperpixel != 24 || infoheader.compression != 0) {
        fclose(fp);
        return NULL;
    }

    int width = infoheader.width;
    int is_top_down = (infoheader.height < 0);
    int height = abs(infoheader.height);

    Image *img = create_image(width, height);
    if (!img) { fclose(fp); return NULL; }

    if (fseek(fp, fileheader.dataoffset, SEEK_SET) != 0) {
        destroy_image(img);
        fclose(fp);
        return NULL;
    }

    int padding = (4 - (width * sizeof(Pixel)) % 4) % 4;

    for (int i = 0; i < height; i++) {
        int y = is_top_down ? i : (height - 1 - i);
        for (int x = 0; x < width; x++) {
            if (fread(&img->data[y * width + x], sizeof(Pixel), 1, fp) != 1) {
                destroy_image(img);
                fclose(fp);
                return NULL;
            }
        }
        fseek(fp, padding, SEEK_CUR);
    }
    fclose(fp);
    return img;
}

int save_image(Image *img, const char *filename) {
    if (!img || !img->data) return 0;
    FILE *fp = fopen(filename, "wb");
    if (!fp) return 0;

    int width = img->width, height = img->height;
    int padding = (4 - (width * sizeof(Pixel)) % 4) % 4;
    uint32_t image_size = (width * sizeof(Pixel) + padding) * height;

    BMPFileHeader fileheader = { 0x4D42, sizeof(BMPFileHeader) + sizeof(BMPInfoHeader) + image_size, 0, 54 };
    BMPInfoHeader infoheader = { sizeof(BMPInfoHeader), width, height, 1, 24, 0 };

    fwrite(&fileheader, sizeof(BMPFileHeader), 1, fp);
    fwrite(&infoheader, sizeof(BMPInfoHeader), 1, fp);

    uint8_t zero = 0;
    for (int y = height - 1; y >= 0; y--) {
        for (int x = 0; x < width; x++) {
            fwrite(&img->data[y * width + x], sizeof(Pixel), 1, fp);
        }
        for (int p = 0; p < padding; p++) fwrite(&zero, 1, 1, fp);
    }
    fclose(fp);
    return 1;
}