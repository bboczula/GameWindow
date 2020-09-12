#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>

#include "d3dx12.h"
#include "BaseWindow.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")
#pragma comment(lib, "D3DCompiler.lib")

#define FRAME_COUNT 2

class Application : public BaseWindow
{
public:
    Application();
    ~Application();
    void initialize() override;
    void tick() override;
private:
    ID3D12Device* primaryDevice;
    IDXGIFactory4* dxgiFactory;
    ID3D12CommandQueue* commandQueue;
    std::vector<IDXGIAdapter1*> hardwareAdapters;
    IDXGISwapChain3* swapChain;
    UINT frameIndex;
    ID3D12DescriptorHeap* rtvHeap;
    UINT rtvDescriptorSize;
    ID3D12Resource* renderTargets[FRAME_COUNT];
    ID3D12CommandAllocator* commandAllocator;
    ID3D12RootSignature* rootSignature;
    ID3D12PipelineState* pipelineState;
    ID3D12GraphicsCommandList* commandList;
    void createDxgiFactory();
    void enumerateAdapters();
    void createDevice();
    void createCommandQueue();
    void createSwapChain();
    void createDescriptorHeaps();
    void createFrameResources();
    void createCommandAllocator();
    void createEmptyRootSignature();
    void createPipelineState();
    void createCommandList();
};