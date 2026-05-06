#version 450

layout (location = 0) in vec2 inTexCoord;
layout (location = 1) in vec3 inNormal;

layout (binding = 1) uniform texture2D texColor;
layout (binding = 2) uniform sampler texSampler;

layout (location = 0) out vec4 outFragColor;

void main()
{
   vec3 albedo = texture(sampler2D(texColor, texSampler), inTexCoord).rgb;
   vec3 normal = normalize(inNormal);
   vec3 lightDirection = normalize(vec3(-0.35, 0.75, 0.45));
   float diffuse = max(dot(normal, lightDirection), 0.0);
   vec3 color = albedo * (0.5 + diffuse * 0.5);
   outFragColor = vec4(color, 1.0);
}
