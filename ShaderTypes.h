#ifndef SHADER_TYPES_H
#define SHADER_TYPES_H

#include <simd/simd.h>

typedef enum VertexInputIndex {
    VertexInputIndexVertices     = 0,
    VertexInputIndexViewportSize = 1,
} VertexInputIndex;

typedef struct {
    simd::float2 position;
    simd::float4 color;
} VertexData;

#endif /* SHADER_TYPES_H */
