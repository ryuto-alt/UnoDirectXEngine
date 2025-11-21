#pragma once

#include "D3D12Common.h"
#include "Shader.h"

namespace UnoEngine {

/// Compute Pipelineの管理クラス
class ComputePipeline {
public:
    ComputePipeline() = default;
    ~ComputePipeline() = default;

    /// 初期化
    void Initialize(ID3D12Device* device, const Shader& computeShader);

    /// アクセサ
    ID3D12RootSignature* GetRootSignature() const { return rootSignature_.Get(); }
    ID3D12PipelineState* GetPipelineState() const { return pipelineState_.Get(); }

private:
    void CreateRootSignature(ID3D12Device* device);
    void CreatePipelineState(ID3D12Device* device, const Shader& computeShader);

private:
    ComPtr<ID3D12RootSignature> rootSignature_;
    ComPtr<ID3D12PipelineState> pipelineState_;
};

} // namespace UnoEngine
