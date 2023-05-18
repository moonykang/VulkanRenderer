#version 450

#extension GL_GOOGLE_include_directive : require
#include "common.glsl"
#include "brdf.glsl"
#include "lighting.glsl"

layout (location = 0) in vec3 inPos;

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


out gl_PerVertex 
{
    vec4 gl_Position;   
};

 
void main()
{
	gl_Position =  globalUBO.light.transform * vec4(inPos, 1.0);
}