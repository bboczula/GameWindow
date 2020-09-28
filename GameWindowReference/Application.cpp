#include "Application.h"

#define FLIP_GPU 0

#if FLIP_GPU
Application::Application() : BaseWindow("GamePerformanceStudies", 1280, 720), primaryGpu(GpuAbstractType::SECONDARY_GPU, FRAME_COUNT), secondaryGpu(GpuAbstractType::PRIMARY_GPU, FRAME_COUNT)
#else
Application::Application() : BaseWindow("GamePerformanceStudies", 1280, 720), primaryGpu(GpuAbstractType::PRIMARY_GPU, FRAME_COUNT), secondaryGpu(GpuAbstractType::SECONDARY_GPU, FRAME_COUNT)
#endif // FLIP_GPU
{
    std::cout << "Application::Application()" << std::endl;

    viewport.Width = static_cast<float>(window.getWidth());
    viewport.Height = static_cast<float>(window.getHeight());
    scissorRect.right = window.getWidth();
    scissorRect.bottom = window.getHeight();
}

Application::~Application()
{
    std::cout << "Application::~Application()" << std::endl;
}

void Application::initialize()
{
    std::cout << " Application::initialize()" << std::endl;

    dxgiManager.createDxgiFactory();
    dxgiManager.enumerateAdapters();
    primaryGpu.createDevice(dxgiManager.getPrimaryAdapter());
    primaryGpu.createCommandQueue();
    secondaryGpu.createDevice(dxgiManager.getSecondaryAdapter());
    secondaryGpu.createCommandQueue();
#if FLIP_GPU
    dxgiManager.createPrimarySwapChain(secondaryGpu.getCommandQueue(), FRAME_COUNT, window.getWidth(), window.getHeight(), window.getHwnd());
#else
    dxgiManager.createPrimarySwapChain(primaryGpu.getCommandQueue(), FRAME_COUNT, window.getWidth(), window.getHeight(), window.getHwnd());
#endif

    primaryGpu.createDescriptorHeaps();
    primaryGpu.createFrameResrouces(&dxgiManager);
    primaryGpu.createCommandAllocator();
    primaryGpu.createEmptyRootSignature();

    secondaryGpu.createDescriptorHeaps();
    secondaryGpu.createFrameResrouces(&dxgiManager);
    secondaryGpu.createCommandAllocator();
    secondaryGpu.createEmptyRootSignature();
    createPipelineState();
    primaryGpu.createCommandList(primaryGpu.getPipelineState());
    secondaryGpu.createCommandList(secondaryGpu.getPipelineState());
    primaryGpu.createSyncObjects();
    secondaryGpu.createSyncObjects();

    createVertexBuffer();
}

void Application::tick()
{
    // Record all the commands we need to render the scene into the command list.
    populateCommandList();

    // Execute the command list.
    primaryGpu.executeCommandList();
    primaryGpu.waitForPreviousFrame();
    secondaryGpu.executeCommandList();
    secondaryGpu.waitForPreviousFrame();

    // Present the frame.
    dxgiManager.present();

    // Wait for both frame to finish
    
    
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

    primaryGpu.createPipelineState(inputElementDescs, _countof(inputElementDescs), vertexShader, pixelShader);
    secondaryGpu.createPipelineState(inputElementDescs, _countof(inputElementDescs), vertexShader, pixelShader);
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
    ThrowIfFailed(primaryGpu.getDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&vertexBuffer)));

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

    //-------------------------------------------------------------------------

    ThrowIfFailed(secondaryGpu.getDevice()->CreateCommittedResource(&CD3DX12_HEAP_PROPERTIES(D3D12_HEAP_TYPE_UPLOAD), D3D12_HEAP_FLAG_NONE,
        &CD3DX12_RESOURCE_DESC::Buffer(vertexBufferSize), D3D12_RESOURCE_STATE_GENERIC_READ, nullptr, IID_PPV_ARGS(&secondaryVertexBuffer)));

    // Copy the triangle data to the vertex buffer.
    UINT8* pSecondaryVertexDataBegin;
    //CD3DX12_RANGE readRange(0, 0);        // We do not intend to read from this resource on the CPU.
    ThrowIfFailed(secondaryVertexBuffer->Map(0, &readRange, reinterpret_cast<void**>(&pSecondaryVertexDataBegin)));
    memcpy(pSecondaryVertexDataBegin, triangleVertices, sizeof(triangleVertices));
    secondaryVertexBuffer->Unmap(0, nullptr);

    // Initialize the vertex buffer view.
    secondaryVertexBufferView.BufferLocation = secondaryVertexBuffer->GetGPUVirtualAddress();
    secondaryVertexBufferView.StrideInBytes = sizeof(Vertex);
    secondaryVertexBufferView.SizeInBytes = vertexBufferSize;
}

void Application::populateCommandList()
{
    primaryGpu.resetCommandAllocator();
    primaryGpu.resetCommandList();
    primaryGpu.setRootSignature();
    

    // Set necessary state.
    //primaryGpu.getCommandList()->SetGraphicsRootSignature(primaryGpu.getRootSignature());
    primaryGpu.getCommandList()->RSSetViewports(1, &viewport);
    primaryGpu.getCommandList()->RSSetScissorRects(1, &scissorRect);

    // Indicate that the back buffer will be used as a render target.
    primaryGpu.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(primaryGpu.getRenderTarget(dxgiManager.getCurrentBackBufferIndex()),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvHandle(primaryGpu.getRtvHeap()->GetCPUDescriptorHandleForHeapStart(), dxgiManager.getCurrentBackBufferIndex(), primaryGpu.getDescriptorSize());
    primaryGpu.getCommandList()->OMSetRenderTargets(1, &rtvHandle, FALSE, nullptr);

    // Record commands.
    const float clearColor[] = { 0.0f, 0.2f, 0.4f, 1.0f };
    primaryGpu.getCommandList()->ClearRenderTargetView(rtvHandle, clearColor, 0, nullptr);
    primaryGpu.getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    primaryGpu.getCommandList()->IASetVertexBuffers(0, 1, &vertexBufferView);
    primaryGpu.getCommandList()->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    primaryGpu.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(primaryGpu.getRenderTarget(dxgiManager.getCurrentBackBufferIndex()),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    primaryGpu.closeCommandList();

    //-------------------------------------------------------------------------

    secondaryGpu.resetCommandAllocator();
    secondaryGpu.resetCommandList();
    secondaryGpu.setRootSignature();


    // Set necessary state.
    //secondaryGpu.getCommandList()->SetGraphicsRootSignature(secondaryGpu.getRootSignature());
    secondaryGpu.getCommandList()->RSSetViewports(1, &viewport);
    secondaryGpu.getCommandList()->RSSetScissorRects(1, &scissorRect);

    // Indicate that the back buffer will be used as a render target.
    secondaryGpu.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(secondaryGpu.getRenderTarget(dxgiManager.getCurrentBackBufferIndex()),
        D3D12_RESOURCE_STATE_PRESENT, D3D12_RESOURCE_STATE_RENDER_TARGET));

    CD3DX12_CPU_DESCRIPTOR_HANDLE rtvSecondaryHandle(secondaryGpu.getRtvHeap()->GetCPUDescriptorHandleForHeapStart(), dxgiManager.getCurrentBackBufferIndex(), secondaryGpu.getDescriptorSize());
    secondaryGpu.getCommandList()->OMSetRenderTargets(1, &rtvSecondaryHandle, FALSE, nullptr);

    // Record commands.
    const float secondaryClearColor[] = { 1.0f, 0.0f, 0.0f, 1.0f };
    secondaryGpu.getCommandList()->ClearRenderTargetView(rtvSecondaryHandle, secondaryClearColor, 0, nullptr);
    secondaryGpu.getCommandList()->IASetPrimitiveTopology(D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
    secondaryGpu.getCommandList()->IASetVertexBuffers(0, 1, &secondaryVertexBufferView);
    secondaryGpu.getCommandList()->DrawInstanced(3, 1, 0, 0);

    // Indicate that the back buffer will now be used to present.
    secondaryGpu.getCommandList()->ResourceBarrier(1, &CD3DX12_RESOURCE_BARRIER::Transition(secondaryGpu.getRenderTarget(dxgiManager.getCurrentBackBufferIndex()),
        D3D12_RESOURCE_STATE_RENDER_TARGET, D3D12_RESOURCE_STATE_PRESENT));

    secondaryGpu.closeCommandList();
}
