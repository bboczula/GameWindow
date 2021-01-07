#include "RenderingContext.h"

#define LOG_FUNC_NAME std::cout << __func__ << "()" << std::endl

#define RELEASE(comObject) if(comObject) { comObject->Release(); comObject = nullptr; }

RenderingContext::RenderingContext(Astral::WindowContext windowContext) : currentFrameIndex(0), mainCamera((float)windowContext.width / (float)windowContext.height)
{
	LOG_FUNC_NAME;

	CreateDxgiFactory();
	EnumerateAdapters();
	EnumerateOutputs();
	CreateDevice();
	CreateCommandQueue();
	CreateSwapChain(windowContext);

	// Create Descriptor Heaps
	CreateRtvDescriptorHeap();
	CreateDsvDescriptorHeap();
	CreateCbvDescriptorHeap();

	CreateRenderTargetViews();
	CreateDepthStencilBuffer(windowContext);
	CreateCommandAllocator();
	CreateCommandList();
	CreateEmptyRootSignature();
	CompileShaders();
	CreatePipelineState();
	CreateVertexBuffer();
	CreateIndexBuffer();
	CreatePerObjectConstantBuffer();
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

void RenderingContext::OnRender(float deltaTime)
{
	UpdateRotation();
	ResetCommandList();
	RecordCommandList(deltaTime);
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

#if 0
	Microsoft::WRL::ComPtr<ID3D12Debug> spDebugController0;
	Microsoft::WRL::ComPtr<ID3D12Debug1> spDebugController1;
	ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&spDebugController0)));
	ThrowIfFailed(spDebugController0->QueryInterface(IID_PPV_ARGS(&spDebugController1)));
	spDebugController1->EnableDebugLayer();
	spDebugController1->SetEnableGPUBasedValidation(true);
#endif

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

void RenderingContext::CreateDsvDescriptorHeap()
{
	LOG_FUNC_NAME;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_DSV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;

	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&dsvDescriptorHeap)));
}

void RenderingContext::CreateCbvDescriptorHeap()
{
	LOG_FUNC_NAME;

	D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
	// I think we only need one descriptor since we have one object
	rtvHeapDesc.NumDescriptors = 1;
	rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;
	rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE; // THis is important for CBV

	ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&cbvDescriptorHeap)));
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

void RenderingContext::CreateDepthStencilBuffer(const Astral::WindowContext& windowContext)
{
	LOG_FUNC_NAME;

	// We need to actually create a new resource, a texture of the same dimensions as the Render Target
	// We will create a commited resource, as usual, because this is the easiest.
	// This time we actually need a 2D texture

	D3D12_HEAP_PROPERTIES heapProperties;
	ZeroMemory(&heapProperties, sizeof(heapProperties));
	heapProperties.Type = D3D12_HEAP_TYPE_DEFAULT; // For some reason, this is important, the UPLOAD doesn't work, crashes the system
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;

	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_TEXTURE2D;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = windowContext.width;
	resourceDesc.Height = windowContext.height;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_D32_FLOAT;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_UNKNOWN;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_ALLOW_DEPTH_STENCIL;

	D3D12_CLEAR_VALUE depthOptimizedClearValue = {};
	depthOptimizedClearValue.Format = DXGI_FORMAT_D32_FLOAT;
	depthOptimizedClearValue.DepthStencil.Depth = 1.0f;
	depthOptimizedClearValue.DepthStencil.Stencil = 0;

	
	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_DEPTH_WRITE, &depthOptimizedClearValue, IID_PPV_ARGS(&depthBuffer)));
	ThrowIfFailed(depthBuffer->SetName(L"DepthBuffer"));

	D3D12_DEPTH_STENCIL_VIEW_DESC dsViewDesk;
	ZeroMemory(&dsViewDesk, sizeof(D3D12_DEPTH_STENCIL_VIEW_DESC));
	dsViewDesk.Format = DXGI_FORMAT_D32_FLOAT;
	dsViewDesk.ViewDimension = D3D12_DSV_DIMENSION_TEXTURE2D;
	dsViewDesk.Flags = D3D12_DSV_FLAG_NONE;
	dsViewDesk.Texture2D.MipSlice = 0;

	D3D12_CPU_DESCRIPTOR_HANDLE heapHandleDsv = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();

	device->CreateDepthStencilView(depthBuffer.Get(), &dsViewDesk, heapHandleDsv);
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

	// create a descriptor range (descriptor table) and fill it out
// this is a range of descriptors inside a descriptor heap
	D3D12_DESCRIPTOR_RANGE  descriptorTableRanges[1]; // only one range right now
	descriptorTableRanges[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV; // this is a range of constant buffer views (descriptors)
	descriptorTableRanges[0].NumDescriptors = 1; // we only have one constant buffer, so the range is only 1
	// This below is super-important for some reason
	descriptorTableRanges[0].BaseShaderRegister = 1; // start index of the shader registers in the range
	descriptorTableRanges[0].RegisterSpace = 0; // space 0. can usually be zero
	descriptorTableRanges[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND; // this appends the range to the end of the root signature descriptor tables

	// create a descriptor table
	D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
	descriptorTable.NumDescriptorRanges = _countof(descriptorTableRanges); // we only have one range
	descriptorTable.pDescriptorRanges = &descriptorTableRanges[0]; // the pointer to the beginning of our ranges array

	D3D12_ROOT_PARAMETER rootParameters[2];
	rootParameters[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_32BIT_CONSTANTS;
	rootParameters[0].Constants.Num32BitValues = 16;
	rootParameters[0].Constants.RegisterSpace = 0;
	rootParameters[0].Constants.ShaderRegister = 0;
	rootParameters[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	rootParameters[1].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
	rootParameters[1].DescriptorTable = descriptorTable;
	rootParameters[1].ShaderVisibility = D3D12_SHADER_VISIBILITY_ALL;

	D3D12_ROOT_SIGNATURE_DESC myRootSignatureDesc;
	myRootSignatureDesc.NumParameters = ARRAYSIZE(rootParameters);
	myRootSignatureDesc.pParameters = rootParameters;
	myRootSignatureDesc.NumStaticSamplers = 0;
	myRootSignatureDesc.pStaticSamplers = nullptr;
	myRootSignatureDesc.Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT;

	Microsoft::WRL::ComPtr<ID3DBlob> signature;
	Microsoft::WRL::ComPtr<ID3DBlob> error;
	ThrowIfFailed(D3D12SerializeRootSignature(&myRootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1_0, &signature, &error));
	ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void RenderingContext::CompileShaders()
{
	vertexShaderObject.setEntryPoint("VSMain");
	vertexShaderObject.setShaderModel("vs_5_1");
	vertexShaderObject.compileShaderFromFile(L"C:\\Users\\bboczula\\source\\repos\\GameWindow\\GameWindow\\shaders.hlsl");

	pixelShaderObject.setEntryPoint("PSMain");
	pixelShaderObject.setShaderModel("ps_5_1");
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
	rasterizerDesc.FillMode = D3D12_FILL_MODE_SOLID;
	rasterizerDesc.CullMode = D3D12_CULL_MODE_NONE;
	rasterizerDesc.FrontCounterClockwise = FALSE;
	rasterizerDesc.DepthBias = D3D12_DEFAULT_DEPTH_BIAS;
	rasterizerDesc.DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP;
	rasterizerDesc.SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS;
	rasterizerDesc.DepthClipEnable = TRUE;
	rasterizerDesc.MultisampleEnable = FALSE;
	rasterizerDesc.AntialiasedLineEnable = FALSE;
	rasterizerDesc.ForcedSampleCount = 0;
	rasterizerDesc.ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF;

	D3D12_BLEND_DESC blendDesc;
	ZeroMemory(&blendDesc, sizeof(blendDesc));
	blendDesc.AlphaToCoverageEnable = FALSE;
	blendDesc.IndependentBlendEnable = FALSE;
	blendDesc.RenderTarget[0].BlendEnable = FALSE;
	blendDesc.RenderTarget[0].LogicOpEnable = FALSE;
	blendDesc.RenderTarget[0].SrcBlend = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlend = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOp = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].SrcBlendAlpha = D3D12_BLEND_ONE;
	blendDesc.RenderTarget[0].DestBlendAlpha = D3D12_BLEND_ZERO;
	blendDesc.RenderTarget[0].BlendOpAlpha = D3D12_BLEND_OP_ADD;
	blendDesc.RenderTarget[0].LogicOp = D3D12_LOGIC_OP_NOOP;
	blendDesc.RenderTarget[0].RenderTargetWriteMask = D3D12_COLOR_WRITE_ENABLE_ALL;

	D3D12_DEPTH_STENCIL_DESC depthStencilDesc;
	ZeroMemory(&depthStencilDesc, sizeof(depthStencilDesc));
	depthStencilDesc.DepthEnable = TRUE;
	depthStencilDesc.DepthWriteMask = D3D12_DEPTH_WRITE_MASK_ALL;
	depthStencilDesc.DepthFunc = D3D12_COMPARISON_FUNC_LESS_EQUAL;
	depthStencilDesc.StencilEnable = FALSE;

	// Describe and create the graphics pipeline state object (PSO).
	D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc;
	ZeroMemory(&psoDesc, sizeof(psoDesc));
	psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
	psoDesc.pRootSignature = rootSignature.Get();
	psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShaderObject.GetBufferPointer()), vertexShaderObject.GetBufferSize() };
	psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShaderObject.GetBufferPointer()), pixelShaderObject.GetBufferSize() };
	psoDesc.RasterizerState = rasterizerDesc;
	psoDesc.BlendState = blendDesc;
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

	// Define the geometry for a cube
	Vertex cubeVertices[] =
	{
		{ { -0.500000,  0.500000,  0.500000 }, { 0.522f, 1.000f, 0.780f, 1.000f } },
		{ { -0.500000, -0.500000,  0.500000 }, { 0.161f, 0.451f, 0.451f, 1.000f } },
		{ { -0.500000,  0.500000, -0.500000 }, { 1.000f, 0.522f, 0.322f, 1.000f } },
		{ { -0.500000, -0.500000, -0.500000 }, { 0.314f, 0.188f, 0.278f, 1.000f } },
		{ {  0.500000,  0.500000,  0.500000 }, { 0.522f, 1.000f, 0.780f, 1.000f } },
		{ {  0.500000, -0.500000,  0.500000 }, { 0.161f, 0.451f, 0.451f, 1.000f } },
		{ {  0.500000,  0.500000, -0.500000 }, { 1.000f, 0.522f, 0.322f, 1.000f } },
		{ {  0.500000, -0.500000, -0.500000 }, { 0.314f, 0.188f, 0.278f, 1.000f } },
	};

	// const UINT vertexBufferSize = sizeof(triangleVertices);
	const UINT vertexBufferSize = sizeof(cubeVertices);

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
	ThrowIfFailed(vertexBuffer->SetName(L"VertexBuffer"));


	// Copy the triangle data to the vertex buffer.
	UINT8* vertexBufferPointers;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&vertexBufferPointers)));
	// memcpy(vertexBufferPointers, triangleVertices, sizeof(triangleVertices));
	memcpy(vertexBufferPointers, cubeVertices, sizeof(cubeVertices));
	vertexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
	vertexBufferView.StrideInBytes = sizeof(Vertex);
	vertexBufferView.SizeInBytes = vertexBufferSize;
}

void RenderingContext::CreateIndexBuffer()
{
	LOG_FUNC_NAME;

	DWORD indices[] =
	{
		4, 2, 0,
		2, 7, 3,
		6, 5, 7,
		1, 7, 5,
		0, 3, 1,
		4, 1, 5,
		4, 6, 2,
		2, 6, 7,
		6, 4, 5,
		1, 3, 7,
		0, 2, 3,
		4, 0, 1
	};

	const UINT indexBufferSize = sizeof(indices);

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
	resourceDesc.Width = indexBufferSize;
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&indexBuffer)));
	ThrowIfFailed(indexBuffer->SetName(L"IndexBuffer"));

	// Copy the triangle data to the vertex buffer.
	UINT8* indexBufferPointers;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(indexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&indexBufferPointers)));
	memcpy(indexBufferPointers, indices, sizeof(indices));
	indexBuffer->Unmap(0, nullptr);

	// Initialize the vertex buffer view.
	indexBufferView.BufferLocation = indexBuffer->GetGPUVirtualAddress();
	indexBufferView.SizeInBytes = 12 * 3 * 32;
	indexBufferView.Format = DXGI_FORMAT_R32_UINT;
}

void RenderingContext::CreatePerObjectConstantBuffer()
{
	LOG_FUNC_NAME;

	// So what we need is the Position, Rotation and Scale matrix. We can't push it through the Root Signature,
	// because it is very limited in size and we would need that for every game object we want to render.

	struct PerObjectData
	{
		DirectX::XMMATRIX worldMatrix;
	};

	PerObjectData perObjectData;

	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians(0.0f));
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 2.0f, 1.0f);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	perObjectData.worldMatrix = translation * rotation * scale;

	// Heap Description
	D3D12_HEAP_PROPERTIES heapProperties;
	ZeroMemory(&heapProperties, sizeof(heapProperties));
	heapProperties.Type = D3D12_HEAP_TYPE_UPLOAD;
	heapProperties.CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN;
	heapProperties.MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN;
	
	// Resource Description
	D3D12_RESOURCE_DESC resourceDesc;
	ZeroMemory(&resourceDesc, sizeof(resourceDesc));
	resourceDesc.Dimension = D3D12_RESOURCE_DIMENSION_BUFFER;
	resourceDesc.Alignment = 0;
	resourceDesc.Width = (sizeof(PerObjectData) + 255) & ~255;    // CB size is required to be 256-byte aligned.
	resourceDesc.Height = 1;
	resourceDesc.DepthOrArraySize = 1;
	resourceDesc.MipLevels = 1;
	resourceDesc.Format = DXGI_FORMAT_UNKNOWN;
	resourceDesc.SampleDesc.Count = 1;
	resourceDesc.SampleDesc.Quality = 0;
	resourceDesc.Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR;
	resourceDesc.Flags = D3D12_RESOURCE_FLAG_NONE;

	ThrowIfFailed(device->CreateCommittedResource(&heapProperties, D3D12_HEAP_FLAG_NONE, &resourceDesc,
		D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&perObjectBuffer)));
	ThrowIfFailed(perObjectBuffer->SetName(L"PerObjectBuffer"));

	// Copy the data
	// Copy the triangle data to the vertex buffer.
	UINT8* constantBufferPointers;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(perObjectBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBufferPointers)));
	memcpy(constantBufferPointers, &perObjectData, sizeof(PerObjectData));
	perObjectBuffer->Unmap(0, nullptr);

	// Create Constant Buffer View
	D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
	cbvDesc.BufferLocation = perObjectBuffer->GetGPUVirtualAddress();
	cbvDesc.SizeInBytes = (sizeof(PerObjectData) + 255) & ~255;    // CB size is required to be 256-byte aligned.
	device->CreateConstantBufferView(&cbvDesc, cbvDescriptorHeap->GetCPUDescriptorHandleForHeapStart());
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

void RenderingContext::UpdateRotation()
{
	angle += 1;
	if (angle > 360)
	{
		angle = 0;
	}

	struct PerObjectData
	{
		DirectX::XMMATRIX worldMatrix;
	};

	PerObjectData perObjectData;

	DirectX::XMMATRIX rotation = DirectX::XMMatrixRotationY(DirectX::XMConvertToRadians((float)angle));
	DirectX::XMMATRIX scale = DirectX::XMMatrixScaling(1.0f, 2.0f, 1.0f);
	DirectX::XMMATRIX translation = DirectX::XMMatrixTranslation(0.0f, 0.0f, 0.0f);
	perObjectData.worldMatrix = translation * rotation * scale;

	// Copy the data
	// Copy the triangle data to the vertex buffer.
	UINT8* constantBufferPointers;
	CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
	ThrowIfFailed(perObjectBuffer->Map(0, &readRange, reinterpret_cast<void**>(&constantBufferPointers)));
	memcpy(constantBufferPointers, &perObjectData, sizeof(PerObjectData));
	perObjectBuffer->Unmap(0, nullptr);
}

void RenderingContext::RecordCommandList(float deltaTime)
{
	// Pipeline setup commands
	commandList->SetGraphicsRootSignature(rootSignature.Get());
	commandList->SetPipelineState(pipelineState.Get());
	commandList->RSSetViewports(1, &viewport);
	commandList->RSSetScissorRects(1, &scissorRectangle);

	// Setup and set Resource Barrier
	//D3D12_RESOURCE_BARRIER resourceBarrier;
	//ZeroMemory(&resourceBarrier, sizeof(resourceBarrier));
	//resourceBarrier.Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION;
	//resourceBarrier.Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE;
	//resourceBarrier.Transition.pResource = renderTargets[currentFrameIndex].Get();
	//resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT;
	//resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//resourceBarrier.Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES;
	//commandList->ResourceBarrier(1, &resourceBarrier);
	
	// Set Render Target and Depth Buffer
	D3D12_CPU_DESCRIPTOR_HANDLE rtvDescriptorHandle = rtvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	UINT descriptorHandleIncrementSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
	D3D12_CPU_DESCRIPTOR_HANDLE dsvDescriptorHandle = dsvDescriptorHeap->GetCPUDescriptorHandleForHeapStart();
	if (currentFrameIndex == 1)
	{
		rtvDescriptorHandle.ptr += SIZE_T(descriptorHandleIncrementSize);
	}
	commandList->OMSetRenderTargets(1, &rtvDescriptorHandle, FALSE, &dsvDescriptorHandle);

	// Handle Constant Buffer
	// This works, but can't close the command list
	ID3D12DescriptorHeap* descriptorHeaps[] = { cbvDescriptorHeap.Get() };
	commandList->SetDescriptorHeaps(_countof(descriptorHeaps), descriptorHeaps);
	// The first parameter below is also super important
	commandList->SetGraphicsRootDescriptorTable(1, cbvDescriptorHeap->GetGPUDescriptorHandleForHeapStart());

	// Record the Clear Screen command on the Command List and Clear Depth Buffer
	float clearColor[] = { 0.224f, 0.224f, 0.227f, 1.0f };
	commandList->ClearRenderTargetView(rtvDescriptorHandle, clearColor, 0, nullptr);
	commandList->ClearDepthStencilView(dsvDescriptorHandle, D3D12_CLEAR_FLAG_DEPTH, 1.0f, 0, 0, nullptr);

	// Update the 32-bit constants
	//std::cout << deltaTime << " ";
	cumTime += deltaTime;
	float temp[1];
	temp[0] = cumTime;
	//commandList->SetGraphicsRoot32BitConstants(0, 1, &temp, 0);

	// Set Constant Buffer for the camera
	DirectX::XMFLOAT4X4 tempMatrix = mainCamera.getDxViewProjectionMatrix();
	//DirectX::XMMATRIX tempMatrix = DirectX::XMMatrixIdentity();
	//tempMatrix = DirectX::XMMatrixTranspose(tempMatrix);
	//tempMatrix._11 = 11.0f;
	//tempMatrix._12 = 12.0f;
	//tempMatrix._13 = 13.0f;

	//std::cout << "view" << std::endl;
	//std::cout << "( " << tempMatrix._11 << " " << tempMatrix._12 << " " << tempMatrix._13 << " " << tempMatrix._14 << " )" << std::endl;
	//std::cout << "( " << tempMatrix._21 << " " << tempMatrix._22 << " " << tempMatrix._23 << " " << tempMatrix._24 << " )" << std::endl;
	//std::cout << "( " << tempMatrix._31 << " " << tempMatrix._32 << " " << tempMatrix._33 << " " << tempMatrix._34 << " )" << std::endl;
	//std::cout << "( " << tempMatrix._41 << " " << tempMatrix._42 << " " << tempMatrix._43 << " " << tempMatrix._44 << " )" << std::endl;

	commandList->SetGraphicsRoot32BitConstants(0, 16, &tempMatrix, 0);


	// Draw Triangle
	commandList->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
	commandList->IASetVertexBuffers(0, 1, &vertexBufferView);
	
	// For triangle
	// commandList->DrawInstanced(3, 1, 0, 0);

	// For cube
	commandList->IASetIndexBuffer(&indexBufferView);
	commandList->DrawIndexedInstanced(12 * 3, 1, 0, 0, 0);

	// Set Resource Barrier back
	//resourceBarrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
	//resourceBarrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;
	//commandList->ResourceBarrier(1, &resourceBarrier);
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
	// This sync interval... actually works! I'm not even sure how, since I didn't set refresh rate.
	ThrowIfFailed(swapChain->Present(2, 0));
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
