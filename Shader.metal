#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

typedef struct {
    float4 position [[position]];
    float2 textureCoordinate;
} RasterizerData;

vertex RasterizerData vertexShader (
        uint vertexID [[vertex_id]],
        constant Vertex *vertices [[buffer(0)]],
        constant vector_uint2 *viewportSizePointer [[buffer(1)]],
        constant float4x4 &mvp [[buffer(2)]],
        constant float4x4 &camera [[buffer(3)]]) {
    
    RasterizerData out;
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = vertices[vertexID].position.x;
    out.position.y = vertices[vertexID].position.z;
    out.position.z = vertices[vertexID].position.y;
    out.position = camera * mvp * out.position;
    out.textureCoordinate = vertices[vertexID].textureCoordinate;
    return out;
}

fragment float4 fragmentShader (RasterizerData in [[stage_in]], texture2d<float> colorTexture [[ texture(0) ]]) {

    constexpr sampler textureSampler(filter::nearest, address::clamp_to_zero, mip_filter::nearest, lod_clamp(1.0f, MAXFLOAT), max_anisotropy(16));
    return colorTexture.sample(textureSampler, in.textureCoordinate);
}

