# GPU Mipmap Generation 実装

## 概要
テクスチャのミップマップをGPUのCompute Shaderで生成する機能を実装。
CPU生成と比較して起動時間を大幅に短縮。

## 問題と解決

### 1. テクスチャエイリアシング（モアレパターン）
**問題**: 遠距離からテクスチャを見るとノイズ/モアレが発生
**解決**: ミップマップ + 異方性フィルタリング

### 2. sRGBフォーマットとUAVの非互換性
**問題**: `DXGI_FORMAT_R8G8B8A8_UNORM_SRGB`はUAV（Unordered Access View）をサポートしない
**解決**: 
- リソースは`DXGI_FORMAT_R8G8B8A8_UNORM`で作成
- SRV作成時に`DXGI_FORMAT_R8G8B8A8_UNORM_SRGB`として解釈（`GraphicsDevice::CreateSRV`で変換）

### 3. 複数テクスチャのディスクリプタ衝突
**問題**: 同一コマンドリストで複数テクスチャをロードすると、ディスクリプタヒープの同じオフセットが上書きされ、先にロードしたテクスチャのミップマップが破損（遠距離で真っ黒）
**解決**: 各テクスチャにユニークなディスクリプタベースオフセットを割り当て

```cpp
// MipmapGenerator.h
static constexpr uint32 MAX_TEXTURES_PER_BATCH = 128;
uint32 m_currentTextureOffset = 0;

// MipmapGenerator.cpp - GenerateMips()
uint32 textureBaseOffset = m_currentTextureOffset * DESCRIPTORS_PER_MIP * MAX_MIP_LEVELS;
m_currentTextureOffset = (m_currentTextureOffset + 1) % MAX_TEXTURES_PER_BATCH;
// ...
uint32 descOffset = textureBaseOffset + srcMip * DESCRIPTORS_PER_MIP;
```

## ファイル構成

### Shaders/GenerateMips.hlsl
```hlsl
SamplerState BilinearClamp : register(s0);
Texture2D<float4> SrcMip   : register(t0);
RWTexture2D<float4> OutMip : register(u0);

cbuffer MipConstants : register(b0)
{
    float2 InvOutTexelSize;
    uint SrcMipIndex;
    uint Padding;
}

[numthreads(8, 8, 1)]
void main(uint3 DTid : SV_DispatchThreadID)
{
    float2 uv = (DTid.xy + 0.5) * InvOutTexelSize;
    OutMip[DTid.xy] = SrcMip.SampleLevel(BilinearClamp, uv, SrcMipIndex);
}
```

### Engine/Graphics/MipmapGenerator.h
- `Initialize()`: ルートシグネチャ、PSO、ディスクリプタヒープ作成
- `GenerateMips()`: 指定テクスチャのミップチェーン生成

### Engine/Graphics/MipmapGenerator.cpp
**ルートシグネチャ構成**:
- Root Parameter 0: 定数 (InvOutTexelSize, SrcMipIndex)
- Root Parameter 1: SRV Descriptor Table
- Root Parameter 2: UAV Descriptor Table
- Static Sampler: Bilinear + Clamp

**リソース状態遷移フロー**:
1. Mip 0: COPY_DEST → NON_PIXEL_SHADER_RESOURCE
2. 各ミップ生成:
   - Dst Mip: 現在状態 → UNORDERED_ACCESS
   - Dispatch
   - UAV Barrier
   - Dst Mip: UNORDERED_ACCESS → NON_PIXEL_SHADER_RESOURCE
3. 全Mip: → PIXEL_SHADER_RESOURCE

## サンプラー設定 (Pipeline.cpp, SkinnedPipeline.cpp)
```cpp
sampler.Filter = D3D12_FILTER_ANISOTROPIC;
sampler.MipLODBias = -0.5f;  // シャープさ維持
sampler.MaxAnisotropy = 16;
```

## 使用方法
```cpp
// Texture2D::LoadFromFile() 内で自動呼び出し
if (mipLevels > 1) {
    graphics->GetMipmapGenerator()->GenerateMips(
        commandList, resource_.Get(), D3D12_RESOURCE_STATE_COPY_DEST
    );
}
```

## 注意点
- `MAX_TEXTURES_PER_BATCH`（128）を超えるテクスチャを同一コマンドリストでロードするとオフセットがラップする
- 各モデル読み込みごとにコマンドリストをフラッシュしているため、通常は問題なし
