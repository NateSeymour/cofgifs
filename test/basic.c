#include <stdio.h>
#include <stdlib.h>
#include "cofgifs.h"

char *get_file_data()
{
    FILE* file = fopen("test-2.gif", "r");

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
    char *data = get_file_data();

    struct cgif gif;
    struct cgif_dict_entry dictionary[CGIF_DICT_COUNT_MAX(32, 16)];
    struct cgif_render_rgb render_buffer[512];
    cgif_error_t err;

    err = cgif_init(&gif, data, dictionary, sizeof(dictionary));
    if(err) { /* Handle Error */ }

    err = cgif_render_next(&gif, render_buffer, sizeof(render_buffer));
    if(err) { /* Handle Error */ }

    for(int y = 0; y < gif.lsd->dimension.height; y++) {
        for(int x = 0; x < gif.lsd->dimension.width; x++) {
            struct cgif_render_rgb pixel = render_buffer[(y * gif.lsd->dimension.width) + x];
            if(pixel.red == 255 && pixel.green == 255 && pixel.blue == 255) {
                printf("   ");
            } else {
                printf(" X ");
            }
        }

        printf("\n");
    }

    free(data);

    return 0;
}