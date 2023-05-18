#version 450

layout(set = 0, binding = 1) uniform sampler2D texSampler;

layout(location = 0) in vec2 inUV;
layout(location = 0) out vec4 outColor;

void main() {
    outColor = vec4(texture(texSampler, inUV).rrr, 1.0f);
}
