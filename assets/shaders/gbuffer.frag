#version 460

#extension GL_GOOGLE_include_directive : require
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inUV;
layout (location = 2) in vec3 inNormal;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;
layout (location = 5) in vec4 inCSPos;
layout (location = 6) in vec4 inPrevCSPos;

layout (location = 0) out vec4 outGBufferA; // RGB: Albedo, A: Metallic
layout (location = 1) out vec4 outGBufferB; // RG: Normal, BA: Motion Vector
layout (location = 2) out vec4 outGBufferC; // R: Roughness, G: Curvature, B: Mesh ID, A: Linear Z

layout(set = 2, binding = 0) uniform ubo_material
{
    vec4 materialFactors; // x: alphaCutoff, y: metallicFactor, z: roughnessFactor, w: reserved
	vec4 baseColorFactor;
} materialUBO;

layout (set = 2, binding = 1) uniform sampler2D textureAlbedo;

// A simple utility to convert a float to a 2-component octohedral representation
vec2 direction_to_octohedral(vec3 normal)
{
    vec2 p = normal.xy * (1.0f / dot(abs(normal), vec3(1.0f)));
    return normal.z > 0.0f ? p : (1.0f - abs(p.yx)) * (step(0.0f, p) * 2.0f - vec2(1.0f));
}

vec2 compute_motion_vector(vec4 prev_pos, vec4 current_pos)
{
    // Perspective division, covert clip space positions to NDC.
    vec2 current = (current_pos.xy / current_pos.w);
    vec2 prev    = (prev_pos.xy / prev_pos.w);

    // Remap to [0, 1] range
    current = current * 0.5 + 0.5;
    prev    = prev * 0.5 + 0.5;

    // Calculate velocity (current -> prev)
    return (prev - current);
}

float compute_curvature(float depth)
{
    vec3 dx = dFdx(inNormal);
    vec3 dy = dFdy(inNormal);

    float x = dot(dx, dx);
    float y = dot(dy, dy);

    return pow(max(x, y), 0.5f);
}

void main() 
{
    // G buffer A
	vec4 albedo = texture(textureAlbedo, inUV);
	outGBufferA.rgb = albedo.rgb;
	outGBufferA.a = materialUBO.materialFactors.y;

    // G buffer B
	vec2 packed_normal = direction_to_octohedral(normalize(inNormal));
	vec2 motion_vector = compute_motion_vector(inPrevCSPos, inCSPos);
    outGBufferB = vec4(packed_normal, motion_vector);

	// G buffer C
    float roughness = materialUBO.materialFactors.z;
    float linear_z  = gl_FragCoord.z / gl_FragCoord.w;
    float curvature = compute_curvature(linear_z);
    float mesh_id   = 1.0f;// float(u_PushConstants.mesh_id);

    outGBufferC = vec4(roughness, curvature, mesh_id, linear_z);
}