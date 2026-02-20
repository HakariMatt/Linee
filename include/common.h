#ifndef COMMON_H
#define COMMON_H

#include <string.h>
#include <stdio.h>
#include <ctype.h>
#include <stdint.h>
#include <stdlib.h>

typedef uint8_t   u8;
typedef uint16_t  u16;
typedef uint32_t  u32;
typedef uint64_t  u64;
typedef int8_t    i8;
typedef int16_t   i16;
typedef int32_t   i32;
typedef int64_t   i64;
typedef size_t    sz;
typedef ssize_t   ssz;

#define RESET     "\x1b[0m"
#define RED       "\x1b[31m"
#define GREEN     "\x1b[32m"
#define YELLOW    "\x1b[33m"
#define BLUE      "\x1b[34m"
#define MAGENTA   "\x1b[35m"
#define CYAN      "\x1b[36m"
#define WHITE     "\x1b[37m"
#define BOLD      "\x1b[1m"
#define DIM       "\x1b[22m"

#define UNSAVED   "\x1b[1;97;41m"
#define SAVED     "\x1b[1;97;42m"

#endif