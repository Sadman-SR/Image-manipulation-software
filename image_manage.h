#ifndef IMAGE_MANAGE_H
#define IMAGE_MANAGE_H

#include <stdint.h>

// --- Struct Definitions ---
typedef struct { 
    uint8_t b, g, r; 
} Pixel;
typedef struct {
     int width, height;
      Pixel *data;
     } Image;

// --- Memory & File Functions ---
Image* create_image(int width, int height);
void destroy_image(Image *img);
Image* copy_image(Image *src);
Image* open_image(const char *filename);
int save_image(Image *img, const char *filename);

#endif