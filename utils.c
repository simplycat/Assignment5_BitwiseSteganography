#include "utils.h"

int get_lsb(unsigned char value)
{
    return value & 1;
}

void set_lsb(unsigned char *value, int bit)
{
    if (value == NULL)
        return;
    *value = (*value & ~1U) | (bit & 1);
}

void write_bit(unsigned char *g, unsigned char *b, int bit)
{
    if (g == NULL || b == NULL)
        return;

    int current_xor = get_lsb(*g) ^ get_lsb(*b);
    if (current_xor != (bit & 1))
    {
        // Flip B's LSB to make the XOR match the desired bit
        *b ^= 1;
    }
    // G left unchanged
}

int read_bit(unsigned char g, unsigned char b)
{
    return get_lsb(g) ^ get_lsb(b);
}

void die(const char *msg)
{
    fprintf(stderr, "Error: %s\n", msg);
    exit(EXIT_FAILURE);
}

int file_exists(const char *filename)
{
    if (filename == NULL)
        return 0;
    FILE *fp = fopen(filename, "r");
    if (fp)
    {
        fclose(fp);
        return 1;
    }
    return 0;
}
