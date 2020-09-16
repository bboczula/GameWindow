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
	void createSwapChain(ID3D12CommandQueue* commandQueue, UINT frameCount, UINT width, UINT height, HWND hwnd);
	void present();
	UINT getCurrentBackBufferIndex();
	IDXGIAdapter1* getPrimaryAdapter();
	IDXGISwapChain3* getSwapChain();
private:
	unsigned int numOfAdapters;
	IDXGIFactory4* dxgiFactory;
	IDXGIAdapter1* hardwareAdapters[MAX_NUM_OF_ADAPTERS];
	IDXGISwapChain3* swapChain;
};