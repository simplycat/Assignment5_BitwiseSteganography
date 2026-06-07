#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "steg.h"

static void print_usage(const char *progname)
{
    fprintf(stderr,
            "Usage:\n"
            "  %s encode <input.ppm> <payload.bin> <output.ppm>\n"
            "  %s decode <input.ppm> <output.bin>\n\n"
            "Description:\n"
            "  Hides/retrieves data in the LSBs of Green and Blue channels of a P3 PPM image\n"
            "  using XOR encoding (1 bit per pixel).\n"
            "  First 16 bits = message length in bytes (big-endian).\n",
            progname, progname);
}

int main(int argc, char *argv[])
{
    if (argc < 2)
    {
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    if (strcmp(argv[1], "encode") == 0)
    {
        if (argc != 5)
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        encode(argv[2], argv[3], argv[4]);
    }
    else if (strcmp(argv[1], "decode") == 0)
    {
        if (argc != 4)
        {
            print_usage(argv[0]);
            return EXIT_FAILURE;
        }
        decode(argv[2], argv[3]);
    }
    else
    {
        fprintf(stderr, "Unknown mode: %s\n", argv[1]);
        print_usage(argv[0]);
        return EXIT_FAILURE;
    }

    return EXIT_SUCCESS;
}
