#version 450

layout (location = 0) in vec2 inTexCoord;

layout (binding = 1) uniform texture2D texColor;
layout (binding = 2) uniform sampler texSampler;

layout (location = 0) out vec4 outFragColor;

void main()
{
	outFragColor = texture(sampler2D(texColor, texSampler), inTexCoord);
}
