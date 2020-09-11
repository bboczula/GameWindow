#include "DisplayDescriptor.h"

DisplayDescriptor::DisplayDescriptor() : width(1280), height(720)
{
	std::cout << "DisplayDescriptor::DisplayDescriptor()" << std::endl;
}

DisplayDescriptor::~DisplayDescriptor()
{
	std::cout << "DisplayDescriptor::~DisplayDescriptor()" << std::endl;
}

LONG DisplayDescriptor::getWidth()
{
	return width;
}

LONG DisplayDescriptor::getHeight()
{
	return height;
}
