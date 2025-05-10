#pragma once
#include <windows.h>

#include "Definitions.h"

class MessageBoxCustomizer {
  public:
    static MessageBoxCustomizer& Instance(); // Access globally
    void Initialize();                       // Call once at app start

  private:
    MessageBoxCustomizer(); // Private constructor
    MessageBoxCustomizer(const MessageBoxCustomizer&) = delete;
    MessageBoxCustomizer& operator=(const MessageBoxCustomizer&) = delete;

    static LRESULT CALLBACK CBTProc(int nCode, WPARAM wParam, LPARAM lParam);
    static LRESULT CALLBACK CustomMsgBoxProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam);

    WNDPROC m_OldMsgBoxProc = nullptr;
    HHOOK m_Hook = nullptr;

    HBRUSH m_BackgroundBrush = nullptr;
    COLORREF m_BackgroundColor = DLG_BACKGROUND_COLOR;
    COLORREF m_TextColor = COLOR_WHITE;
};