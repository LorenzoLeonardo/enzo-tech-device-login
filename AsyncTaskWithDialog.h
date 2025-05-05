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

    CAsyncTaskWithDialog(std::shared_ptr<TDialog> dialog, TaskFunc taskFunc, bool showDialog = true)
        : m_pDialog(std::move(dialog)), m_taskFunc(std::move(taskFunc)), m_showDialog(showDialog),
          m_isDone(false) {}

    TResult Await() {
        if (m_showDialog && m_pDialog && ::IsWindow(m_pDialog->GetSafeHwnd())) {
            m_pDialog->ShowWindow(SW_SHOW);
        }

        std::thread worker([dialog = m_pDialog, this]() {
            this->m_result = this->m_taskFunc(dialog.get());
            this->m_isDone = true;
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

        return m_result;
    }

  private:
    std::shared_ptr<TDialog> m_pDialog;
    TaskFunc m_taskFunc;
    bool m_showDialog;
    std::atomic<bool> m_isDone;
    TResult m_result{};
};