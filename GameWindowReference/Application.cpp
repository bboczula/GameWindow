#include "Application.h"

Application::Application() : Game(960, 540)
{
    std::cout << "Application::Application()" << std::endl;
}

Application::~Application()
{
    std::cout << "Application::~Application()" << std::endl;
}

void Application::Initialization()
{
    std::cout << " Application::initialize()" << std::endl;
}

void Application::Logic()
{
}