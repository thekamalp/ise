// InfiniScroll Engine
// ise_gfx.h
// Bitmap header
//
// Kamal Pillai
// 1/13/2019

#ifndef __ISE_BMP_H
#define __ISE_BMP_H

#define ISE_BMP_BI_RGB 0
#define ISE_BMP_BI_RLE8 1
#define ISE_BMP_BI_RLE4 2
#define ISE_BMP_BI_BITFIELDS 3
#define ISE_BMP_BI_JPEG 4
#define ISE_BMP_BI_PNG 5
#define ISE_BMP_BI_ALPHABITFIELDS 6
#define ISE_BMP_BI_CMYK 11
#define ISE_BMP_BI_CMYKRLE8 12
#define ISE_BMP_BI_CMYKRLE4 13

#pragma pack(push, 1)
typedef struct {
    char id[2];
    uint32_t file_size;
    uint16_t unused0;
    uint16_t unused1;
    uint32_t bitmap_offset;
    uint32_t dib_header_size;
    uint32_t width;
    uint32_t height;
    uint16_t num_planes;
    uint16_t bits_per_pixel;
    uint32_t compression;
    uint32_t bitmap_size;
    uint32_t horiz_dpi;
    uint32_t vert_dpi;
    uint32_t palette_colors;
    uint32_t significant_colors;
} ise_bmp_header_t;

typedef struct {
    uint32_t red_mask;
    uint32_t green_mask;
    uint32_t blue_mask;
    uint32_t alpha_mask;
} ise_bmp_mask_header_t;
#pragma pack(pop)

#endif // #ifndef __ISE_BMP_H
