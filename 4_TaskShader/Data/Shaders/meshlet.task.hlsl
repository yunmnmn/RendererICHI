struct SceneConstants
{
    float4x4 projectionMatrix;
    float4x4 modelMatrix;
    float4x4 viewMatrix;
    uint meshletCount;
    uint3 padding;
    float4 cameraWorldPositionScale; // xyz: camera world position, w: uniform model scale
    float4 frustumData;              // x: tan(fovY / 2), y: aspect, z: near, w: far
};

struct Meshlet
{
    uint4 data;              // x: vertex offset, y: vertex count, z: triangle offset, w: triangle count
    float4 centerRadius;
    float4 coneApexCutoff;
    float4 coneAxis;
};

struct MeshTaskPayload
{
    uint meshletIndices[32];
};

ConstantBuffer<SceneConstants> scene : register(b0, space0);
StructuredBuffer<Meshlet> meshlets : register(t1, space0);

static const uint MeshletsPerTask = 32u;

groupshared MeshTaskPayload taskPayload;
groupshared uint visibleMeshletCount;

bool IsSphereInsideViewFrustum(float3 viewCenter, float radius)
{
    const float tanHalfFovY = scene.frustumData.x;
    const float tanHalfFovX = tanHalfFovY * scene.frustumData.y;
    const float nearPlane = scene.frustumData.z;
    const float farPlane = scene.frustumData.w;

    const float3 leftPlane = normalize(float3(1.0f, 0.0f, -tanHalfFovX));
    const float3 rightPlane = normalize(float3(-1.0f, 0.0f, -tanHalfFovX));
    const float3 bottomPlane = normalize(float3(0.0f, 1.0f, -tanHalfFovY));
    const float3 topPlane = normalize(float3(0.0f, -1.0f, -tanHalfFovY));

    if (dot(leftPlane, viewCenter) < -radius)
        return false;
    if (dot(rightPlane, viewCenter) < -radius)
        return false;
    if (dot(bottomPlane, viewCenter) < -radius)
        return false;
    if (dot(topPlane, viewCenter) < -radius)
        return false;
    if (-viewCenter.z - nearPlane < -radius)
        return false;
    if (viewCenter.z + farPlane < -radius)
        return false;

    return true;
}

bool IsMeshletBackfacing(Meshlet meshlet)
{
    const float3 cameraWorldPosition = scene.cameraWorldPositionScale.xyz;
    const float3 coneApexWorld = mul(scene.modelMatrix, float4(meshlet.coneApexCutoff.xyz, 1.0f)).xyz;
    const float3 coneAxisWorldUnnormalized = mul((float3x3)scene.modelMatrix, meshlet.coneAxis.xyz);
    const float coneCutoff = meshlet.coneApexCutoff.w;
    const float3 apexToCameraUnnormalized = coneApexWorld - cameraWorldPosition;
    const float coneAxisLengthSq = dot(coneAxisWorldUnnormalized, coneAxisWorldUnnormalized);
    const float apexToCameraLengthSq = dot(apexToCameraUnnormalized, apexToCameraUnnormalized);

    if (coneCutoff > 1.0f || coneAxisLengthSq < 1e-8f || apexToCameraLengthSq < 1e-8f)
        return false;

    const float3 coneAxisWorld = coneAxisWorldUnnormalized * rsqrt(coneAxisLengthSq);
    const float3 apexToCamera = apexToCameraUnnormalized * rsqrt(apexToCameraLengthSq);
    return dot(apexToCamera, coneAxisWorld) >= coneCutoff;
}

bool IsMeshletVisible(Meshlet meshlet)
{
    const float radius = meshlet.centerRadius.w * scene.cameraWorldPositionScale.w;
    const float3 worldCenter = mul(scene.modelMatrix, float4(meshlet.centerRadius.xyz, 1.0f)).xyz;
    const float3 viewCenter = mul(scene.viewMatrix, float4(worldCenter, 1.0f)).xyz;

    return IsSphereInsideViewFrustum(viewCenter, radius) && !IsMeshletBackfacing(meshlet);
}

[numthreads(32, 1, 1)]
void main(uint3 groupId : SV_GroupID,
          uint groupThreadIndex : SV_GroupIndex)
{
    if (groupThreadIndex == 0u)
    {
        visibleMeshletCount = 0u;
    }

    GroupMemoryBarrierWithGroupSync();

    const uint taskBase = groupId.x * MeshletsPerTask;
    const uint meshletIndex = taskBase + groupThreadIndex;

    if (meshletIndex < scene.meshletCount)
    {
        Meshlet meshlet = meshlets[meshletIndex];
        if (IsMeshletVisible(meshlet))
        {
            uint payloadIndex;
            InterlockedAdd(visibleMeshletCount, 1u, payloadIndex);
            taskPayload.meshletIndices[payloadIndex] = meshletIndex;
        }
    }

    GroupMemoryBarrierWithGroupSync();
    DispatchMesh(visibleMeshletCount, 1u, 1u, taskPayload);
}
