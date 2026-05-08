struct SceneConstants
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
};

struct Meshlet
{
    uint4 data;              // x: vertex offset, y: vertex count, z: triangle offset, w: triangle count
    float4 centerRadius;
    float4 coneApexCutoff;
    float4 coneAxis;
};

struct MeshVertex
{
    float4 position : SV_Position;
    float3 color : COLOR0;
};

ConstantBuffer<SceneConstants> scene : register(b0, space0);
StructuredBuffer<Meshlet> meshlets : register(t1, space0);
StructuredBuffer<float4> positions : register(t2, space0);
StructuredBuffer<uint> vertexIndices : register(t3, space0);
StructuredBuffer<uint> triangleIndices : register(t4, space0);

float3 MakeColor(float3 position)
{
    float3 n = normalize(abs(position) + float3(0.001f, 0.001f, 0.001f));
    return saturate(float3(0.18f, 0.28f, 0.42f) + n * 0.72f);
}

[outputtopology("triangle")]
[numthreads(64, 1, 1)]
void main(uint3 groupId : SV_GroupID,
          uint groupThreadIndex : SV_GroupIndex,
          out vertices MeshVertex verts[64],
          out indices uint3 tris[124])
{
    Meshlet meshlet = meshlets[groupId.x];
    uint vertexCount = meshlet.data.y;
    uint triangleCount = meshlet.data.w;

    SetMeshOutputCounts(vertexCount, triangleCount);

    for (uint i = groupThreadIndex; i < vertexCount; i += 64)
    {
        uint vertexIndex = vertexIndices[meshlet.data.x + i];
        float4 localPosition = float4(positions[vertexIndex].xyz, 1.0f);
        float4 worldPosition = mul(scene.modelMatrix, localPosition);
        float4 viewPosition = mul(scene.viewMatrix, worldPosition);

        verts[i].position = mul(scene.projectionMatrix, viewPosition);
        verts[i].color = MakeColor(worldPosition.xyz);
    }

    for (uint i = groupThreadIndex; i < triangleCount; i += 64)
    {
        uint triBase = meshlet.data.z + i * 3u;
        tris[i] = uint3(triangleIndices[triBase + 0u],
                        triangleIndices[triBase + 1u],
                        triangleIndices[triBase + 2u]);
    }
}
