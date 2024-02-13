#include <string.h>

#include "cofgifs.h"

cgif_error_t cgif_init(struct cgif *self, char const *data, char *scratch, size_t scratch_size)
{
    // Base
    self->base = data;

    if(self->base == NULL) {
        return CGIF_ERROR_NOGIF;
    }

    if(strncmp(self->header->magic, "GIF87a", 6) != 0 && strncmp(self->header->magic, "GIF89a", 6) != 0) {
        return CGIF_ERROR_NOGIF;
    }

    // LSD and GCT
    self->lsd = (void*)self->header + sizeof(struct cgif_header);
    self->cursor = (void*)self->lsd + sizeof(struct cgif_lsd);

    if(cgif_gct_enable(self)) {
        self->gct = (struct cgif_rgb const*)self->cursor;
        self->cursor = (void*)self->gct + cgif_gct_size(self);
    } else {
        self->gct = NULL;
    }

    // Scratch
    self->scratch = scratch;
    self->scratch_size = scratch_size;

    if(self->scratch == NULL) {
        return CGIF_ERROR_NOBUF;
    }

    if(cgif_gct_enable(self) && scratch_size < cgif_gct_size(self) * 2) {
        return CGIF_ERROR_BUFSIZE;
    }

    return CGIF_SUCCESS;
}

cgif_error_t cgif_render_next(struct cgif *self, struct cgif_rgb *buffer, size_t buffer_size)
{
    struct cgif_image_descriptor *id = NULL;
    struct cgif_rgb const *color_table = NULL;
    uint8_t color_table_count = 0;

    switch(*self->cursor) {
        case '!': {
            return CGIF_ERROR_NOEXT;
        }

        case ',': {
            id = (struct cgif_image_descriptor *)self->cursor;
            self->cursor += sizeof(struct cgif_image_descriptor);
            break;
        }

        case ';': {
            return CGIF_ERROR_EOF;
        }

        default: {
            return CGIF_ERROR_UNEXPCHAR;
        }
    }

    if(cgif_id_lct_enable(id)) {
        color_table = (void*)self->cursor;
        color_table_count = cgif_id_lct_count(id);
        self->cursor += color_table_count;
    } else {
        color_table = self->gct;
        color_table_count = cgif_gct_count(self);
    }

    // START THE DECOMPRESSION PROCESS WOOOO!
    uint8_t min_code_size = *self->cursor;
    self->cursor++;

    uint8_t code_size = min_code_size + 1;

    struct cgif_dict_entry *dictionary = (struct cgif_dict_entry *)self->scratch;
    uint8_t dictionary_count = 0;

    uint8_t block_size = *self->cursor;
    char const *block_base = ++self->cursor;
    uint8_t bit_index = 0;
    uint16_t chunk = *(uint16_t*)self->cursor;
    uint8_t output_index = 0;
    uint8_t previous_output_index = 0;
    uint16_t bitmask = (uint16_t)-1 >> (16 - code_size);
    uint8_t clear_code = 1 << min_code_size;
    uint8_t stop_code = clear_code + 1;
    while((void*)self->cursor - (void*)block_base < block_size) {
        int8_t bits_remaining = 16 - bit_index;
        if(bits_remaining < code_size) {
            uint16_t next_chunk = *(uint16_t*)++self->cursor;
            chunk |= next_chunk << bit_index;
            bit_index = 0;
        }

        uint16_t code = chunk & bitmask;

        // Discard read bits
        chunk >>= code_size;
        bit_index += code_size;

        // Code is in color table
        uint8_t output_size = 0;
        if(code == clear_code) {
            dictionary_count = 0;
        } else if(code == stop_code) {
            break;
        } else if(code <= color_table_count) { // Code in color table
            memcpy(&buffer[output_index], &color_table[code], sizeof(struct cgif_rgb));
            output_size = 1;
        } else {
            code -= stop_code + 1;

            if(code < dictionary_count) { // Code is in dictionary
                memcpy(&buffer[dictionary[code].index], &buffer[output_index], sizeof(struct cgif_rgb) * dictionary[code].count);
                output_size = dictionary[code].count;
            }
        }

        if(output_index > 0) {
            dictionary[dictionary_count].index = previous_output_index;
            dictionary[dictionary_count].count = output_index - previous_output_index + 1;
            dictionary_count++;

            if(stop_code + dictionary_count + 1 > bitmask) {
                code_size++;
                bitmask = (uint16_t)-1 >> (16 - code_size);
            }
        }

        previous_output_index = output_index;
        output_index += output_size;
    }

    return CGIF_SUCCESS;
}

size_t cgif_gct_size(struct cgif *self)
{
    return cgif_gct_count(self) * sizeof(struct cgif_rgb);
}

uint8_t cgif_gct_count(struct cgif *self)
{
    return (uint8_t)(2 << (self->lsd->gct & 0b00000111));
}

bool cgif_gct_enable(struct cgif *self)
{
    return self->lsd->gct >> 7;
}

uint8_t cgif_gct_resolution(struct cgif *self)
{
    return (self->lsd->gct & 0b01110000) >> 4;
}

bool cgif_gct_sort(struct cgif *self)
{
    return self->lsd->gct & 0b00001000;
}

bool cgif_id_lct_enable(struct cgif_image_descriptor *self)
{
    return self->lct >> 7;
}

uint8_t cgif_id_lct_size(struct cgif_image_descriptor *self)
{
    return (uint8_t)(2 << (self->lct & 0b00000111));
}

uint8_t cgif_id_lct_count(struct cgif_image_descriptor *self)
{
    return cgif_id_lct_size(self) * sizeof(struct cgif_rgb);
}
