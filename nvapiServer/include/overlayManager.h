#pragma once

#include <windows.h>
#include <vector>
#include <d2d1.h>
#include <dwrite.h>

// OverlayManager manages creation, display, and cleanup of overlay windows.
// Each overlay window shows the numeric identifier (as provided by NVAPI order)
// on its corresponding display.

class OverlayManager {
public:
	OverlayManager();
	~OverlayManager();

	// Toggle overlays on/off.
   // When turning on, provide a vector of display numbers (in NVAPI order).
   // If overlays are already active, calling ToggleOverlays() will remove them.

	void ToggleOverlays(const std::vector<int>& displayNumbers);

	bool isActive() const { return m_active; }

private:
	// Starts the overlay thread.

	void StartOverlayThread(const std::vector<int>& displayNumbers);

	// Stops the overlay thread and destroys overlay windows.
	void StopOverlayThread();

	// Thread procedure (static wrapper).
	static DWORD WINAPI OverlayThreadProc(LPVOID lpParameter);

	// The function run on the overlay thread.
	void OverlayMessageLoop();

	// Creates an overlay window for a given monitor rectangle and display number.
	HWND CreateOverlayWindow(int displayNumber, const RECT& monitorRect);

	// Enumerate monitors to get their rectangles.
	static BOOL CALLBACK MonitorEnumProc(HMONITOR, HDC hdcMOnitor, LPRECT lprcMonitor, LPARAM dwData);

	// Initialize and cleanup shared Direct2D/DirectWrite resources.
	bool InitializeD2DResources();
	void CleanupD2DResources();

	// Shared Direct2D/DirectWrite resources
	struct D2DResources {
		ID2D1Factory* pFactory;
		IDWriteFactory* pDWriteFactory;
		IDWriteTextFormat* pTextFormat;

	}m_d2d;

	// Overlay thread handle and id.
	HANDLE m_threadHandle;
	DWORD m_threadId;
	bool m_active;

	// The display numbers to be shown (in NVAPI order).
	std::vector<int> m_displayNumbers;

	// Monitor rectangles obtained via EnumDisplayMonitors
	std::vector<RECT> m_monitorRects;

	// Overlay window handles.
	std::vector<HWND> m_overlayWindows;



};