#ifndef COFGIFS_COFGIFS_H
#define COFGIFS_COFGIFS_H

#include <stdint.h>
#include <stdbool.h>

typedef enum {
    CGIF_SUCCESS = 0,

    CGIF_ERROR_NOGIF,
    CGIF_ERROR_NOBUF,
    CGIF_ERROR_BUFSIZE,
    CGIF_ERROR_UNEXPCHAR,
    CGIF_ERROR_EOF,
    CGIF_ERROR_NOEXT,
} cgif_error_t;

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
    uint8_t count;
    uint8_t index;
};

struct cgif
{
    union
    {
        char const *base;
        struct cgif_header const *header;
    };

    struct cgif_lsd const *lsd;
    struct cgif_rgb const *gct;

    char *scratch;
    size_t scratch_size;

    char const *cursor;
};

cgif_error_t cgif_init(struct cgif *self, char const *data, char *scratch, size_t scratch_size);
cgif_error_t cgif_render_next(struct cgif *self, struct cgif_rgb *buffer, size_t buffer_size);

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
