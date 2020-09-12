#include "DisplayDescriptor.h"

Astral::DisplayDescriptor::DisplayDescriptor() : width(1280), height(720)
{
	std::cout << "DisplayDescriptor::DisplayDescriptor()" << std::endl;
}

Astral::DisplayDescriptor::~DisplayDescriptor()
{
	std::cout << "DisplayDescriptor::~DisplayDescriptor()" << std::endl;
}

LONG Astral::DisplayDescriptor::getWidth()
{
	return width;
}

LONG Astral::DisplayDescriptor::getHeight()
{
	return height;
}
