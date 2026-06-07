#include "steg.h"
#include "ppm.h"
#include "utils.h"

#include <stdint.h>

// Read entire payload file into memory. Returns size in bytes or -1 on error.
static long read_payload(const char *filename, unsigned char **out_data)
{
    FILE *fp = fopen(filename, "rb");
    if (!fp)
    {
        fprintf(stderr, "Error: Cannot open payload file '%s'\n", filename);
        return -1;
    }

    fseek(fp, 0, SEEK_END);
    long size = ftell(fp);
    fseek(fp, 0, SEEK_SET);

    if (size <= 0 || size > 65535)
    { // 16-bit length limit
        fprintf(stderr, "Error: Payload size %ld is invalid (must be 1..65535 bytes)\n", size);
        fclose(fp);
        return -1;
    }

    *out_data = (unsigned char *)malloc(size);
    if (!*out_data)
    {
        fclose(fp);
        return -1;
    }

    if (fread(*out_data, 1, size, fp) != (size_t)size)
    {
        free(*out_data);
        *out_data = NULL;
        fclose(fp);
        return -1;
    }

    fclose(fp);
    return size;
}

// Convert 16-bit length + payload into a bitstream buffer
// Returns allocated buffer of bits (0/1 values) and sets *bit_count.
static unsigned char *create_bitstream(const unsigned char *payload, long payload_size,
                                       size_t *bit_count)
{
    size_t total_bits = 16 + (size_t)payload_size * 8;
    unsigned char *bits = (unsigned char *)malloc(total_bits);
    if (!bits)
        return NULL;

    // First 16 bits: payload size as big-endian 16-bit unsigned
    uint16_t len = (uint16_t)payload_size;
    for (int i = 0; i < 16; i++)
    {
        /* MSB first */
        bits[i] = (len >> (15 - i)) & 1;
    }

    // Then payload bits, MSB-first per byte
    size_t bit_idx = 16;
    for (long i = 0; i < payload_size; i++)
    {
        unsigned char byte = payload[i];
        for (int b = 7; b >= 0; b--)
        {
            bits[bit_idx++] = (byte >> b) & 1;
        }
    }

    *bit_count = total_bits;
    return bits;
}

void encode(const char *input_ppm, const char *payload_file, const char *output_ppm)
{
    // Check for duplicate filenames
    if (strcmp(input_ppm, output_ppm) == 0 ||
        strcmp(input_ppm, payload_file) == 0 ||
        strcmp(payload_file, output_ppm) == 0)
    {
        die("Duplicate filenames are not allowed (input, payload, output must be distinct)");
    }

    PPMImage *img = read_ppm(input_ppm);
    if (!img || !validate_ppm_header(img))
    {
        if (img)
            free_ppm(img);
        die("Invalid or unreadable input PPM file");
    }

    unsigned char *payload = NULL;
    long payload_size = read_payload(payload_file, &payload);
    if (payload_size < 0 || !payload)
    {
        free_ppm(img);
        die("Failed to read payload file");
    }

    size_t needed_bits = 0;
    unsigned char *bitstream = create_bitstream(payload, payload_size, &needed_bits);
    if (!bitstream)
    {
        free(payload);
        free_ppm(img);
        die("Memory allocation failed for bitstream");
    }

    size_t total_pixels = (size_t)img->width * img->height;
    if (needed_bits > total_pixels)
    {
        free(bitstream);
        free(payload);
        free_ppm(img);
        die("Payload too large for image capacity (need more pixels)");
    }

    // Encode bits into image
    for (size_t i = 0; i < needed_bits; i++)
    {
        size_t pixel_idx = i; /* 1 bit per pixel, row-major */
        size_t data_idx = pixel_idx * 3;

        unsigned char *g = &img->data[data_idx + 1];
        unsigned char *b = &img->data[data_idx + 2];

        write_bit(g, b, bitstream[i]);
    }

    // Write output
    if (write_ppm(output_ppm, img) != 0)
    {
        free(bitstream);
        free(payload);
        free_ppm(img);
        die("Failed to write output PPM file");
    }

    printf("Successfully encoded %ld bytes (%zu bits) into %s\n",
           payload_size, needed_bits, output_ppm);

    free(bitstream);
    free(payload);
    free_ppm(img);
}

void decode(const char *input_ppm, const char *output_file)
{
    // Check duplicates
    if (strcmp(input_ppm, output_file) == 0)
    {
        die("Input and output filenames must be different");
    }

    PPMImage *img = read_ppm(input_ppm);
    if (!img || !validate_ppm_header(img))
    {
        if (img)
            free_ppm(img);
        die("Invalid or unreadable input PPM file");
    }

    size_t total_pixels = (size_t)img->width * img->height;

    // First, read 16 bits to get length
    if (total_pixels < 16)
    {
        free_ppm(img);
        die("Image too small to contain even the length header");
    }

    uint16_t length = 0;
    for (int i = 0; i < 16; i++)
    {
        size_t pixel_idx = i;
        size_t data_idx = pixel_idx * 3;
        unsigned char g = img->data[data_idx + 1];
        unsigned char b = img->data[data_idx + 2];
        int bit = read_bit(g, b);
        length = (length << 1) | bit;
    }

    size_t message_bits = (size_t)length * 8;
    size_t total_needed = 16 + message_bits;

    if (total_needed > total_pixels)
    {
        fprintf(stderr, "Warning: Declared message size (%u bytes) exceeds image capacity. "
                        "Will decode partial data.\n",
                length);
        // Try to decode what is possible
        message_bits = (total_pixels - 16) / 8 * 8; // truncate to full bytes
        length = message_bits / 8;
    }

    // Allocate output buffer
    unsigned char *message = (unsigned char *)calloc(length, 1);
    if (!message && length > 0)
    {
        free_ppm(img);
        die("Memory allocation failed for decoded message");
    }

    // Read message bits
    for (size_t i = 0; i < message_bits; i++)
    {
        size_t bit_pos = 16 + i;
        size_t pixel_idx = bit_pos;
        size_t data_idx = pixel_idx * 3;

        unsigned char g = img->data[data_idx + 1];
        unsigned char b = img->data[data_idx + 2];
        int bit = read_bit(g, b);

        size_t byte_idx = i / 8;
        int bit_in_byte = 7 - (i % 8); /* MSB first */
        if (bit)
        {
            message[byte_idx] |= (1 << bit_in_byte);
        }
    }

    // Write binary output
    FILE *out = fopen(output_file, "wb");
    if (!out)
    {
        free(message);
        free_ppm(img);
        die("Cannot create output file");
    }

    if (length > 0)
    {
        fwrite(message, 1, length, out);
    }
    fclose(out);

    printf("Successfully decoded %u bytes to %s\n", length, output_file);

    free(message);
    free_ppm(img);
}
