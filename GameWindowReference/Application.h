#pragma once

#include <d3d12.h>
#include <dxgi.h>
#include <d3dcompiler.h>
#include <iostream>
#include <vector>
#include <DirectXMath.h>

#include "d3dx12.h"
#include "BaseWindow.h"

#pragma comment(lib, "d3d12.lib")
#pragma comment(lib, "D3DCompiler.lib")

class Application : public BaseWindow
{
public:
    Application();
    ~Application();
    void initialize() override;
    void tick() override;
};