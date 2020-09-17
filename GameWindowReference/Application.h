#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <DirectXMath.h>

#include "d3dx12.h"
#include "BaseWindow.h"
#include "DxgiManager.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define FRAME_COUNT 2

struct Vertex
{
    DirectX::XMFLOAT3 position;
    DirectX::XMFLOAT4 color;
};

class Application : public BaseWindow
{
public:
    Application();
    ~Application();
    void initialize() override;
    void tick() override;
private:
    DxgiManager dxgiManager;
    ID3D12Device* primaryDevice;
    ID3D12CommandQueue* commandQueue;
    //UINT frameIndex;
    ID3D12DescriptorHeap* rtvHeap;
    UINT rtvDescriptorSize;
    ID3D12Resource* renderTargets[FRAME_COUNT];
    ID3D12CommandAllocator* commandAllocator;
    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineState;
    ID3D12GraphicsCommandList* commandList;
    ID3D12Resource* vertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    HANDLE fenceEvent;
    ID3D12Fence* fence;
    UINT64 fenceValue;
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;
    void createDevice();
    void createCommandQueue();
    void createDescriptorHeaps();
    void createFrameResources();
    void createCommandAllocator();
    void createEmptyRootSignature();
    void createPipelineState();
    void createCommandList();
    void createVertexBuffer();
    void createSyncObjects();
    void waitForPreviousFrame();
    void populateCommandList();
};