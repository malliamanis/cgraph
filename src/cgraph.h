#ifndef CGRAPH_H
#define CGRAPH_H

#include <stddef.h>
#include <stdint.h>

typedef int8_t   i8;
typedef int16_t  i16;
typedef int32_t  i32;
typedef int64_t  i64;

typedef uint8_t  ui8;
typedef uint16_t ui16;
typedef uint32_t ui32;
typedef uint64_t ui64;

typedef float f32;
typedef double f64;

typedef size_t usize;

void cgraph_run(ui32 width, ui32 height, ui32 pixel_width);

#endif

