#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H

#include <simd/simd.h>

typedef struct {
    simd::float3 position;
    simd::float2 textureCoordinate;
    simd::float3 normal;
} Vertex;

#endif /* SHADER_TYPES_H */
