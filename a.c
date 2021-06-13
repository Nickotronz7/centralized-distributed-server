#include <stdio.h>
#include <stdlib.h>

#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#define STB_IMAGE_WRITE_IMPLEMENTATION
#include "stb/stb_image_write.h"



int filtrxor(char* name,int xor) {
     int width, height, channels;
     unsigned char *img = stbi_load(name, &width, &height, &channels, 0);
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
 
     for(unsigned char *p = img, *pg = sepia_img; p != img + img_size; p += channels, pg += channels) {
         *pg       = (uint8_t)fmin(*p ^ xor , 255.0);         // red
         *(pg + 1) = (uint8_t)fmin(*p ^ xor , 255.0);         // green
         *(pg + 2) = (uint8_t)fmin(*p ^ xor , 255.0);         // blue        
         if(channels == 4) {
             *(pg + 3) = *(p + 3);
         }
     }
     sleep(1);
        
     stbi_write_jpg("sky_sepia.jpg", width, height, channels, sepia_img, 100);     
}

void main(void){
    char* a ="a";

    // strcat(a,"xor");
    puts(a);
    // filtrxor("2.png",255);
}