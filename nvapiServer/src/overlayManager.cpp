#define NOMINMAX
#include <windows.h>
#include <d2d1.h>
#include <dwrite.h>
#include <string>
#include <iostream>
#include <algorithm>
#include "overlayManager.h"
#include "global.h"

// Link necessary libraries.
#pragma comment(lib, "d2d1.lib")
#pragma comment(lib, "Dwrite.lib")

// Structure to store per-window overlay data.
struct OverlayData {
    int displayNumber;
    RECT monitorRect;
    // Per-window Direct2D render target and brush.
    ID2D1HwndRenderTarget* pRenderTarget;
    ID2D1SolidColorBrush* pBrush;
    // Pointer to the shared text format (AddRef'd so it remains valid).
    IDWriteTextFormat* pTextFormat;
};

// Forward declaration of our window procedure.
LRESULT CALLBACK OverlayWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
    OverlayData* pData = reinterpret_cast<OverlayData*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));
    switch (message) {
    case WM_PAINT: {
        PAINTSTRUCT ps;
        BeginPaint(hWnd, &ps);
        if (pData && pData->pRenderTarget) {
            pData->pRenderTarget->BeginDraw();
            // Clear with a transparent background.
            pData->pRenderTarget->Clear(D2D1::ColorF(0, 0.0f));
            // Prepare the text (the display number).
            std::wstring text = std::to_wstring(pData->displayNumber);
            // Get the client area of the window.
            RECT rc;
            GetClientRect(hWnd, &rc);
            D2D1_RECT_F layoutRect = D2D1::RectF(static_cast<FLOAT>(rc.left), static_cast<FLOAT>(rc.top), static_cast<FLOAT>(rc.right), static_cast<FLOAT>(rc.bottom));
            // Draw the text in white.
            pData->pRenderTarget->DrawText(text.c_str(), static_cast<UINT32>(text.length()), pData->pTextFormat, layoutRect, pData->pBrush);
            HRESULT hr = pData->pRenderTarget->EndDraw();
            if (FAILED(hr)) {
                std::cerr << "Overlay: EndDraw failed with HRESULT: " << hr << std::endl;
            }
        }
        EndPaint(hWnd, &ps);
        return 0;
    }
    case WM_ERASEBKGND:
        return 1; // Prevent flicker.
    case WM_DESTROY: {
        if (pData) {
            if (pData->pBrush) pData->pBrush->Release();
            if (pData->pRenderTarget) pData->pRenderTarget->Release();
            if (pData->pTextFormat) pData->pTextFormat->Release();
            delete pData;
        }
        return 0;
    }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }
}

// -------------------- OverlayManager Implementation --------------------

OverlayManager::OverlayManager() : m_threadHandle(nullptr), m_threadId(0), m_active(false) {
    m_d2d.pFactory = nullptr;
    m_d2d.pDWriteFactory = nullptr;
    m_d2d.pTextFormat = nullptr;
}

OverlayManager::~OverlayManager() {
    if (m_active)
        StopOverlayThread();
    CleanupD2DResources();
}

void OverlayManager::ToggleOverlays(const std::vector<int>& displayNumbers) {
    if (!m_active) {
        m_displayNumbers = displayNumbers;
        m_monitorRects.clear();
        if (!EnumDisplayMonitors(nullptr, nullptr, MonitorEnumProc, reinterpret_cast<LPARAM>(&m_monitorRects))) {
            std::cerr << "Overlay: EnumDisplayMonitors failed." << std::endl;
            return;
        }
        StartOverlayThread(m_displayNumbers);
    }
    else {
        StopOverlayThread();
    }
}

void OverlayManager::StartOverlayThread(const std::vector<int>& /*displayNumbers*/) {
    if (m_active)
        return;
    m_active = true;
    m_threadHandle = CreateThread(nullptr, 0, OverlayThreadProc, this, 0, &m_threadId);
    if (!m_threadHandle) {
        std::cerr << "Overlay: Failed to create overlay thread." << std::endl;
        m_active = false;
    }
}

void OverlayManager::StopOverlayThread() {
    if (!m_active)
        return;
    PostThreadMessage(m_threadId, WM_QUIT, 0, 0);
    WaitForSingleObject(m_threadHandle, INFINITE);
    CloseHandle(m_threadHandle);
    m_threadHandle = nullptr;
    m_active = false;
}

DWORD WINAPI OverlayManager::OverlayThreadProc(LPVOID lpParameter) {
    OverlayManager* pThis = reinterpret_cast<OverlayManager*>(lpParameter);
    pThis->OverlayMessageLoop();
    return 0;
}

void OverlayManager::OverlayMessageLoop() {
    if (!InitializeD2DResources()) {
        std::cerr << "Overlay: Failed to initialize D2D resources." << std::endl;
        return;
    }
    const wchar_t CLASS_NAME[] = L"OverlayWindowClass";
    WNDCLASSEX wc = { 0 };
    wc.cbSize = sizeof(WNDCLASSEX);
    wc.lpfnWndProc = OverlayWndProc;
    wc.hInstance = GetModuleHandle(nullptr);
    wc.lpszClassName = CLASS_NAME;
    wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
    wc.hbrBackground = nullptr;
    if (!RegisterClassEx(&wc)) {
        std::cerr << "Overlay: Failed to register window class." << std::endl;
        return;
    }
    size_t count = std::min(m_displayNumbers.size(), m_monitorRects.size());
    for (size_t i = 0; i < count; i++) {
        HWND hWnd = CreateOverlayWindow(m_displayNumbers[i], m_monitorRects[i]);
        if (hWnd) {
            m_overlayWindows.push_back(hWnd);
            ShowWindow(hWnd, SW_SHOW);
            UpdateWindow(hWnd);
        }
        else {
            std::cerr << "Overlay: Failed to create overlay window for display " << m_displayNumbers[i] << std::endl;
        }
    }
    MSG msg;
    while (GetMessage(&msg, nullptr, 0, 0)) {
        TranslateMessage(&msg);
        DispatchMessage(&msg);
    }
    for (HWND hWnd : m_overlayWindows)
        DestroyWindow(hWnd);
    m_overlayWindows.clear();
    UnregisterClass(CLASS_NAME, GetModuleHandle(nullptr));
}

BOOL CALLBACK OverlayManager::MonitorEnumProc(HMONITOR hMonitor, HDC hdcMonitor, LPRECT lprcMonitor, LPARAM dwData) {
    std::vector<RECT>* pRects = reinterpret_cast<std::vector<RECT>*>(dwData);
    if (pRects)
        pRects->push_back(*lprcMonitor);
    return TRUE;
}

HWND OverlayManager::CreateOverlayWindow(int displayNumber, const RECT& monitorRect) {
    const wchar_t CLASS_NAME[] = L"OverlayWindowClass";
    DWORD dwExStyle = WS_EX_LAYERED | WS_EX_TOPMOST | WS_EX_TRANSPARENT;
    DWORD dwStyle = WS_POPUP;
    int width = monitorRect.right - monitorRect.left;
    int height = monitorRect.bottom - monitorRect.top;
    HWND hWnd = CreateWindowEx(dwExStyle, CLASS_NAME, L"Overlay", dwStyle, monitorRect.left, monitorRect.top, width, height, nullptr, nullptr, GetModuleHandle(nullptr), nullptr);
    if (!hWnd) {
        std::cerr << "Overlay: CreateWindowEx failed with error " << GetLastError() << std::endl;
        return nullptr;
    }
    OverlayData* pData = new OverlayData();
    pData->displayNumber = displayNumber;
    pData->monitorRect = monitorRect;
    pData->pRenderTarget = nullptr;
    pData->pBrush = nullptr;
    pData->pTextFormat = nullptr;
    SetWindowLongPtr(hWnd, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(pData));
    D2D1_SIZE_U size = D2D1::SizeU(width, height);
    HRESULT hr = m_d2d.pFactory->CreateHwndRenderTarget(D2D1::RenderTargetProperties(), D2D1::HwndRenderTargetProperties(hWnd, size), &pData->pRenderTarget);
    if (FAILED(hr)) {
        std::cerr << "Overlay: Failed to create render target. HRESULT: " << hr << std::endl;
        DestroyWindow(hWnd);
        delete pData;
        return nullptr;
    }
    hr = pData->pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pData->pBrush);
    if (FAILED(hr)) {
        std::cerr << "Overlay: Failed to create solid color brush. HRESULT: " << hr << std::endl;
        pData->pRenderTarget->Release();
        DestroyWindow(hWnd);
        delete pData;
        return nullptr;
    }
    m_d2d.pTextFormat->AddRef();
    pData->pTextFormat = m_d2d.pTextFormat;
    return hWnd;
}

bool OverlayManager::InitializeD2DResources() {
    HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &m_d2d.pFactory);
    if (FAILED(hr)) {
        std::cerr << "Overlay: D2D1CreateFactory failed. HRESULT: " << hr << std::endl;
        return false;
    }
    hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&m_d2d.pDWriteFactory));
    if (FAILED(hr)) {
        std::cerr << "Overlay: DWriteCreateFactory failed. HRESULT: " << hr << std::endl;
        return false;
    }
    hr = m_d2d.pDWriteFactory->CreateTextFormat(L"Arial", nullptr, DWRITE_FONT_WEIGHT_BOLD, DWRITE_FONT_STYLE_NORMAL, DWRITE_FONT_STRETCH_NORMAL, 72.0f, L"en-us", &m_d2d.pTextFormat);
    if (FAILED(hr)) {
        std::cerr << "Overlay: CreateTextFormat failed. HRESULT: " << hr << std::endl;
        return false;
    }
    m_d2d.pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
    m_d2d.pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);
    return true;
}

void OverlayManager::CleanupD2DResources() {
    if (m_d2d.pTextFormat) { m_d2d.pTextFormat->Release(); m_d2d.pTextFormat = nullptr; }
    if (m_d2d.pDWriteFactory) { m_d2d.pDWriteFactory->Release(); m_d2d.pDWriteFactory = nullptr; }
    if (m_d2d.pFactory) { m_d2d.pFactory->Release(); m_d2d.pFactory = nullptr; }
}
