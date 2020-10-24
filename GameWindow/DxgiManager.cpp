#include "DxgiManager.h"

DxgiManager::DxgiManager() : factory(nullptr)
{
	std::cout << "DxgiManager::DxgiManager" << std::endl;
	CreateDxgiFactory();
	EnumerateAdapters();
}

DxgiManager::~DxgiManager()
{
}

void DxgiManager::CreateDxgiFactory()
{
	std::cout << " DxgiManager::CreateDxgiFactory" << std::endl;
	HRESULT result = CreateDXGIFactory2(0, IID_PPV_ARGS(&factory));
	if (FAILED(result))
	{
		std::cout << "CreateDXGIFactory2 failed!" << std::endl;
		exit(1);
	}
}

void DxgiManager::EnumerateAdapters()
{
	std::cout << " DxgiManager::EnumerateAdapters" << std::endl;

	// Here we assume that DXGI Factory creation was successful
	Microsoft::WRL::ComPtr<IDXGIAdapter1> currentAdapter;
	UINT index = 0;
	while(1)
	{
		HRESULT result = factory->EnumAdapters1(index, &currentAdapter);
		if (result == DXGI_ERROR_INVALID_CALL)
		{
			std::cout << "  Invalid call to EnumAdapters" << std::endl;
			exit(1);
		}
		else if (result == DXGI_ERROR_NOT_FOUND)
		{
			std::cout << "  No more adapters" << std::endl;
			break;
		}
		else
		{
			
			DXGI_ADAPTER_DESC1 adapterDescriptor;
			currentAdapter->GetDesc1(&adapterDescriptor);
			std::wcout << "  " << index << ") " << adapterDescriptor.Description << " [" << adapterDescriptor.DedicatedVideoMemory << "]" << std::endl;

			if (index == 0)
			{
				adapter = currentAdapter.Detach();
			}

			index++;
		}
	}
}
