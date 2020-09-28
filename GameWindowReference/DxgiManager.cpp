#include "DxgiManager.h"

DxgiManager::DxgiManager() : numOfAdapters(0)
{
}

DxgiManager::~DxgiManager()
{
}

void DxgiManager::createDxgiFactory()
{
    std::cout << " DxgiManager::createDxgiFactory()" << std::endl;

    UINT dxgiFactoryFlags = 0;
#ifdef _DEBUG
    ID3D12Debug* debugController;
    ThrowIfFailed(D3D12GetDebugInterface(IID_PPV_ARGS(&debugController)));
    debugController->EnableDebugLayer();
    dxgiFactoryFlags |= DXGI_CREATE_FACTORY_DEBUG;
#endif // DEBUG
    ThrowIfFailed(CreateDXGIFactory2(dxgiFactoryFlags, IID_PPV_ARGS(&dxgiFactory)));
}

void DxgiManager::enumerateAdapters()
{
    std::cout << " DxgiManager::enumerateAdapters()" << std::endl;

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
            hardwareAdapters[numOfAdapters] = hardwareAdapter;
            hardwareAdaptersDescriptors[numOfAdapters] = adapterDescriptor;
            numOfAdapters++;

            if (numOfAdapters == MAX_NUM_OF_ADAPTERS)
            {
                std::cout << " WARNING: Maximum number of adapters reached" << std::endl;
                return;
            }
        }

        i++;
    }
}

void DxgiManager::createPrimarySwapChain(ID3D12CommandQueue* commandQueue, UINT frameCount, UINT width, UINT height, HWND hwnd)
{
    std::cout << " DxgiManager::createPrimarySwapChain()" << std::endl;

    // Describe and create the swap chain.
    DXGI_SWAP_CHAIN_DESC swapChainDesc = {};
    swapChainDesc.BufferCount = frameCount;
    swapChainDesc.BufferDesc.Width = width;
    swapChainDesc.BufferDesc.Height = height;
    swapChainDesc.BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM;
    swapChainDesc.BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT;
    swapChainDesc.SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD;
    swapChainDesc.OutputWindow = hwnd;
    swapChainDesc.SampleDesc.Count = 1;
    swapChainDesc.Windowed = TRUE;

    IDXGISwapChain* swapChainTemp;
    ThrowIfFailed(dxgiFactory->CreateSwapChain(commandQueue, &swapChainDesc, &swapChainTemp));

    // Upgrade to SwapChain3
    ThrowIfFailed(swapChainTemp->QueryInterface(__uuidof(IDXGISwapChain3), (void**)&swapChain));

    // Store params
    this->width = width;
    this->height = height;
}

void DxgiManager::present()
{
    ThrowIfFailed(getSwapChain()->Present(1, 0));
}

UINT DxgiManager::getCurrentBackBufferIndex()
{
    return swapChain->GetCurrentBackBufferIndex();
}

UINT DxgiManager::getRenderTargetWidth()
{
    return width;
}

UINT DxgiManager::getRenderTargetHeight()
{
    return height;
}

IDXGIAdapter1* DxgiManager::getPrimaryAdapter()
{
    // Adapter[0] is the adapter that Presents frames to the display
    return hardwareAdapters[0];
}

IDXGIAdapter1* DxgiManager::getSecondaryAdapter()
{
    return hardwareAdapters[1];
}

IDXGISwapChain3* DxgiManager::getSwapChain()
{
    return swapChain;
}
