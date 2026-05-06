#version 450

layout (location = 0) in vec3 inPos;
layout (location = 1) in vec2 inTexCoord;
layout (location = 2) in vec3 inNormal;

layout (binding = 0) uniform UBO
{
   mat4 projectionMatrix;
   mat4 modelMatrix;
   mat4 viewMatrix;
} ubo;

layout (location = 0) out vec2 outTexCoord;
layout (location = 1) out vec3 outNormal;

out gl_PerVertex
{
    vec4 gl_Position;
};

void main()
{
   outTexCoord = inTexCoord;
   outNormal = mat3(ubo.modelMatrix) * inNormal;
   gl_Position = ubo.projectionMatrix * ubo.viewMatrix * ubo.modelMatrix * vec4(inPos, 1.0);
}
