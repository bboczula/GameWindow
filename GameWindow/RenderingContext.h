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
	void OnRender(float deltaTime);
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
	void CompileShaders();
	void CreatePipelineState();
	void CreateVertexBuffer();
	void CreateSynchronizationObjects();
	void RecordCommandList(float deltaTime);
	void CloseCommandList();
	void ResetCommandList();
	void ExecuteCommandList();
	void PresentFrame();
	void WaitForThePreviousFrame();
	void CreateViewportAndScissorsRect(FLOAT width, FLOAT height);
private:
	UINT currentFrameIndex;
	HANDLE fenceEvent;
	Microsoft::WRL::ComPtr<ID3D12Fence> fence;
	UINT64 fenceValue;
	D3D12_VIEWPORT viewport;
	D3D12_RECT scissorRectangle;
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
	Microsoft::WRL::ComPtr<ID3DBlob> vertexShader;
	Microsoft::WRL::ComPtr<ID3DBlob> pixelShader;
	Microsoft::WRL::ComPtr<ID3D12PipelineState> pipelineState;
	D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
	Microsoft::WRL::ComPtr< ID3D12Resource> vertexBuffer;
	Shader vertexShaderObject;
	Shader pixelShaderObject;
	float cumTime;
};