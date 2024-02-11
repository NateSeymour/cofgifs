#include <assert.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "cofgifs.h"

char *get_file_data()
{
    FILE* file = fopen("test.gif", "r");

    // Get file size
    fseek(file, 0, SEEK_END);
    size_t file_size = ftell(file);
    rewind(file);

    printf("File size: %zu\n", file_size);

    // Read file
    char *data = malloc(file_size);
    fread(data, file_size, 1, file);

    return data;
}

int main()
{
    cgif_error_t err;

    char *data = get_file_data();

    char scratch[1024];

    struct cgif gif;
    err = cgif_init(&gif, data, scratch, 1024);

    struct cgif_render_rgb render_buffer[512];
    memset(render_buffer, 0, 512 * sizeof(struct cgif_render_rgb));

    err = cgif_render_next(&gif, render_buffer, 512 * sizeof(struct cgif_render_rgb));

    for(int y = 0; y < gif.lsd->dimension.height; y++) {
        for(int x = 0; x < gif.lsd->dimension.width; x++) {
            struct cgif_render_rgb pixel = render_buffer[x * y];
            if(pixel.red || pixel.green || pixel.blue) {
                printf(" X ");
            } else {
                printf("   ");
            }
        }

        printf("\n");
    }

    free(data);

    return 0;
}