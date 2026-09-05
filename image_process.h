#ifndef IMAGE_PROCESS_H
#define IMAGE_PROCESS_H

#include "image_manage.h"

// --- Image Manipulation Functions ---
void apply_grayscale(Image *img);
void adjust_brightness(Image *img, int amount);
void invert_colors(Image *img);
void flip_horizontal(Image *img);
void flip_vertical(Image *img);
Image* rotate_90_cw(Image *img);
Image* crop_image(Image *src, int crop_x, int crop_y, int crop_w, int crop_h);
Image* blur_image(Image *src);

#endif