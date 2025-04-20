#pragma once

#include <afxwin.h>
#include <atomic>
#include <functional>
#include <memory>
#include <thread>

template <typename TDialog, typename TResult>
class CAsyncTaskWithDialog {
  public:
    using TaskFunc = std::function<TResult(TDialog*)>;

    CAsyncTaskWithDialog(TDialog* pDialog, TaskFunc taskFunc)
        : m_pDialog(pDialog), m_taskFunc(taskFunc), m_isDone(false) {}

    TResult Await() {
        if (m_pDialog && ::IsWindow(m_pDialog->GetSafeHwnd())) {
            m_pDialog->ShowWindow(SW_SHOW);
        }

        std::thread worker([&]() {
            m_result = m_taskFunc(m_pDialog);
            m_isDone = true;
        });
        worker.detach();

        MSG msg;
        while (!m_isDone) {
            while (::PeekMessage(&msg, nullptr, 0, 0, PM_REMOVE)) {
                ::TranslateMessage(&msg);
                ::DispatchMessage(&msg);
            }
            Sleep(10);
        }

        if (m_pDialog && ::IsWindow(m_pDialog->GetSafeHwnd())) {
            m_pDialog->DestroyWindow();
        }

        return m_result;
    }

  private:
    TDialog* m_pDialog;
    TaskFunc m_taskFunc;
    std::atomic<bool> m_isDone;
    TResult m_result{};
};