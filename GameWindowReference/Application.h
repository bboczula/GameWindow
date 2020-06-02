#pragma once

#include "BaseWindow.h"

class Application : public BaseWindow
{
public:
    Application();
    ~Application();
    void initialize() override;
    void update() override;
    void render() override;
};