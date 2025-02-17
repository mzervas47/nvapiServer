#include "d2dHelper.h"
#include <iostream>
#include <string>

ID2D1Factory* pFactory = nullptr;
ID2D1HwndRenderTarget* pRenderTarget = nullptr;
ID2D1SolidColorBrush* pBrush = nullptr;
IDWriteFactory* pDWriteFactory = nullptr;
IDWriteTextFormat* pTextFormat = nullptr;

void initializeD2D(HWND hwnd) {
	HRESULT hr = D2D1CreateFactory(D2D1_FACTORY_TYPE_SINGLE_THREADED, &pFactory);

	if (FAILED(hr)) {
		printf("Failed to create D2D factory. HRESULT: %d\n", hr);
		return;
			
	}

	hr = DWriteCreateFactory(DWRITE_FACTORY_TYPE_SHARED, __uuidof(IDWriteFactory), reinterpret_cast<IUnknown**>(&pDWriteFactory));
	if (FAILED(hr)) {
		printf("Faled to create DirectWrite factory. HRESULT: %d\n", hr);
		return;
	}

	hr = pDWriteFactory->CreateTextFormat(
		L"Arial",
		nullptr,
		DWRITE_FONT_WEIGHT_NORMAL,
		DWRITE_FONT_STYLE_NORMAL,
		DWRITE_FONT_STRETCH_NORMAL,
		36.0f,
		L"en-us",
		&pTextFormat

	);

	if (FAILED(hr)) {
		printf("Failed to create text format. HRESULT: %d\n", hr);
		return;
	}

	pTextFormat->SetTextAlignment(DWRITE_TEXT_ALIGNMENT_CENTER);
	pTextFormat->SetParagraphAlignment(DWRITE_PARAGRAPH_ALIGNMENT_CENTER);

	RECT rc;
	GetClientRect(hwnd, &rc);
	D2D1_SIZE_U size = D2D1::SizeU(rc.right - rc.left, rc.bottom - rc.top);

	hr = pFactory->CreateHwndRenderTarget(
		D2D1::RenderTargetProperties(),
		D2D1::HwndRenderTargetProperties(hwnd, size),
		&pRenderTarget

	);

	if (FAILED(hr)) {
		printf("Failed to create render target. HRESULT: %d\n", hr);
		return;
	}

	hr = pRenderTarget->CreateSolidColorBrush(D2D1::ColorF(D2D1::ColorF::White), &pBrush);
	if (FAILED(hr)) {
		printf("Failed to create brush. HRESULT: %d\n", hr);
	}


}

void renderDisplayID(int displayID, RECT bounds) {
	if (!pRenderTarget) {
		std::cerr << "Render target not initialized.\n";
		return;
	}

	pRenderTarget->BeginDraw();


	pRenderTarget->Clear(D2D1::ColorF(D2D1::ColorF::Black, 0.0f));


	std::wstring displayIDText = std::to_wstring(displayID);

	D2D1_RECT_F layoutRect = D2D1::RectF(
		static_cast<FLOAT>(bounds.left),
		static_cast<FLOAT>(bounds.top),
		static_cast<FLOAT>(bounds.right),
		static_cast<FLOAT>(bounds.bottom)
	);

	pRenderTarget->DrawText(
		displayIDText.c_str(),
		displayIDText.length(),
		pTextFormat,
		layoutRect,
		pBrush

	);

	HRESULT hr = pRenderTarget->EndDraw();
	if (FAILED(hr)) {
		printf("Failed to draw. HRESULT: %d\n", hr);
	}

}


void cleanupD2D() {
	if (pBrush) pBrush->Release();
	if (pRenderTarget) pRenderTarget->Release();
	if (pTextFormat) pTextFormat->Release();
	if (pDWriteFactory) pDWriteFactory->Release();
	if (pFactory) pFactory->Release();
}