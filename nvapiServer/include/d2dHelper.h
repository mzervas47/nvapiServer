#pragma once

#include "d2d1.h"
#include "dwrite.h"

void initializeD2D(HWND hwnd);
void renderDisplayID(int displayID, RECT bounds);
void cleanupD2D();
