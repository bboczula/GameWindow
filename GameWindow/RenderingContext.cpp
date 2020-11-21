#include "RenderingContext.h"

#define LOG_FUNC_NAME std::cout << __func__ << "()" << std::endl

#define RELEASE(comObject) if(comObject) { comObject->Release(); comObject = nullptr; }

RenderingContext::RenderingContext(Astral::WindowContext windowContext) : currentFrameIndex(0)
{
	LOG_FUNC_NAME;

	CreateDxgiFactory();
	EnumerateAdapters();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain(windowContext);
	CreateRtvDescriptorHeap();
	CreateRenderTargetViews();
	CreateCommandAllocator();
	CreateCommandList();
}

RenderingContext::~RenderingContext()
{
	LOG_FUNC_NAME;

	RELEASE(commandList);
	RELEASE(commandAllocator);

	for (int i = 0; i < FRAME_COUNT; i++)
	{
		RELEASE(renderTargets[i]);
	}

	RELEASE(rtvDescriptorHeap);
	RELEASE(swapChain);
	RELEASE(graphicsCommandQueue);
	RELEASE(device);
	RELEASE(adapter);
	RELEASE(factory);
}

void RenderingContext::OnRender()
{
	RecordCommandList();
	ExecuteCommandList();
	PresentFrame();
	WaitForThePreviousFrame();
}

void RenderingContext::CreateDxgiFactory()
{
	LOG_FUNC_NAME;
	ThrowIfFailed(CreateDXGIFactory2(0, IID_PPV_ARGS(&factory)));
}

void RenderingContext::EnumerateAdapters()
{
	LOG_FUNC_NAME;

	// Here we assume that DXGI Factory creation was successful
	Microsoft::WRL::ComPtr<IDXGIAdapter1> currentAdapter;
	UINT index = 0;
	while (1)
	{
		HRESULT result = factory->EnumAdapters1(index, &currentAdapter);
		if (result == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		ThrowIfFailed(result);

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

void RenderingContext::CreateDevice()
{
	LOG_FUNC_NAME;

	ThrowIfFailed(D3D12CreateDevice(adapter.Get(), D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(device.GetAddressOf())));
}

void RenderingContext::CreateCommandQueue()
{
	LOG_FUNC_NAME;

	D3D12_COMMAND_QUEUE_DESC commandQueueDesc;
	commandQueueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;
	commandQueueDesc.Priority = D3D12_COMMAND_QUEUE_PRIORITY_NORMAL;
	commandQueueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
	commandQueueDesc.NodeMask = 0;

	ThrowIfFailed(device->CreateCommandQueue(&commandQueueDesc, IID_PPV_ARGS(graphicsCommandQueue.GetAddressOf())));
}

void RenderingContext::CreateSwapChain(const Astral::WindowContext& windowContext)
{
	LOG_FUNC_NAME;

	DXGI_MODE_DESC dxgiModeDesc;
	dxgiModeDesc.Width = windowContext.width;
	dxgiModeDesc.Height = windowContext.height;
	dxgiModeDesc.RefreshRate = DXGI_RATIONAL({ 0,0 });
	dxgiModeDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
	dxgiModeDesc.ScanlineOrdering = DXGI_MODE_SCANLINE_ORDER_UNSPECIFIED;
	dxgiModeDesc.Scaling = DXGI_MODE_SCALING_UNSPECIFIED;

	DXGI_SAMPLE_DESC dxgiSampleDesc;
	dxgiSampleDesc.Count = 1;
	dxgiSampleDesc.Quality = 0;

	DXGI_SWAP_CHAIN_DESC swapChainDesc;
	swapChainDesc.BufferDesc = dxgiModeDesc;
	swapChainDesc.SampleDesc = dxgiSampleDesc;
	swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
	swapChainDesc.BufferCount = FRAME_COUNT;
	swapChainDesc.OutputWindow = windowContext.hwnd;
	swapChainDesc.Windowed = TRUE;
	swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_SEQUENTIAL;
	swapChainDesc.Flags = 0;

	Microsoft::WRL::ComPtr<IDXGISwapChain> tempSwapChain;
	ThrowIfFailed(factory->CreateSwapChain(graphicsCommandQueue.Get(), &swapChainDesc, tempSwapChain.GetAddressOf()));

	// We need to "upcast" this to SwapChain3
	tempSwapChain.As(&swapChain);
}

void RenderingContext::CreateRtvDescriptorHeap()
{
	LOG_FUNC_NAME;
	
	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = FRAME_COUNT;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvDescriptorHeap)));
}

void RenderingContext::CreateRenderTargetViews()
{
	LOG_FUNC_NAME;

	// Descriptor handle is the address of the descriptor. Used as a last parameter
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	// We will have two separate descriptors on the same CPU descriptor heap, on the first and second position
	for (int i = 0; i < FRAME_COUNT; i++)
	{
		ThrowIfFailed(swapChain->GetBuffer(i, IID_PPV_ARGS(&renderTargets[i])));

		device->CreateRenderTargetView(renderTargets[i].Get(), nullptr, rtvDescriptorHandle);
		rtvDescriptorHandle.ptr += SIZE_T(descriptorHandleIncrementSize);
	}
}

void RenderingContext::CreateCommandAllocator()
{
	LOG_FUNC_NAME;

	ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void RenderingContext::CreateCommandList()
{
	LOG_FUNC_NAME;

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), nullptr, IID_PPV_ARGS(&commandList)));
	commandList->Close();
}

void RenderingContext::RecordCommandList()
{
	ThrowIfFailed(commandAllocator->Reset());

	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), nullptr));

	// Clear screen command
	// Depending on the frame, we should return the proper descriptor for RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
	if (currentFrameIndex == 1)
	{
		clearColor[0] = 0.1f;
		clearColor[1] = 0.3f;
		rtvDescriptorHandle.ptr += SIZE_T(descriptorHandleIncrementSize);
	}
	
	// Record the Clear Screen command on the Command List
	commandList->ClearRenderTargetView(rtvDescriptorHandle, clearColor, 0, nullptr);

	// Close the command list
	ThrowIfFailed(commandList->Close());
}

void RenderingContext::ExecuteCommandList()
{
	ID3D12CommandList* commandListArray[] = { commandList.Get() };
	graphicsCommandQueue->ExecuteCommandLists(_countof(commandListArray), commandListArray);
}

void RenderingContext::PresentFrame()
{
	ThrowIfFailed(swapChain->Present(1, 0));
}

void RenderingContext::WaitForThePreviousFrame()
{
	// Sleep for 16 miliseconds (roughly 60 FPS)
	Sleep(16);

	// Get the current Back Buffer index
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
}
