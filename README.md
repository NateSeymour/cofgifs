# cofgifs

_Pronounced 'sea' of gifs._

A gif decoder library made for embedded devices. 

## Features

- Ultra-low memory consumption.
- No dynamic memory allocation.
- In-place GIF parsing. No extra memory used for storing GIF structures.

## Memory Usage

Reduction of memory usage to the minimum while retaining performance is the main focus of this project. The ``cgif`` 
structure itself only consumes 28 (32-bit addressing) or 56 (64-bit addressing) bytes of memory, mostly pointers to 
specific areas of the 
GIF data itself.

The render buffer is an array of 4 bytes per pixel. The extra byte is empty to help retain alignment of the pixels 
in memory for efficient access by the rendering code. 

Besides the ``cgif`` structure and the render buffer, the library also needs access to space to store its LZW 
dictionary during decompression of the GIF data. Currently (lazily), the library assumes it will need one dictionary 
entry per pixel to be outputted. 

That brings the total memory consumption of the library to 8 bytes per pixel. For animations, half of this space is 
reusable, as the dictionary is discarded between render passes. 

## Portability

This project was originally written with RP2040 processor (Cortex-M0+) in mind, and it leans on the assumption of 
little-endianness. Significant changes would need to be made to support BE processing elements. 

## Example

```c
struct cgif gif;
struct cgif_dict_entry dictionary[CGIF_DICT_COUNT_MAX(32, 16)];
struct cgif_render_rgb render_buffer[512];
cgif_error_t err;

err = cgif_init(&gif, data, dictionary, sizeof(dictionary));
if(err) { /* Handle Error */ }

err = cgif_render_next(&gif, render_buffer, sizeof(render_buffer));
if(err) { /* Handle Error */ }
```

## Limitations

- Currently doesn't support GIF extensions and will return an error when they are encountered.

## Motivation

While working on an embedded project involving a rather large LED matrix, I need a convenient way to create and 
upload color matrices to the device. GIF seemed to be the natural choice for its small file sizes, great compression 
performance and built-in support for animation, not to mention its support by nearly all modern image editors.

None of the libraries I could find met my standards and exact use case of the project. Working with a Raspberry Pi 
Pico W, the available memory was quite limited. Also, working with an embedded device, I wanted to avoid dynamic 
memory allocation when at all possible.