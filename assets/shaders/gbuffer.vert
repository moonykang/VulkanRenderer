#version 460

#extension GL_GOOGLE_include_directive : require
#include "common.glsl"

layout (location = 0) in vec3 inPosition;
layout (location = 1) in vec3 inNormal;
layout (location = 2) in vec2 inUV;
layout (location = 3) in vec3 inTangent;
layout (location = 4) in vec3 inBitangent;

layout (location = 0) out vec3 outPos;
layout (location = 1) out vec2 outUV;
layout (location = 2) out vec3 outNormal;
layout (location = 3) out vec3 outTangent;
layout (location = 4) out vec3 outBitangent;
layout (location = 5) out vec4 outCSPos;
layout (location = 6) out vec4 outPrevCSPos;

layout(set = 0, binding = 0) uniform PerFrameUBO
{
    mat4  view_inverse;
    mat4  proj_inverse;
    mat4  view_proj_inverse;
    mat4  prev_view_proj;
    mat4  view_proj;
    vec4  cam_pos;
    vec4  current_prev_jitter;
    Light light;
    uint num_frames;
    uint inverse_scale;
} globalUBO;

layout(set = 1, binding = 0) uniform ubo_mesh
{
    mat4 model;
} meshUBO;

void main()
{
    // Transform position into world space
    vec4 world_pos = meshUBO.model * vec4(inPosition, 1.0);

    // Since this demo has static scenes we can use the current Model matrix as the previous one
    vec4 prev_world_pos = meshUBO.model * vec4(inPosition, 1.0);

    // Transform world position into clip space
    gl_Position = globalUBO.view_proj * world_pos;

    // Pass world position into Fragment shader
    outPos = world_pos.xyz;

    // Pass clip space positions for motion vectors
    outCSPos     = gl_Position;
    outPrevCSPos = globalUBO.prev_view_proj * prev_world_pos;

    // Pass texture coordinate
    outUV = inUV;

    // Transform vertex normal into world space
    mat3 normal_mat = mat3(meshUBO.model);

    outNormal    = normal_mat * inNormal;
    outTangent   = normal_mat * inTangent;
    outBitangent = normal_mat * inBitangent;
}