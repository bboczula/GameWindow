#pragma once

#include <Windows.h>
#include <iostream>

class DisplayDescriptor
{
public:
	DisplayDescriptor();
	~DisplayDescriptor();
	LONG getWidth();
	LONG getHeight();
private:
	LONG width;
	LONG height;
};