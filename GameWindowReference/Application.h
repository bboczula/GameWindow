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
#include "GpuAbstract.h"

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
    GpuAbstract primaryGpu;
    GpuAbstract secondaryGpu;
    ID3D12Resource* vertexBuffer;
    ID3D12Resource* secondaryVertexBuffer;
    D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
    D3D12_VERTEX_BUFFER_VIEW secondaryVertexBufferView;
    D3D12_VIEWPORT viewport;
    D3D12_RECT scissorRect;
    void createPipelineState();
    void createVertexBuffer();
    void populateCommandList();
};