#pragma once

#include <dxgi1_4.h>
#include <d3d12.h>
#include <iostream>

#include "Utils.h"

#define MAX_NUM_OF_ADAPTERS 2

class DxgiManager
{
public:
	DxgiManager();
	~DxgiManager();
	void createDxgiFactory();
	void enumerateAdapters();
	void createPrimarySwapChain(ID3D12CommandQueue* commandQueue, UINT frameCount, UINT width, UINT height, HWND hwnd);
	void present();
	UINT getCurrentBackBufferIndex();
	UINT getRenderTargetWidth();
	UINT getRenderTargetHeight();
	IDXGIAdapter1* getPrimaryAdapter();
	IDXGIAdapter1* getSecondaryAdapter();
	IDXGISwapChain3* getSwapChain();
private:
	unsigned int numOfAdapters;
	IDXGIFactory4* dxgiFactory;
	IDXGIAdapter1* hardwareAdapters[MAX_NUM_OF_ADAPTERS];
	DXGI_ADAPTER_DESC1 hardwareAdaptersDescriptors[MAX_NUM_OF_ADAPTERS];
	IDXGISwapChain3* swapChain;
	IDXGISwapChain3* secondarySwapChain;
	UINT width;
	UINT height;
};