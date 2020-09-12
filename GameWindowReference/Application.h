#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <vector>

#include "d3dx12.h"
#include "BaseWindow.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "dxgi.lib")

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
    void createDxgiFactory();
    void enumerateAdapters();
    void createDevice();
    void createCommandQueue();
    void createSwapChain();
    void createDescriptorHeaps();
    void createFrameResources();
    void createCommandAllocator();
    void createEmptyRootSignature();
};