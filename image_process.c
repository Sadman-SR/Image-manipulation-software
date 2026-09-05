#include <stdlib.h>
#include "image_process.h"

// Helper Utility - for RGB limits 
// to keep the input value from 0-255 as it the pixel range....
static inline int clamp_int(int val) {
    if (val < 0) return 0;
    if (val > 255) return 255;
    return val;
}

// 1. Grayscale
void apply_grayscale(Image *img) {
    
    if (!img) return; // checks if there is an image....
    for (int i = 0; i < img->width * img->height; i++)
     {
        uint8_t gray = (uint8_t)(0.299 * img->data[i].r +
                                 0.587 * img->data[i].g +
                                 0.114 * img->data[i].b);
        img->data[i].r = gray;
        img->data[i].g = gray; 
        img->data[i].b = gray;
    }
}

// 2. Brightness
void adjust_brightness(Image *img, int amount)
{
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) {
        img->data[i].r = (uint8_t)clamp_int(img->data[i].r + amount);
        img->data[i].g = (uint8_t)clamp_int(img->data[i].g + amount);
        img->data[i].b = (uint8_t)clamp_int(img->data[i].b + amount);
    }
}

// 3. Invert Colors
void invert_colors(Image *img)
{
    if (!img) return;
    for (int i = 0; i < img->width * img->height; i++) 
        {
            img->data[i].r = 255 - img->data[i].r;
            img->data[i].g = 255 - img->data[i].g;
            img->data[i].b = 255 - img->data[i].b;
        }
}

// 4. Horizontal Flip
void flip_horizontal(Image *img) {
    if (!img) return;
    for (int y = 0; y < img->height; y++) 
    {
        for (int x = 0; x < img->width / 2; x++) //untill mid
        {
            int idx1 = y * img->width + x;  //pixel from left
            int idx2 = y * img->width + (img->width - 1 - x); //pixel from right
            Pixel temp = img->data[idx1];
            img->data[idx1] = img->data[idx2];   //pixel swap
            img->data[idx2] = temp;
        }
    }
}

// 5. Vertical Flip
void flip_vertical(Image *img) {
    if (!img) return;
    for (int y = 0; y < img->height / 2; y++) 
    {
        for (int x = 0; x < img->width; x++) 
         {
            int idx1 = y * img->width + x; //pixel from top
            int idx2 = (img->height - 1 - y) * img->width + x; // last row 1st pixel...
            Pixel temp = img->data[idx1];
            img->data[idx1] = img->data[idx2]; //pixel swap...
            img->data[idx2] = temp;
        }
    }
}

// 6. Rotate 90 Degrees Clockwise
Image* rotate_90_cw(Image *img) {
    if (!img) return NULL;
    int new_width = img->height;
    int new_height = img->width;
    Image *rotated = create_image(new_width, new_height);
    if (!rotated) return NULL;

    for (int y = 0; y < img->height; y++) 
    {
        for (int x = 0; x < img->width; x++) 
        {
            rotated->data[x * new_width + (img->height - 1 - y)] = img->data[y * img->width + x];
        }
    }
    return rotated;
}

// 7. Crop Image
Image* crop_image(Image *img, int crop_x, int crop_y, int crop_w, int crop_h) 
// crop_X=starting x position, y=starting  y position, w=width,h-height
{
    if (!img || !img->data) return NULL;
    if (crop_x < 0) crop_x = 0; 
    if (crop_y < 0) crop_y = 0;
    if (crop_x >= img->width || crop_y >= img->height) return NULL;
    if (crop_x + crop_w > img->width) crop_w = img->width - crop_x;
    if (crop_y + crop_h > img->height) crop_h = img->height - crop_y;
    if (crop_w <= 0 || crop_h <= 0) return NULL;

    Image *cropped = create_image(crop_w, crop_h);
    if (!cropped) return NULL;

    for (int y = 0; y < crop_h; y++) {
        for (int x = 0; x < crop_w; x++) {
            cropped->data[y * crop_w + x] = img->data[(crop_y + y) * img->width + (crop_x + x)];
        }
    }
    return cropped;
}

// 8. Blur Image 
Image* blur_image(Image *img) {
    if (!img || !img->data) return NULL;
    
    Image *new_img = create_image(img->width, img->height);
    if (!new_img) return NULL;

    for (int y = 0; y < img->height; y++) {
        for (int x = 0; x < img->width; x++) {
            int sum_r = 0, sum_g = 0, sum_b = 0, count = 0;

            for (int dy = -1; dy <= 1; dy++) {
                for (int dx = -1; dx <= 1; dx++) {
                    int ny = y + dy;
                    int nx = x + dx;

                    if (nx >= 0 && nx < img->width && ny >= 0 && ny < img->height) {
                        int idx = ny * img->width + nx;
                        sum_r += img->data[idx].r;
                        sum_g += img->data[idx].g;
                        sum_b += img->data[idx].b;
                        count++;
                    }
                }
            }
            int dst_idx = y * img->width + x;
            new_img->data[dst_idx].r = (uint8_t)clamp_int(sum_r / count);
            new_img->data[dst_idx].g = (uint8_t)clamp_int(sum_g / count);
            new_img->data[dst_idx].b = (uint8_t)clamp_int(sum_b / count);
        }
    }
    return new_img;
}