#ifndef PPM_H
#define PPM_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <ctype.h>

// Simple PPM image structure (P3 ASCII format, 24-bit RGB)
typedef struct
{
    int width;
    int height;
    int maxval;          // Should be 255
    unsigned char *data; // Flat array: R G B R G B ... (row-major)
} PPMImage;

// Reads an ASCII PPM (P3) file
// Returns NULL on error (invalid format, file issues...)
// Caller must free the returned image with free_ppm()
PPMImage *read_ppm(const char *filename);

// Writes a PPMImage to an ASCII PPM (P3) file.
// Returns 0 on success, non-zero on failure.
int write_ppm(const char *filename, PPMImage *img);

// Frees a PPMImage and its data
void free_ppm(PPMImage *img);

// Validates basic PPM header (P3, dims, maxval==255)
// Returns 1 if valid, 0 otherwise
int validate_ppm_header(PPMImage *img);

#endif // PPM_H
