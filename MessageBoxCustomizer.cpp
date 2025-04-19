#include "pch.h"

#include "MessageBoxCustomizer.h"
#include <afxwin.h> // Needed for AfxGetApp()

MessageBoxCustomizer& MessageBoxCustomizer::Instance() {
    static MessageBoxCustomizer instance;
    return instance;
}

MessageBoxCustomizer::MessageBoxCustomizer() {
    m_BackgroundBrush = CreateSolidBrush(m_BackgroundColor);
}

void MessageBoxCustomizer::Initialize() {
    if (!m_Hook) {
        m_Hook = SetWindowsHookEx(WH_CBT, CBTProc,
                                  AfxGetApp()->m_hInstance, // Safe in MFC apps
                                  GetCurrentThreadId());
    }
}

LRESULT CALLBACK MessageBoxCustomizer::CBTProc(int nCode, WPARAM wParam, LPARAM lParam) {
    if (nCode == HCBT_ACTIVATE) {
        HWND hMsgBox = (HWND)wParam;
        auto& instance = Instance();

        instance.m_OldMsgBoxProc =
            (WNDPROC)SetWindowLongPtr(hMsgBox, GWLP_WNDPROC, (LONG_PTR)CustomMsgBoxProc);

        UnhookWindowsHookEx(instance.m_Hook);
        instance.m_Hook = nullptr;
    }

    return CallNextHookEx(Instance().m_Hook, nCode, wParam, lParam);
}

LRESULT CALLBACK MessageBoxCustomizer::CustomMsgBoxProc(HWND hWnd, UINT msg, WPARAM wParam,
                                                        LPARAM lParam) {
    auto& instance = Instance();

    switch (msg) {
        case WM_ERASEBKGND: {
            HDC hdc = (HDC)wParam;
            RECT rc;
            GetClientRect(hWnd, &rc);
            FillRect(hdc, &rc, instance.m_BackgroundBrush);
            return 1;
        }
        case WM_CTLCOLORSTATIC: {
            HDC hdc = (HDC)wParam;
            SetTextColor(hdc, instance.m_TextColor);
            SetBkColor(hdc, instance.m_BackgroundColor);
            return (INT_PTR)instance.m_BackgroundBrush;
        }
    }

    return CallWindowProc(instance.m_OldMsgBoxProc, hWnd, msg, wParam, lParam);
}