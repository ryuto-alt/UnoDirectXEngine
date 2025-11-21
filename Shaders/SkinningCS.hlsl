// GPU Skinning Compute Shader
// 頂点スキニングをGPU上で実行し、結果をByteAddressBuffer (UAV)に書き込む

// ジョイント行列バッファ (SRV)
StructuredBuffer<float4x4> g_JointMatrices : register(t0);

// 入力頂点バッファ (SRV)
struct VertexSkinned
{
    float3 position;
    float3 normal;
    float2 texCoord;
    uint4 joints;
    float4 weights;
};

StructuredBuffer<VertexSkinned> g_InputVertices : register(t1);

// 出力頂点バッファ (UAV)
struct VertexOutput
{
    float3 position;
    float3 normal;
    float2 texCoord;
};

RWByteAddressBuffer g_OutputVertices : register(u0);

// スレッドグループサイズ
#define THREAD_GROUP_SIZE 64

[numthreads(THREAD_GROUP_SIZE, 1, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    uint vertexIndex = DTid.x;

    // 入力頂点データを取得
    VertexSkinned inputVertex = g_InputVertices[vertexIndex];

    // スキニング計算: 4つのジョイントの影響を合成
    float3 skinnedPosition = float3(0.0f, 0.0f, 0.0f);
    float3 skinnedNormal = float3(0.0f, 0.0f, 0.0f);

    for (uint i = 0; i < 4; ++i)
    {
        uint jointIndex = inputVertex.joints[i];
        float weight = inputVertex.weights[i];

        if (weight > 0.0f)
        {
            float4x4 jointMatrix = g_JointMatrices[jointIndex];

            // 位置変換 (同次座標)
            float4 pos = float4(inputVertex.position, 1.0f);
            skinnedPosition += mul(pos, jointMatrix).xyz * weight;

            // 法線変換 (方向ベクトル)
            float4 norm = float4(inputVertex.normal, 0.0f);
            skinnedNormal += mul(norm, jointMatrix).xyz * weight;
        }
    }

    // 法線を正規化
    skinnedNormal = normalize(skinnedNormal);

    // 出力バッファに書き込み (ByteAddressBufferなので手動でオフセット計算)
    uint outputOffset = vertexIndex * 32; // sizeof(VertexOutput) = 3*4 + 3*4 + 2*4 = 32 bytes

    // position (12 bytes)
    g_OutputVertices.Store3(outputOffset, asuint(skinnedPosition));

    // normal (12 bytes)
    g_OutputVertices.Store3(outputOffset + 12, asuint(skinnedNormal));

    // texCoord (8 bytes)
    g_OutputVertices.Store2(outputOffset + 24, asuint(inputVertex.texCoord));
}
