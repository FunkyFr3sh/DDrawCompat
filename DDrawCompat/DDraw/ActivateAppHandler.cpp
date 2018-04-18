#define CINTERFACE

#include <ddraw.h>

#include "Common/Hook.h"
#include "DDraw/ActivateAppHandler.h"
#include "Gdi/Gdi.h"
#include "Win32/DisplayMode.h"
#include "Win32/FontSmoothing.h"

namespace
{
	WNDPROC g_origDdWndProc = nullptr;

	void activateApp()
	{
		Gdi::enableEmulation();
	}

	void deactivateApp()
	{
		Gdi::disableEmulation();
	}

	LRESULT CALLBACK ddWndProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
	{
		static bool isDisplayChangeNotificationEnabled = true;

		switch (uMsg)
		{
		case WM_ACTIVATEAPP:
		{
			isDisplayChangeNotificationEnabled = false;
			if (TRUE == wParam)
			{
				activateApp();
			}
			else
			{
				deactivateApp();
			}
			LRESULT result = g_origDdWndProc(hwnd, uMsg, wParam, lParam);
			isDisplayChangeNotificationEnabled = true;
			return result;
		}

		case WM_DISPLAYCHANGE:
		{
			// Fix for alt-tabbing in Commandos 2
			if (!isDisplayChangeNotificationEnabled)
			{
				return 0;
			}
			break;
		}
		}

		return g_origDdWndProc(hwnd, uMsg, wParam, lParam);
	}
}

namespace DDraw
{
	namespace ActivateAppHandler
	{
		void setCooperativeLevel(HWND hwnd, DWORD flags)
		{
			static bool isDdWndProcHooked = false;
			if ((flags & DDSCL_FULLSCREEN) && !isDdWndProcHooked)
			{
				g_origDdWndProc = reinterpret_cast<WNDPROC>(GetWindowLongPtr(hwnd, GWLP_WNDPROC));
				Compat::hookFunction(reinterpret_cast<void*&>(g_origDdWndProc), ddWndProc);
				isDdWndProcHooked = true;
			}
		}
	}
}
