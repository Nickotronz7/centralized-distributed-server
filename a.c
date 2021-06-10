#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"

int main(void) {
     int width, height, channels;
     unsigned char *img = stbi_load("2.png", &width, &height, &channels, 0);
     if(img == NULL) {
         printf("Error in loading the image\n");
         exit(1);
     }
     printf("Loaded image with a width of %dpx, a height of %dpx and %d channels\n", width, height, channels);
    size_t img_size = width * height * channels;
    unsigned char *sepia_img = malloc(img_size);
     if(sepia_img == NULL) {
         printf("Unable to allocate memory for the sepia image.\n");
         exit(1);
     }
 
     // Sepia filter coefficients from https://stackoverflow.com/questions/1061093/how-is-a-sepia-tone-created
     for(unsigned char *p = img, *pg = sepia_img; p != img + img_size; p += channels, pg += channels) {
         *pg       = (uint8_t)fmin(*p ^ 40 , 255.0);         // red
         *(pg + 1) = (uint8_t)fmin(*p ^ 40 , 255.0);         // green
         *(pg + 2) = (uint8_t)fmin(*p ^ 40 , 255.0);         // blue        
         if(channels == 4) {
             *(pg + 3) = *(p + 3);
         }
     }
 
     stbi_write_jpg("sky_sepia.jpg", width, height, channels, sepia_img, 100);     
}