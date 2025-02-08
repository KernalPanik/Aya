#pragma once
#include <stdint.h>
#include <stddef.h>

typedef int8_t i8;
typedef int16_t i16;
typedef int32_t i32;
typedef int64_t i64;
typedef uint8_t u8;
typedef uint16_t u16;
typedef uint32_t u32;
typedef uint64_t u64;

typedef float f32;
typedef double f64;

typedef uintptr_t ptr;

typedef struct
{
    f64 p1;
    f64 p2;
} largeF64;

// Tested function that takes the base (first arg), transformer (second arg), and stores the result (third arg)
typedef void(*TestableFunc)(void*, void*, ptr*);

// base, transformer, storage, tag
typedef void(*TransformerFunc)(void*, void*, void**, char**);

// Output comparer fptr, used when producing MRs. It should mostly tell if two vars are equal.
typedef bool(*ComparerFunc)(void*, void*);

// Consumer must override their type.ToString() for PrettyPrint functions
typedef char*(*ToStringFunc)(void*);