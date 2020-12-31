#include "RenderingContext.h"

#define LOG_FUNC_NAME std::cout << __func__ << "()" << std::endl

#define RELEASE(comObject) if(comObject) { comObject->Release(); comObject = nullptr; }

RenderingContext::RenderingContext(Astral::WindowContext windowContext) : currentFrameIndex(0)
{
	LOG_FUNC_NAME;

	CreateDxgiFactory();
	EnumerateAdapters();
	EnumerateOutputs();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain(windowContext);
	CreateRtvDescriptorHeap();
	CreateRenderTargetViews();
	CreateCommandAllocator();
	CreateCommandList();
	CreateEmptyRootSignature();
	CompileShaders();
	CreatePipelineState();
	CreateVertexBuffer();
	CreateSynchronizationObjects();
	CreateViewportAndScissorsRect(windowContext.width, windowContext.height);
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
	ResetCommandList();
	RecordCommandList();
	CloseCommandList();
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

		// Print some info about the adapter
		DXGI_ADAPTER_DESC1 adapterDesc;
		currentAdapter->GetDesc1(&adapterDesc);
		std::wcout << adapterDesc.Description << " [" << adapterDesc.DedicatedVideoMemory << "]" << std::endl;

		if (index == 0)
		{
			adapter = currentAdapter.Detach();
		}

		index++;
	}
}

void RenderingContext::EnumerateOutputs()
{
	LOG_FUNC_NAME;

	UINT outputIndex = 0;
	IDXGIOutput* output;

	while (1)
	{
		HRESULT result = adapter->EnumOutputs(outputIndex, &output);
		if (result == DXGI_ERROR_NOT_FOUND)
		{
			break;
		}
		ThrowIfFailed(result);

		// Print some info about the output
		DXGI_OUTPUT_DESC outputDesc;
		output->GetDesc(&outputDesc);
		// std::wcout << outputDesc.DeviceName << " " << outputDesc.Monitor << std::endl;

		// First, let's fetch how many modes the output supports
		UINT numOfSupportedModes = 0;
		output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numOfSupportedModes, NULL);

		// Now lets get the list of all supported modes
		DXGI_MODE_DESC* supportedModes = new DXGI_MODE_DESC[numOfSupportedModes];
		output->GetDisplayModeList(DXGI_FORMAT_R8G8B8A8_UNORM, 0, &numOfSupportedModes, supportedModes);

		// Now we can iterate over all supported modes to eventually pick one
		for (int i = 0; i < numOfSupportedModes; i++)
		{
			// Whatever setting you pick, this is the input to the CreateSwapChain function
			// std::wcout << " " << i << ". " << supportedModes[i].Width << "x" << supportedModes[i].Height << " " << supportedModes[i].RefreshRate.Numerator << " " << supportedModes[i].RefreshRate.Denominator << std::endl;
		}

		// To avoid a memory leak, when you finish using the output interface, call the Release method to decrement the reference count.
		output->Release();

		// Continue with the next output
		outputIndex++;
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
	dxgiModeDesc.RefreshRate = DXGI_RATIONAL({ 0, 1 });
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

	ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator.Get(), pipelineState.Get(), IID_PPV_ARGS(&commandList)));
	commandList->Close();
}

void RenderingContext::CreateEmptyRootSignature()
{
	LOG_FUNC_NAME;

	D3D12_ROOT_SIGNATURE_DESC myRootSignatureDesc;
	myRootSignatureDesc.NumParameters = 0;
	myRootSignatureDesc.pParameters = nullptr;
	myRootSignatureDesc.NumStaticSamplers = 0;
	myRootSignatureDesc.pStaticSamplers = nullptr;
	myRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&myRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void RenderingContext::CompileShaders()
{
	vertexShaderObject.setEntryPoint("VSMain");
	vertexShaderObject.setShaderModel("vs_5_0");
	vertexShaderObject.compileShaderFromFile(L"C:\\Users\\bboczula\\source\\repos\\GameWindow\\GameWindow\\shaders.hlsl");

	pixelShaderObject.setEntryPoint("PSMain");
	pixelShaderObject.setShaderModel("ps_5_0");
	pixelShaderObject.compileShaderFromFile(L"C:\\Users\\bboczula\\source\\repos\\GameWindow\\GameWindow\\shaders.hlsl");
}

void RenderingContext::CreatePipelineState()
{
	LOG_FUNC_NAME;

	// Define the vertex input layout.
	D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
	{
		{ "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
		{ "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, D3D12_APPEND_ALIGNED_ELEMENT, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
	};

	D3D12_RASTERIZER_DESC rasterizerDesc;
	ZeroMemory(&rasterizerDesc, sizeof(rasterizerDesc));

	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(psoDesc));
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShaderObject.GetBufferPointer()), vertexShaderObject.GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShaderObject.GetBufferPointer()), pixelShaderObject.GetBufferSize() };
	psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
	psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);;
	psoDesc.DepthStencilState = depthStencilDesc;
	psoDesc.SampleMask = UINT_MAX;
	psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
	psoDesc.NumRenderTargets = 1;
	psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
	psoDesc.SampleDesc.Count = 1;
	psoDesc.SampleDesc.Quality = 0;

	ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}

void RenderingContext::CreateVertexBuffer()
{
	LOG_FUNC_NAME;

	// Define the geometry for a triangle.
	Vertex triangleVertices[] =
	{
		{ {  0.0f,  0.5f, 0.0f }, { 0.898f, 0.345f, 0.000f, 1.000f } },
		{ {  0.5f, -0.5f, 0.0f }, { 0.937f, 0.635f, 0.000f, 1.000f } },
		{ { -0.5f, -0.5f, 0.0f }, { 0.364f, 0.556f, 0.701f, 1.000f } }
	};

	const UINT vertexBufferSize = sizeof(triangleVertices);

	// Note: using upload heaps to transfer static data like vert buffers is not 
	// recommended. Every time the GPU needs it, the upload heap will be marshalled 
	// over. Please read up on Default Heap usage. An upload heap is used here for 
	// code simplicity and because there are very few verts to actually transfer.
	D3D12_HEAP_PROPERTIES heapProperties;
	ZeroMemory(&heapProperties, sizeof(heapProperties));
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = vertexBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)));

	// Copy the triangle data to the vertex buffer.
	UINT8* vertexBufferPointers;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexBufferPointers)));
	memcpy(vertexBufferPointers, triangleVertices, sizeof(triangleVertices));
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;
}

void RenderingContext::CreateSynchronizationObjects()
{
	LOG_FUNC_NAME;

	ThrowIfFailed(device->CreateFence(0, D3D12_FENCE_FLAG_NONE, IID_PPV_ARGS(&fence)));
	fenceValue = 1;

	// Create an event handle to use for frame synchronization.
	fenceEvent = CreateEvent(nullptr, FALSE, FALSE, nullptr);
	if (fenceEvent == nullptr)
	{
		ThrowIfFailed(HRESULT_FROM_WIN32(GetLastError()));
	}

	// Wait for the command list to execute; we are reusing the same command 
	// list in our main loop but for now, we just want to wait for setup to 
	// complete before continuing.
	WaitForThePreviousFrame();
}

void RenderingContext::RecordCommandList()
{
	// Clear screen command
	// Depending on the frame, we should return the proper descriptor for RTV
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRectangle);
	
	// Set Render Target
	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));
	if (currentFrameIndex == 1)
	{
		rtvDescriptorHandle.ptr += SIZE_T(descriptorHandleIncrementSize);
	}
	commandList->OMSetRenderTargets(1, &rtvDescriptorHandle, FALSE, nullptr);

	// Record the Clear Screen command on the Command List
	float clearColor[] = { 0.925f, 0.886f, 0.776f, 1.0f };
	commandList->ClearRenderTargetView(rtvDescriptorHandle, clearColor, 0, nullptr);

	// Draw Triangle
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	commandList->DrawInstanced(3, 1, 0, 0);

	commandList->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(renderTargets[currentFrameIndex].Get(), D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));
}

void RenderingContext::CloseCommandList()
{
	// Close the command list
	ThrowIfFailed(commandList->Close());
}

void RenderingContext::ResetCommandList()
{
	ThrowIfFailed(commandAllocator->Reset());
	ThrowIfFailed(commandList->Reset(commandAllocator.Get(), pipelineState.Get()));
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
	// WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
    // This is code implemented as such for simplicity. More advanced samples 
    // illustrate how to use fences for efficient resource usage.

    // Signal and increment the fence value.
    const UINT64 tempFence = fenceValue;
    ThrowIfFailed(graphicsCommandQueue->Signal(fence.Get(), tempFence));
    fenceValue++;

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < tempFence)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(tempFence, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }

	// Get the current Back Buffer index
	currentFrameIndex = swapChain->GetCurrentBackBufferIndex();
}

void RenderingContext::CreateViewportAndScissorsRect(FLOAT width, FLOAT height)
{
	LOG_FUNC_NAME;

	ZeroMemory(&viewport, sizeof(viewport));
	viewport.TopLeftX = 0;
	viewport.TopLeftY = 0;
	viewport.Width = width;
	viewport.Height = height;
	viewport.MinDepth = D3D12_MIN_DEPTH;
	viewport.MaxDepth = D3D12_MAX_DEPTH;

	ZeroMemory(&scissorRectangle, sizeof(scissorRectangle));
	scissorRectangle.left = 0;
	scissorRectangle.right = width;
	scissorRectangle.top = 0;
	scissorRectangle.bottom = height;
}
