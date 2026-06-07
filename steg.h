#ifndef STEG_H
#define STEG_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdint.h>

// Encodes a payload into the LSBs (via G^B XOR) of a PPM image
// The first 16 bits store the payload size in bytes (big-endian)
// Then the actual message bits follow (MSB-first per byte)
void encode(const char *input_ppm, const char *payload_file, const char *output_ppm);

// Decodes a hidden message from a PPM image.
// Reads the first 16 bits as length, then extracts that many bytes.
void decode(const char *input_ppm, const char *output_file);

#endif // STEG_H
