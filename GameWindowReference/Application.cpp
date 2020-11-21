#include "Application.h"

#define FLIP_GPU 0

#if FLIP_GPU
Application::Application() : BaseWindow("GamePerformanceStudies", 1280, 720), primaryGpu(GpuAbstractType::SECONDARY_GPU, FRAME_COUNT), secondaryGpu(GpuAbstractType::PRIMARY_GPU, FRAME_COUNT)
#else
Application::Application() : BaseWindow("GamePerformanceStudies", 1280, 720)
#endif // FLIP_GPU
{
    std::cout << "Application::Application()" << std::endl;
    std::cout << "sizeof(BaseWindow): " << sizeof(BaseWindow) << std::endl;
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
