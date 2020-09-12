#include "Application.h"

inline void ThrowIfFailed(HRESULT hr)
{
    if (FAILED(hr))
    {
        // Set a breakpoint on this line to catch DirectX API errors
        throw std::exception();
    }
}

Application::Application() : BaseWindow("GamePerformanceStudies", 1280, 720)
{
    std::cout << "Application::Application()" << std::endl;
}

Application::~Application()
{
    std::cout << "Application::~Application()" << std::endl;
}

void Application::initialize()
{
    std::cout << " Application::initialize()" << std::endl;

    createDxgiFactory();
    enumerateAdapters();
    createDevice();
    createCommandQueue();
    createSwapChain();
    createDescriptorHeaps();
    createFrameResources();
    createCommandAllocator();
    createEmptyRootSignature();
    createPipelineState();
    createCommandList();
    createVertexBuffer();
}

void Application::tick()
{
}

void Application::createDxgiFactory()
{
    std::cout << " Application::createDxgiFactory()" << std::endl;

    UINT flags = 0;
#ifdef DEBUG
    flags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // DEBUG
    ThrowIfFailed(CreateDXGIFactory2(flags, IID_PPV_ARGS(&dxgiFactory)));
}

void Application::enumerateAdapters()
{
    std::cout << " Application::enumerateAdapters()" << std::endl;

    IDXGIAdapter1* hardwareAdapter;
    int i = 0;
    while ((DXGI_ERROR_NOT_FOUND != dxgiFactory->EnumAdapters1(i, &hardwareAdapter)))
    {
        DXGI_ADAPTER_DESC1 adapterDescriptor;
        hardwareAdapter->GetDesc1(&adapterDescriptor);

        // This basically means "if this is a hardware adapter"
        if (((adapterDescriptor.Flags & DXGI_ADAPTER_FLAG_SOFTWARE) == 0) && (adapterDescriptor.VendorId != 5140))
        {
            std::wcout << "  " << i << ". " << adapterDescriptor.Description << std::endl;
            hardwareAdapters.push_back(hardwareAdapter);
    }

        i++;

        // hardwareAdapter->Release();
}
}

void Application::createDevice()
{
    std::cout << " Application::createDevice()" << std::endl;

    ThrowIfFailed(D3D12CreateDevice(hardwareAdapters[0], D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&primaryDevice)));
}

void Application::createCommandQueue()
{
    std::cout << " Application::createCommandQueue()" << std::endl;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(primaryDevice->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));

}

void Application::createSwapChain()
{
    std::cout << " Application::createSwapChain()" << std::endl;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = FRAME_COUNT;
    swapChainDesc.BufferDesc.Width = window.getWidth();
    swapChainDesc.BufferDesc.Height = window.getHeight();
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = window.getHwnd();
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    IDXGISwapChain* swapChainTemp;
    ThrowIfFailed(dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChainTemp));

    // Upgrade to SwapChain3
    ThrowIfFailed(swapChainTemp->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&swapChain));

    // This sample does not support fullscreen transitions.
    ThrowIfFailed(dxgiFactory->MakeWindowAssociation(window.getHwnd(), DXGI_MWA_NO_ALT_ENTER));

    frameIndex = swapChain->GetCurrentBackBufferIndex();
}

void Application::createDescriptorHeaps()
{
    std::cout << " Application::createDescriptorHeaps()" << std::endl;

    // Describe and create a render target view (RTV) descriptor heap.
    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = FRAME_COUNT;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(primaryDevice->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = primaryDevice->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
}

void Application::createFrameResources()
{
    std::cout << " Application::createFrameResources()" << std::endl;

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

    // Create a RTV for each frame.
    for (UINT n = 0; n < FRAME_COUNT; n++)
    {
        ThrowIfFailed(swapChain->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
        primaryDevice->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
        rtvHandle.Offset(1, rtvDescriptorSize);
    }
}

void Application::createCommandAllocator()
{
    std::cout << " Application::createCommandAllocator()" << std::endl;

    ThrowIfFailed(primaryDevice->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void Application::createEmptyRootSignature()
{
    std::cout << " Application::createEmptyRootSignature()" << std::endl;

    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* signature;
    ID3DBlob* error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDesc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(primaryDevice->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void Application::createPipelineState()
{
    std::cout << " Application::createPipelineState()" << std::endl;

    ID3DBlob* vertexShader;
    ID3DBlob* pixelShader;

#if defined(_DEBUG)
    // Enable better shader debugging with the graphics debugging tools.
    UINT compileFlags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
    UINT compileFlags = 0;
#endif

    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "VSMain", "vs_5_0", compileFlags, 0, &vertexShader, nullptr));
    ThrowIfFailed(D3DCompileFromFile(L"shaders.hlsl", nullptr, nullptr, "PSMain", "ps_5_0", compileFlags, 0, &pixelShader, nullptr));

    // Define the vertex input layout.
    D3D12_INPUT_ELEMENT_DESC inputElementDescs[] =
    {
        { "POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT, 0, 0, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 },
        { "COLOR", 0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, 12, D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0 }
    };

    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputElementDescs, _countof(inputElementDescs) };
    psoDesc.pRootSignature = rootSignature;
    psoDesc.VS = { reinterpret_cast<UINT8*>(vertexShader->GetBufferPointer()), vertexShader->GetBufferSize() };
    psoDesc.PS = { reinterpret_cast<UINT8*>(pixelShader->GetBufferPointer()), pixelShader->GetBufferSize() };
    psoDesc.RasterizerState = CD3DX12_RASTERIZER_DESC(D3D12_DEFAULT);
    psoDesc.BlendState = CD3DX12_BLEND_DESC(D3D12_DEFAULT);
    psoDesc.DepthStencilState.DepthEnable = FALSE;
    psoDesc.DepthStencilState.StencilEnable = FALSE;
    psoDesc.SampleMask = UINT_MAX;
    psoDesc.PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE;
    psoDesc.NumRenderTargets = 1;
    psoDesc.RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM;
    psoDesc.SampleDesc.Count = 1;
    ThrowIfFailed(primaryDevice->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}

void Application::createCommandList()
{
    std::cout << " Application::createCommandList()" << std::endl;

    ThrowIfFailed(primaryDevice->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, pipelineState, IID_PPV_ARGS(&commandList)));
    ThrowIfFailed(commandList->Close());
}

void Application::createVertexBuffer()
{
    std::cout << " Application::createVertexBuffer()" << std::endl;

    float m_aspectRatio = static_cast<float>(window.getWidth()) / static_cast<float>(window.getHeight());
    // Define the geometry for a triangle.
    Vertex triangleVertices[] =
    {
        { { 0.0f, 0.25f * m_aspectRatio, 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
        { { 0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
        { { -0.25f, -0.25f * m_aspectRatio, 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
    };

    const UINT vertexBufferSize = sizeof(triangleVertices);

    // Note: using upload heaps to transfer static data like vert buffers is not 
    // recommended. Every time the GPU needs it, the upload heap will be marshalled 
    // over. Please read up on Default Heap usage. An upload heap is used here for 
    // code simplicity and because there are very few verts to actually transfer.
    ThrowIfFailed(primaryDevice->CreateCommittedResource(
        &CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD),
        D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize),
        D3D12_RESOURCE_STATE_GENERIC_READ,
        nullptr,
        IID_PPV_ARGS(&vertexBuffer)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pVertexDataBegin;
    CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(vertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pVertexDataBegin)));
    memcpy(pVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    vertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    vertexBufferView.BufferLocation = vertexBuffer->GetGPUVirtualAddress();
    vertexBufferView.StrideInBytes = sizeof(Vertex);
    vertexBufferView.SizeInBytes = vertexBufferSize;
}

