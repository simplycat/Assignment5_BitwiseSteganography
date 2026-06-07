#include "ppm.h"
#include "utils.h"

// Helper: skip whitespace and comments in PPM file
static void skip_whitespace_and_comments(FILE *fp)
{
    int c;
    while ((c = fgetc(fp)) != EOF)
    {
        if (isspace(c))
        {
            continue;
        }
        else if (c == '#')
        {
            /* Skip comment until end of line */
            while ((c = fgetc(fp)) != EOF && c != '\n')
                ;
        }
        else
        {
            ungetc(c, fp);
            break;
        }
    }
}

PPMImage *read_ppm(const char *filename)
{
    FILE *fp = fopen(filename, "r");
    if (!fp)
    {
        fprintf(stderr, "Error: Cannot open PPM file '%s'\n", filename);
        return NULL;
    }

    PPMImage *img = (PPMImage *)calloc(1, sizeof(PPMImage));
    if (!img)
    {
        fclose(fp);
        return NULL;
    }

    char magic[3];
    if (fscanf(fp, "%2s", magic) != 1 || strcmp(magic, "P3") != 0)
    {
        fprintf(stderr, "Error: Not a valid P3 PPM file\n");
        free(img);
        fclose(fp);
        return NULL;
    }

    skip_whitespace_and_comments(fp);

    if (fscanf(fp, "%d %d", &img->width, &img->height) != 2)
    {
        fprintf(stderr, "Error: Invalid width/height in PPM\n");
        free(img);
        fclose(fp);
        return NULL;
    }

    skip_whitespace_and_comments(fp);

    if (fscanf(fp, "%d", &img->maxval) != 1 || img->maxval != 255)
    {
        fprintf(stderr, "Error: PPM maxval must be 255 (got %d)\n", img->maxval);
        free(img);
        fclose(fp);
        return NULL;
    }

    skip_whitespace_and_comments(fp);

    size_t pixel_count = (size_t)img->width * img->height;
    size_t data_size = pixel_count * 3;
    img->data = (unsigned char *)malloc(data_size);
    if (!img->data)
    {
        free(img);
        fclose(fp);
        return NULL;
    }

    // Read RGB values (ASCII)
    for (size_t i = 0; i < data_size; i++)
    {
        int val;
        if (fscanf(fp, "%d", &val) != 1 || val < 0 || val > 255)
        {
            fprintf(stderr, "Error: Invalid pixel data in PPM at index %zu\n", i);
            free(img->data);
            free(img);
            fclose(fp);
            return NULL;
        }
        img->data[i] = (unsigned char)val;
    }

    fclose(fp);
    return img;
}

int write_ppm(const char *filename, PPMImage *img)
{
    if (!img || !img->data)
        return -1;

    FILE *fp = fopen(filename, "w");
    if (!fp)
    {
        fprintf(stderr, "Error: Cannot create output PPM '%s'\n", filename);
        return -1;
    }

    fprintf(fp, "P3\n%d %d\n255\n", img->width, img->height);

    size_t total = (size_t)img->width * img->height * 3;
    for (size_t i = 0; i < total; i++)
    {
        fprintf(fp, "%d", img->data[i]);
        if ((i + 1) % 3 == 0)
        {
            fprintf(fp, "\n");
        }
        else
        {
            fprintf(fp, " ");
        }
    }

    fclose(fp);
    return 0;
}

void free_ppm(PPMImage *img)
{
    if (img)
    {
        free(img->data);
        free(img);
    }
}

int validate_ppm_header(PPMImage *img)
{
    if (!img)
        return 0;
    if (img->width <= 0 || img->height <= 0)
        return 0;
    if (img->maxval != 255)
        return 0;
    if (!img->data)
        return 0;
    return 1;
}
