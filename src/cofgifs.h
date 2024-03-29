#ifndef COFGIFS_COFGIFS_H
#define COFGIFS_COFGIFS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    CGIF_SUCCESS = 0,

    CGIF_ERROR_NOGIF,
    CGIF_ERROR_NODICT,
    CGIF_ERROR_DICTSIZE,
    CGIF_ERROR_DICTOVERFLOW,
    CGIF_ERROR_UNEXPCHAR,
    CGIF_ERROR_EOF,
    CGIF_ERROR_NOEXT,
    CGIF_ERROR_BUFSIZE,
    CGIF_ERROR_BUFOVERFLOW,
} cgif_error_t;

typedef enum {
    CGIF_EXT_GRAPHIC_CONTROL = 0xF9,
    CGIF_EXT_COMMENT = 0xFE,
} cgif_extmagic_t;

struct __attribute__((packed)) cgif_coordinate
{
    uint8_t x;
    uint8_t y;
};

struct __attribute__((packed)) cgif_dimension
{
    uint16_t width;
    uint16_t height;
};

struct __attribute__((packed)) cgif_header
{
    char const magic[6];
};

struct __attribute__((packed)) cgif_lsd
{
    struct cgif_dimension dimension;

    uint8_t gct;
    uint8_t background_index;
    uint8_t pixel_aspect;
};

struct cgif_render_rgb
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
    uint8_t _pad;
};

struct __attribute__((packed)) cgif_rgb
{
    uint8_t red;
    uint8_t green;
    uint8_t blue;
};

struct __attribute__((packed)) cgif_image_descriptor
{
    uint8_t seperator;

    struct cgif_coordinate left;
    struct cgif_coordinate top;

    struct cgif_dimension size;

    uint8_t lct;
};

struct __attribute__((packed)) cgif_dict_entry
{
    uint16_t count;
    uint16_t index;
};

#define CGIF_DICT_COUNT_MAX(w, h) (w * h)

struct cgif
{
    union
    {
        char const *base;
        struct cgif_header const *header;
    };

    struct cgif_lsd const *lsd;
    struct cgif_rgb const *gct;

    struct cgif_dict_entry *dictionary;
    size_t dict_count;
    size_t dict_size;

    char const *cursor;
};

cgif_error_t cgif_init(struct cgif *self, char const *data, struct cgif_dict_entry *dictionary, size_t dict_size);
cgif_error_t cgif_render_next(struct cgif *self, struct cgif_render_rgb *buffer, size_t buffer_size);

inline bool cgif_gct_enable(struct cgif *self);
inline uint8_t cgif_gct_resolution(struct cgif *self);
inline bool cgif_gct_sort(struct cgif *self);
inline size_t cgif_gct_size(struct cgif *self);
inline uint8_t cgif_gct_count(struct cgif *self);

inline bool cgif_id_lct_enable(struct cgif_image_descriptor *self);
inline bool cgif_id_lct_interlace(struct cgif_image_descriptor *self);
inline bool cgif_id_lct_sort(struct cgif_image_descriptor *self);
inline uint8_t cgif_id_lct_resolution(struct cgif_image_descriptor *self);
inline uint8_t cgif_id_lct_size(struct cgif_image_descriptor *self);
inline uint8_t cgif_id_lct_count(struct cgif_image_descriptor *self);

#endif //COFGIFS_COFGIFS_H
