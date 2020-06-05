#include <metal_stdlib>
#include <simd/simd.h>

using namespace metal;

#import "ShaderTypes.h"

typedef struct {
    float4 position [[position]];
    float2 textureCoordinate;
    float3 normal;
    float secondsSinceFirstLoaded;
    float distance;
} RasterizerData;

vertex RasterizerData vertexShader (
        uint vertexID [[vertex_id]],
        constant Vertex *vertices [[buffer(0)]],
        constant vector_uint2 *viewportSizePointer [[buffer(1)]],
        constant float4x4 &mvp [[buffer(2)]],
        constant float4x4 &camera [[buffer(3)]],
        constant float &secondsSinceFirstLoaded [[buffer(4)]]
) {
    
    RasterizerData out;
    out.position = vector_float4(0.0, 0.0, 0.0, 1.0);
    out.position.x = vertices[vertexID].position.x;
    out.position.y = vertices[vertexID].position.z;
    out.position.z = vertices[vertexID].position.y;
    out.position = camera * mvp * out.position;
    out.textureCoordinate = vertices[vertexID].textureCoordinate;
    out.normal = vertices[vertexID].normal;
    out.secondsSinceFirstLoaded = secondsSinceFirstLoaded;
    out.distance =  1.0 + 0.001 * exp(dot(out.position.xz, out.position.xz) / 1000);
    return out;
}

fragment float4 fragmentShader (RasterizerData in [[stage_in]], texture2d<float> colorTexture [[ texture(0) ]]) {
    const float3 directionalLight = normalize(vector_float3(0.5, 0.8, 1.0));
    const float brightness = 4.0 / 7.0 + 3 * dot(directionalLight, in.normal) / 7.0;
    constexpr sampler textureSampler(filter::nearest, address::clamp_to_zero, mip_filter::linear, lod_clamp(0.0f, MAXFLOAT), max_anisotropy(16));
    float4 out = brightness * colorTexture.sample(textureSampler, in.textureCoordinate);
    out.a = in.secondsSinceFirstLoaded > 0.5 ? 1.0: 2.0 * in.secondsSinceFirstLoaded;
    out = (1.0 / in.distance) * out + (1.0 - 1.0 / in.distance) * vector_float4(0.707, 0.8125, 0.957, 1);
    out.a = 1.0;
    return out;
}

