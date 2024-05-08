#pragma once
#include <Windows.h>

class Overlay {
public:
    Overlay(HINSTANCE hInstance, const wchar_t* className, const wchar_t* windowName, int width, int height);
    ~Overlay();

    HWND GetWindowHandle() const { return m_hwnd; }

private:
    static LRESULT CALLBACK WndProc(HWND hwnd, UINT message, WPARAM wParam, LPARAM lParam);

    HWND m_hwnd;
    WNDCLASSEXW m_wc;
};