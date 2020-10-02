#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <DirectXMath.h>
#include <iostream>

#include "Utils.h"
#include "d3dx12.h"
#include "DxgiManager.h"

enum GpuAbstractType
{
	PRIMARY_GPU,
	SECONDARY_GPU
};

// this is the structure of our constant buffer.
struct ConstantBuffer {
	DirectX::XMFLOAT4 colorMultiplier;
};

class GpuAbstract
{
public:
	GpuAbstract(GpuAbstractType type, UINT frameCount);
	~GpuAbstract();
	void createDevice(IDXGIAdapter1* adapter);
	void createCommandQueue();
	void createDescriptorHeaps();
	void createFrameResrouces(DxgiManager* dxgiManager);
	void createCommandAllocator();
	void createEmptyRootSignature();
	void createPipelineState(D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT numOfInputLayoutElements, ID3DBlob* vertexShader, ID3DBlob* pixelShader);
	void createCommandList(ID3D12PipelineState* pipelineState);
	void createSyncObjects();
	void executeCommandList();
	void resetCommandAllocator();
	void resetCommandList();
	void closeCommandList();
	void waitForPreviousFrame();
	void setRootSignature();
	ID3D12Device* getDevice();
	ID3D12CommandQueue* getCommandQueue();
	ID3D12RootSignature* getRootSignature();
	ID3D12CommandAllocator* getCommandAllocator();
	ID3D12Resource* getRenderTarget(UINT index);
	ID3D12DescriptorHeap* getRtvHeap();
	UINT getDescriptorSize();
	ID3D12GraphicsCommandList* getCommandList();
	ID3D12PipelineState* getPipelineState();
private:
	ConstantBuffer cbColorMultiplierData;
	UINT8* cbColorMultiplierGPUAddress[2];
	GpuAbstractType type;
	ID3D12Device* device;
	ID3D12CommandQueue* commandQueue;
	ID3D12DescriptorHeap* rtvHeap;
	ID3D12DescriptorHeap* cbvHeap;
	UINT rtvDescriptorSize;
	UINT frameCount;
	ID3D12Resource* renderTargets[2];
	ID3D12Resource* cbBufferUploadHeap[2];
	ID3D12CommandAllocator* commandAllocator;
	ID3D12RootSignature* rootSignature;
	ID3D12GraphicsCommandList* commandList;
	ID3D12PipelineState* pipelineState;
	ID3D12Fence* fence;
	UINT64 fenceValue;
	HANDLE fenceEvent;
};