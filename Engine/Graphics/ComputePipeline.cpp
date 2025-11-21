#include "ComputePipeline.h"
#include "d3dx12.h"
#include <stdexcept>

namespace UnoEngine {

void ComputePipeline::Initialize(ID3D12Device* device, const Shader& computeShader) {
    CreateRootSignature(device);
    CreatePipelineState(device, computeShader);
}

void ComputePipeline::CreateRootSignature(ID3D12Device* device) {
    // Root Parameters:
    // [0] SRV StructuredBuffer<float4x4> g_JointMatrices : register(t0)
    // [1] SRV StructuredBuffer<VertexSkinned> g_InputVertices : register(t1)
    // [2] UAV RWByteAddressBuffer g_OutputVertices : register(u0)

    CD3DX12_DESCRIPTOR_RANGE1 srvRange0(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 0); // t0
    CD3DX12_DESCRIPTOR_RANGE1 srvRange1(D3D12_DESCRIPTOR_RANGE_TYPE_SRV, 1, 1); // t1
    CD3DX12_DESCRIPTOR_RANGE1 uavRange(D3D12_DESCRIPTOR_RANGE_TYPE_UAV, 1, 0);  // u0

    CD3DX12_ROOT_PARAMETER1 rootParameters[3];
    rootParameters[0].InitAsDescriptorTable(1, &srvRange0, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[1].InitAsDescriptorTable(1, &srvRange1, D3D12_SHADER_VISIBILITY_ALL);
    rootParameters[2].InitAsDescriptorTable(1, &uavRange, D3D12_SHADER_VISIBILITY_ALL);

    D3D12_VERSIONED_ROOT_SIGNATURE_DESC rootSignatureDesc = {};
    rootSignatureDesc.Version = D3D_ROOT_SIGNATURE_VERSION_1_1;
    rootSignatureDesc.Desc_1_1.NumParameters = _countof(rootParameters);
    rootSignatureDesc.Desc_1_1.pParameters = rootParameters;
    rootSignatureDesc.Desc_1_1.Flags = D3D12_ROOT_SIGNATURE_FLAG_NONE;

    ComPtr<ID3DBlob> signature;
    ComPtr<ID3DBlob> error;
    HRESULT hr = D3D12SerializeVersionedRootSignature(&rootSignatureDesc, &signature, &error);
    if (FAILED(hr)) {
        if (error) {
            OutputDebugStringA(static_cast<char*>(error->GetBufferPointer()));
        }
        throw std::runtime_error("Failed to serialize compute root signature");
    }

    hr = device->CreateRootSignature(
        0,
        signature->GetBufferPointer(),
        signature->GetBufferSize(),
        IID_PPV_ARGS(&rootSignature_)
    );

    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create compute root signature");
    }
}

void ComputePipeline::CreatePipelineState(ID3D12Device* device, const Shader& computeShader) {
    D3D12_COMPUTE_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.pRootSignature = rootSignature_.Get();
    psoDesc.CS = { computeShader.GetBytecode()->GetBufferPointer(), computeShader.GetBytecode()->GetBufferSize() };

    HRESULT hr = device->CreateComputePipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState_));
    if (FAILED(hr)) {
        throw std::runtime_error("Failed to create compute pipeline state");
    }
}

} // namespace UnoEngine
