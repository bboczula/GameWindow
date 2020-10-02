#include "GpuAbstract.h"

GpuAbstract::GpuAbstract(GpuAbstractType type, UINT frameCount) : type(type), frameCount(frameCount)
{
}

GpuAbstract::~GpuAbstract()
{
}

void GpuAbstract::createDevice(IDXGIAdapter1* adapter)
{
	std::cout << " GpuAbstract::createDevice()" << std::endl;

	ThrowIfFailed(D3D12CreateDevice(adapter, D3D_FEATURE_LEVEL_11_0, IID_PPV_ARGS(&device)));
}

void GpuAbstract::createCommandQueue()
{
    std::cout << " GpuAbstract::createCommandQueue()" << std::endl;

    D3D12_COMMAND_QUEUE_DESC queueDesc = {};
    queueDesc.Flags = D3D12_COMMAND_QUEUE_FLAG_NONE;
    queueDesc.Type = D3D12_COMMAND_LIST_TYPE_DIRECT;

    ThrowIfFailed(device->CreateCommandQueue(&queueDesc, IID_PPV_ARGS(&commandQueue)));
}

void GpuAbstract::createDescriptorHeaps()
{
    std::cout << " GpuAbstract::createDescriptorHeaps()" << std::endl;

    D3D12_DESCRIPTOR_HEAP_DESC rtvHeapDesc = {};
    rtvHeapDesc.NumDescriptors = frameCount;
    rtvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV;
    rtvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE;
    ThrowIfFailed(device->CreateDescriptorHeap(&rtvHeapDesc, IID_PPV_ARGS(&rtvHeap)));

    rtvDescriptorSize = device->GetDescriptorHandleIncrementSize(D3D12_DESCRIPTOR_HEAP_TYPE_RTV);
    std::cout << " - rtvDescriptorSize: " << rtvDescriptorSize << std::endl;

    for (int i = 0; i < frameCount; i++)
    {
        D3D12_DESCRIPTOR_HEAP_DESC cbvHeapDesc = {};
        cbvHeapDesc.NumDescriptors = 1;
        cbvHeapDesc.Flags = D3D12_DESCRIPTOR_HEAP_FLAG_SHADER_VISIBLE;
        cbvHeapDesc.Type = D3D12_DESCRIPTOR_HEAP_TYPE_CBV_SRV_UAV;

        ThrowIfFailed(device->CreateDescriptorHeap(&cbvHeapDesc, IID_PPV_ARGS(&cbvHeap)));
    }
}

void GpuAbstract::createFrameResrouces(DxgiManager* dxgiManager)
{
    std::cout << " GpuAbstract::createFrameResrouces()" << std::endl;

    // Create Resrouce Heap for our Constant Buffer
    for (UINT n = 0; n < frameCount; n++)
    {
        ThrowIfFailed(device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
            &CD3DX12_RESOURCE_DESC::Buffer(1024 * 64), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&cbBufferUploadHeap[n])));

        D3D12_CONSTANT_BUFFER_VIEW_DESC cbvDesc = {};
        cbvDesc.BufferLocation = cbBufferUploadHeap[n]->GetGPUVirtualAddress();
        cbvDesc.SizeInBytes = (sizeof(ConstantBuffer) + 255) & ~255;
        device->CreateConstantBufferView(&cbvDesc, cbvHeap->GetCPUDescriptorHandleForHeapStart());

        ZeroMemory(&cbColorMultiplierData, sizeof(cbColorMultiplierData));
        CD3DX12_RANGE readRange(0, 0);
        ThrowIfFailed(cbBufferUploadHeap[n]->Map(0, &readRange, reinterpret_cast<void**>(&cbColorMultiplierGPUAddress[n])));
        memcpy(cbColorMultiplierGPUAddress, &cbColorMultiplierData, sizeof(cbColorMultiplierData));
    }

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());

    if (type == PRIMARY_GPU)
    {
        std::cout << "  - creating resources for primary GPU" << std::endl;

        for (UINT n = 0; n < frameCount; n++)
        {
            ThrowIfFailed(dxgiManager->getSwapChain()->GetBuffer(n, IID_PPV_ARGS(&renderTargets[n])));
            device->CreateRenderTargetView(renderTargets[n], nullptr, rtvHandle);
            rtvHandle.Offset(1, rtvDescriptorSize);
        }
    }
    else
    {
        std::cout << "  - creating resources for secondary GPU" << std::endl;

        const CD3DX12_RESOURCE_DESC renderTargetDesc = CD3DX12_RESOURCE_DESC::Tex2D(DXGI_FORMAT_R8G8B8A8_UNORM, dxgiManager->getRenderTargetWidth(),
            dxgiManager->getRenderTargetHeight(), 1u, 1u, 1, 0, D3D12_RESOURCE_FLAG_ALLOW_RENDER_TARGET, D3D12_TEXTURE_LAYOUT_UNKNOWN, 0u);

        D3D12_CLEAR_VALUE clearValue{ DXGI_FORMAT_R8G8B8A8_UNORM, { 1.0f, 0.0f, 0.0f, 1.0f } };

        CD3DX12_CPU_DESCRIPTOR_HANDLE rtvSecondaryHandle(rtvHeap->GetCPUDescriptorHandleForHeapStart());
        for (UINT n = 0; n < frameCount; n++)
        {
            device->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_DEFAULT), D3D12_HEAP_FLAG_NONE, &renderTargetDesc,
                D3D12_RESOURCE_STATE_COMMON, nullptr, IID_PPV_ARGS(&renderTargets[n]));
            device->CreateRenderTargetView(renderTargets[n], nullptr, rtvSecondaryHandle);
            rtvSecondaryHandle.Offset(1, rtvDescriptorSize);
        }
    }  
}

void GpuAbstract::createCommandAllocator()
{
    std::cout << " GpuAbstract::createCommandAllocator()" << std::endl;

    ThrowIfFailed(device->CreateCommandAllocator(D3D12_COMMAND_LIST_TYPE_DIRECT, IID_PPV_ARGS(&commandAllocator)));
}

void GpuAbstract::createEmptyRootSignature()
{
    std::cout << " GpuAbstract::createEmptyRootSignature()" << std::endl;

    // CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDesc;
    // rootSignatureDesc.Init(0, nullptr, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    D3D12_DESCRIPTOR_RANGE descriptorRange[1];
    ZeroMemory(&descriptorRange, sizeof(D3D12_DESCRIPTOR_RANGE));
    descriptorRange[0].RangeType = D3D12_DESCRIPTOR_RANGE_TYPE_CBV;
    descriptorRange[0].NumDescriptors = 1;
    descriptorRange[0].BaseShaderRegister = 0;
    descriptorRange[0].RegisterSpace = 0;
    descriptorRange[0].OffsetInDescriptorsFromTableStart = D3D12_DESCRIPTOR_RANGE_OFFSET_APPEND;

    D3D12_ROOT_DESCRIPTOR_TABLE descriptorTable;
    ZeroMemory(&descriptorTable, sizeof(D3D12_ROOT_DESCRIPTOR_TABLE));
    descriptorTable.NumDescriptorRanges = _countof(descriptorRange);
    descriptorTable.pDescriptorRanges = &descriptorRange[0];

    D3D12_ROOT_PARAMETER rootParameter[1];
    rootParameter[0].ParameterType = D3D12_ROOT_PARAMETER_TYPE_DESCRIPTOR_TABLE;
    rootParameter[0].DescriptorTable = descriptorTable;
    rootParameter[0].ShaderVisibility = D3D12_SHADER_VISIBILITY_VERTEX;

    // RootSignature is the paramter list of the pipeline.
    CD3DX12_ROOT_SIGNATURE_DESC rootSignatureDescription;
    rootSignatureDescription.Init(_countof(rootParameter), rootParameter, 0, nullptr, D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT);

    ID3DBlob* signature;
    ID3DBlob* error;
    ThrowIfFailed(D3D12SerializeRootSignature(&rootSignatureDescription, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
    ThrowIfFailed(device->CreateRootSignature(0, signature->GetBufferPointer(), signature->GetBufferSize(), IID_PPV_ARGS(&rootSignature)));
}

void GpuAbstract::createPipelineState(D3D12_INPUT_ELEMENT_DESC* inputLayout, UINT numOfInputLayoutElements, ID3DBlob* vertexShader, ID3DBlob* pixelShader)
{
    // Describe and create the graphics pipeline state object (PSO).
    D3D12_GRAPHICS_PIPELINE_STATE_DESC psoDesc = {};
    psoDesc.InputLayout = { inputLayout, numOfInputLayoutElements };
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
    ThrowIfFailed(device->CreateGraphicsPipelineState(&psoDesc, IID_PPV_ARGS(&pipelineState)));
}

void GpuAbstract::createCommandList(ID3D12PipelineState* pipelineState)
{
    std::cout << " GpuAbstract::createCommandList()" << std::endl;

    ThrowIfFailed(device->CreateCommandList(0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, pipelineState, IID_PPV_ARGS(&commandList)));
    ThrowIfFailed(commandList->Close());
    if (type == PRIMARY_GPU)
    {
        ThrowIfFailed(commandList->SetName(L"PrimaryCommandList"));
    }
    else
    {
        ThrowIfFailed(commandList->SetName(L"SecondaryCommandList"));
    }
}

void GpuAbstract::createSyncObjects()
{
    std::cout << " GpuAbstract::createSyncObjects()" << std::endl;

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
    waitForPreviousFrame();
}

void GpuAbstract::executeCommandList()
{
    ID3D12CommandList* ppCommandLists[] = { commandList };
    commandQueue->ExecuteCommandLists(_countof(ppCommandLists), ppCommandLists);
}

void GpuAbstract::resetCommandAllocator()
{
    // Command list allocators can only be reset when the associated 
    // command lists have finished execution on the GPU; apps should use 
    // fences to determine GPU execution progress.
    ThrowIfFailed(commandAllocator->Reset());
}

void GpuAbstract::resetCommandList()
{
    // However, when ExecuteCommandList() is called on a particular command 
    // list, that command list can then be reset at any time and must be before 
    // re-recording.
    ThrowIfFailed(commandList->Reset(commandAllocator, pipelineState));
}

void GpuAbstract::closeCommandList()
{
    ThrowIfFailed(commandList->Close());
}

void GpuAbstract::waitForPreviousFrame()
{
    // WAITING FOR THE FRAME TO COMPLETE BEFORE CONTINUING IS NOT BEST PRACTICE.
   // This is code implemented as such for simplicity. More advanced samples 
   // illustrate how to use fences for efficient resource usage.

   // Signal and increment the fence value.
    const UINT64 fenceTemp = fenceValue;
    ThrowIfFailed(commandQueue->Signal(fence, fenceTemp));
    fenceValue++;

    // Wait until the previous frame is finished.
    if (fence->GetCompletedValue() < fenceTemp)
    {
        ThrowIfFailed(fence->SetEventOnCompletion(fenceTemp, fenceEvent));
        WaitForSingleObject(fenceEvent, INFINITE);
    }
}

void GpuAbstract::setRootSignature()
{
    commandList->SetGraphicsRootSignature(rootSignature);
}

ID3D12Device* GpuAbstract::getDevice()
{
    return device;
}

ID3D12CommandQueue* GpuAbstract::getCommandQueue()
{
    return commandQueue;
}

ID3D12RootSignature* GpuAbstract::getRootSignature()
{
    return rootSignature;
}

ID3D12CommandAllocator* GpuAbstract::getCommandAllocator()
{
    return commandAllocator;
}

ID3D12Resource* GpuAbstract::getRenderTarget(UINT index)
{
    return renderTargets[index];
}

ID3D12DescriptorHeap* GpuAbstract::getRtvHeap()
{
    return rtvHeap;
}

UINT GpuAbstract::getDescriptorSize()
{
    return rtvDescriptorSize;
}

ID3D12GraphicsCommandList* GpuAbstract::getCommandList()
{
    return commandList;
}

ID3D12PipelineState* GpuAbstract::getPipelineState()
{
    return pipelineState;
}
