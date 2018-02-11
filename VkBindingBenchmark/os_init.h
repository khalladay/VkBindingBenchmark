#pragma once

#ifndef WIN32_LEAN_AND_MEAN
#define WIN32_LEAN_AND_MEAN
#endif

#define NOMINMAX
#include <Windows.h>
#include <timeapi.h>

#include <atlstr.h>

namespace OS
{
	struct AppInfo
	{
		struct HWND__* wndHdl;
		struct HINSTANCE__* instance;
		void(*resizeCallback)(int, int) = nullptr;
		double initialMS = 0.0;
		int curW;
		int curH;
	};

	extern AppInfo GAppInfo;

	void setResizeCallback(void(*cb)(int, int));
	double getMilliseconds();
	void handleEvents();
	HWND makeWindow(HINSTANCE Instance, const char* title, unsigned int width, unsigned int height);

}
