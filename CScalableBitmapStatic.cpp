#include "pch.h"

#include "CScalableBitmapStatic.h"

BEGIN_MESSAGE_MAP(CScalableBitmapStatic, CStatic)
ON_WM_PAINT()
END_MESSAGE_MAP()

void CScalableBitmapStatic::SetBitmapResource(UINT nResID) {
    m_bitmapResID = nResID;
    Invalidate(); // trigger redraw
}

void CScalableBitmapStatic::OnPaint() {
    CPaintDC dc(this);
    CRect rect;
    GetClientRect(&rect);

    if (m_bitmapResID == 0)
        return;

    HBITMAP hBmp = (HBITMAP)::LoadImage(AfxGetInstanceHandle(), MAKEINTRESOURCE(m_bitmapResID),
                                        IMAGE_BITMAP, 0, 0, LR_CREATEDIBSECTION);

    if (!hBmp)
        return;

    BITMAP bmp = {};
    GetObject(hBmp, sizeof(BITMAP), &bmp);

    CDC memDC;
    memDC.CreateCompatibleDC(&dc);
    CBitmap* pOldBmp = memDC.SelectObject(CBitmap::FromHandle(hBmp));

    dc.SetStretchBltMode(HALFTONE);

    // Get actual DPI of the control's HDC
    int dpiX = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSX);
    int dpiY = GetDeviceCaps(dc.GetSafeHdc(), LOGPIXELSY);
    float scaleX = dpiX / 96.0f;
    float scaleY = dpiY / 96.0f;

    // Control size in logical pixels
    int logicalWidth = static_cast<int>(rect.Width() * scaleX);
    int logicalHeight = static_cast<int>(rect.Height() * scaleY);

    // Calculate scaled size while preserving aspect ratio
    float aspectBmp = (float)bmp.bmWidth / bmp.bmHeight;
    float aspectCtrl = (float)logicalWidth / logicalHeight;

    int drawWidth, drawHeight;

    drawWidth = rect.Width(); // in device units
    drawHeight = static_cast<int>(drawWidth / aspectBmp);

    drawHeight = rect.Height();
    drawWidth = static_cast<int>(drawHeight * aspectBmp);

    int offsetX = (rect.Width() - drawWidth) / 2;
    int offsetY = (rect.Height() - drawHeight) / 2;

    // Optional: clear background
    dc.FillSolidRect(&rect, GetSysColor(COLOR_BTNFACE));

    // Stretch and draw
    dc.StretchBlt(offsetX, offsetY, logicalWidth, logicalHeight, &memDC, 0, 0, bmp.bmWidth,
                  bmp.bmHeight, SRCCOPY);

    memDC.SelectObject(pOldBmp);
    ::DeleteObject(hBmp);
}
