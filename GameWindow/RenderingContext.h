#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <wrl.h>
#include <windows.h>
#include <vector>
#include <DirectXMath.h>

#include "WindowContext.h"
#include "Utils.h"
#include "Shader.h"
#include "d3dx12.h"

#pragma comment(lib, "dxgi.lib")

#define FRAME_COUNT 2

struct Vertex
{
	DirectX::XMFLOAT3 position;
	DirectX::XMFLOAT4 color;
};

class RenderingContext
{
public:
	RenderingContext(Astral::WindowContext windowContext);
	~RenderingContext();
	void OnRender();
private:
	void CreateDxgiFactory();
	void EnumerateAdapters();
	void EnumerateOutputs();
	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(const Astral::WindowContext& windowContext);
	void CreateRtvDescriptorHeap();
	void CreateRenderTargetViews();
	void CreateCommandAllocator();
	void CreateCommandList();
	void CreateEmptyRootSignature();
	void CreatePipelineState();
	void CreateVertexBuffer();
	void RecordCommandList();
	void ResetCommandList();
	void ExecuteCommandList();
	void PresentFrame();
	void WaitForThePreviousFrame();
private:
	UINT currentFrameIndex;
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
	Microsoft::WRL::ComPtr<ID3D12Device> device;
	Microsoft::WRL::ComPtr<ID3D12CommandQueue> graphicsCommandQueue;
	Microsoft::WRL::ComPtr<IDXGISwapChain3> swapChain;	// Needed "3" to get current buffer index
	Microsoft::WRL::ComPtr<ID3D12DescriptorHeap> rtvDescriptorHeap;
	Microsoft::WRL::ComPtr<ID3D12Resource> renderTargets[FRAME_COUNT];
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> commandAllocator;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
	Microsoft::WRL::ComPtr<ID3D12RootSignature> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	Microsoft::WRL::ComPtr< ID3D12Resource> vertexBuffer;
};