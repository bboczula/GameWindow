#pragma once

#include <dxgi1_4.h>
#include <iostream>
#include <wrl.h>

#pragma comment(lib, "dxgi.lib")

class DxgiManager
{
public:
	DxgiManager();
	~DxgiManager();
private:
	void CreateDxgiFactory();
	void EnumerateAdapters();
	Microsoft::WRL::ComPtr<IDXGIFactory4> factory;
	Microsoft::WRL::ComPtr<IDXGIAdapter1> adapter;
};