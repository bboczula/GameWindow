#include "Application.h"

Application::Application() : Engine(960, 540)
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
}

void Application::tick()
{
}