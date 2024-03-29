#include <string.h>

#include "cofgifs.h"

cgif_error_t cgif_init(struct cgif *self, char const *data, struct cgif_dict_entry *dictionary, size_t dict_size)
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
    self->dictionary = dictionary;
    self->dict_size = dict_size;

    if(self->dictionary == NULL) {
        return CGIF_ERROR_NODICT;
    }

    if(self->dict_size < CGIF_DICT_COUNT_MAX(self->lsd->dimension.width, self->lsd->dimension.height) * sizeof(struct cgif_dict_entry)) {
        return CGIF_ERROR_DICTSIZE;
    }

    return CGIF_SUCCESS;
}

cgif_error_t cgif_render_next(struct cgif *self, struct cgif_render_rgb *buffer, size_t buffer_size)
{
    if(buffer_size < sizeof(struct cgif_render_rgb) * self->lsd->dimension.height * self->lsd->dimension.width) {
        return CGIF_ERROR_BUFSIZE;
    }

    struct cgif_image_descriptor *id = NULL;
    struct cgif_rgb const *color_table = NULL;

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

    // Get color table size
    uint8_t color_table_count = 0;

    if(cgif_id_lct_enable(id)) {
        color_table = (void*)self->cursor;
        color_table_count = cgif_id_lct_count(id);
        self->cursor += color_table_count;
    } else {
        color_table = self->gct;
        color_table_count = cgif_gct_count(self);
    }

    // START THE DECOMPRESSION PROCESS WOOOO!
    uint8_t min_code_size = *self->cursor++;
    uint8_t block_size = *self->cursor++;
    char const *block_base = self->cursor;

    uint8_t code_size = min_code_size + 1;
    uint8_t clear_code = 1 << min_code_size;
    uint8_t stop_code = clear_code + 1;

    uint32_t chunk = 0;
    uint8_t bits_remaining = 0;
    uint32_t bitmask = (uint32_t)-1 >> (32 - code_size);

    unsigned int output_index = 0;
    unsigned int previous_output_index = 0;
    unsigned int output_size;
    while((void*)self->cursor - (void*)block_base < block_size) {
        // Expand chunk if empty
        if(bits_remaining < code_size) {
            uint16_t next_chunk = *(uint16_t*)self->cursor;
            chunk |= (((uint32_t)next_chunk) << bits_remaining);
            bits_remaining += 16;
            self->cursor += 2;
        }

        // Read incoming code
        uint16_t code = chunk & bitmask;

        // Discard used bits
        chunk >>= code_size;
        bits_remaining -= code_size;

        // Decode incoming code
        if(code == clear_code) {
            self->dict_count = 0;
            code_size = min_code_size + 1;
            bitmask = (uint32_t)-1 >> (32 - code_size);
            continue;
        } else if(code == stop_code) {
            break;
        } else if(code <= color_table_count) { // Code in color table
            memcpy(&buffer[output_index], &color_table[code], sizeof(struct cgif_render_rgb));
            output_size = 1;
        } else {
            code -= stop_code + 1;

            // Preempted dictionary entry
            if(self->dict_count - code == 1) {
                memcpy(&buffer[output_index], &buffer[previous_output_index], sizeof(struct cgif_render_rgb));
            }

            memcpy(&buffer[output_index], &buffer[self->dictionary[code].index], sizeof(struct cgif_render_rgb) * self->dictionary[code].count);
            output_size = self->dictionary[code].count;
        }

        // Make new dictionary entry
        if(stop_code + self->dict_count + 1 > bitmask) {
            code_size++;
            bitmask = (uint32_t)-1 >> (32 - code_size);
        }

        self->dictionary[self->dict_count].index = output_index;
        self->dictionary[self->dict_count].count = output_size + 1;
        self->dict_count++;

        if(self->dict_count * sizeof(struct cgif_dict_entry) >= self->dict_size) {
            return CGIF_ERROR_DICTOVERFLOW;
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
