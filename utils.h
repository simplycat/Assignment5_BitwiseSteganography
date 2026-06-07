#ifndef UTILS_H
#define UTILS_H

#include <stdio.h>
#include <stdlib.h>
#include <stdint.h>

// Bit manipulation helpers

// Returns the least significant bit (0 or 1)
int get_lsb(unsigned char value);

// Sets the LSB of *value to the given bit (0 or 1)
void set_lsb(unsigned char *value, int bit);

// Adjusts the LSBs of green and blue so that (g & 1) ^ (b & 1) == bit
// Uses minimal change (toggles B if necessary). Keeps original G
void write_bit(unsigned char *g, unsigned char *b, int bit);

// Extracts the encoded bit: (g & 1) ^ (b & 1)
int read_bit(unsigned char g, unsigned char b);

// Error handling helper
void die(const char *msg);

// File util
int file_exists(const char *filename);

#endif // UTILS_H
