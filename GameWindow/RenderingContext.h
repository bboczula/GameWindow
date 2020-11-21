#pragma once

#include <d3d12.h>
#include <dxgi1_4.h>
#include <iostream>
#include <wrl.h>
#include <windows.h>

#include "WindowContext.h"
#include "Utils.h"
#include "WindowContext.h"

#pragma comment(lib, "dxgi.lib")

#define FRAME_COUNT 2

class RenderingContext
{
public:
	RenderingContext(Astral::WindowContext windowContext);
	~RenderingContext();
	void OnRender();
private:
	void CreateDxgiFactory();
	void EnumerateAdapters();
	void CreateDevice();
	void CreateCommandQueue();
	void CreateSwapChain(const Astral::WindowContext& windowContext);
	void CreateRtvDescriptorHeap();
	void CreateRenderTargetViews();
	void CreateCommandAllocator();
	void CreateCommandList();
	void RecordCommandList();
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
	Microsoft::WRL::ComPtr<ID3D12CommandAllocator> rootSignature;
	Microsoft::WRL::ComPtr<ID3D12GraphicsCommandList> commandList;
};