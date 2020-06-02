#pragma once

#include <string>
#include <Windows.h>
#include <windowsx.h>
#include <stdint.h>
typedef uint64_t QWORD;

#include "IWindowComponent.h"

#define MAX_TITLE_LENGTH 64
#define MAX_WINDOW_COMPONENTS 2

class BaseWindow
{
public:
	BaseWindow(const char* title, int width, int heigh);
	virtual ~BaseWindow();
	void start();
	void add(IWindowComponent* component);
protected:
	CHAR title[MAX_TITLE_LENGTH];
	const UINT16 width;
	const UINT16 height;
	HWND hWindow;
	HINSTANCE instance;
	virtual void initialize();
	virtual void update();
	virtual void render();
private:
	void registerWindow();
	void createWindow();
	void presentWindow();
	static LRESULT CALLBACK ThisWindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	LRESULT WindowProcedure(HWND hwnd, UINT msg, WPARAM wParam, LPARAM lParam);
	UINT16 numOfComponents;
	IWindowComponent* components[MAX_WINDOW_COMPONENTS];
	const LONG windowStyleEx;
	const LONG windowStyle;
};