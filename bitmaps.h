#ifndef BITMAPS_H
#define BITMAPS_H

#include <Arduino.h>

// Declare the bitmap arrays for sun 
extern const unsigned char epd_bitmap__a_frm0_200[];
extern const unsigned char epd_bitmap__a_frm1_200[];
extern const unsigned char epd_bitmap__a_frm2_200[];
extern const unsigned char epd_bitmap__a_frm3_200[];
extern const unsigned char epd_bitmap__a_frm4_200[];
extern const unsigned char epd_bitmap__a_frm5_200[];
extern const unsigned char epd_bitmap__a_frm6_200[];

// Declare the array of all bitmaps and its length
extern const int epd_bitmap_allArray_LEN;
extern const unsigned char* epd_bitmap_allArray[];

// Declare the bitmap arrays for cloud 
extern const unsigned char cloud_a_frm0_200[];
extern const unsigned char cloud_a_frm1_200[];
extern const unsigned char cloud_a_frm2_200[];
extern const unsigned char cloud_a_frm3_200[];
extern const unsigned char cloud_a_frm4_200[];

// Declare the array of all bitmaps and its length
extern const int cloudallArray_LEN;
extern const unsigned char* cloudallArray[];


#endif // BITMAPS_H
