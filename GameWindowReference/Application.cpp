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
