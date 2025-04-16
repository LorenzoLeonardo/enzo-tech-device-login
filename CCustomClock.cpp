#include "pch.h"
#include "CCustomClock.h"

CCustomClock* pCCustomClock;
CCustomClock::CCustomClock() {
	m_csFontStyle = _T("Microsoft Sans Serif");
	m_nFontSize = 12;
	m_nFontWeight = FW_NORMAL;
	m_textColor = RGB(0, 0, 0);
	m_textBKColor = RGB(13, 71, 161);
	pCCustomClock = this;
}
CCustomClock::~CCustomClock() {}

void CCustomClock::CreateClock() {
	VERIFY(m_cfont.CreateFont(m_nFontSize, // nHeight
		0,								   // nWidth
		0,								   // nEscapement
		0,								   // nOrientation
		m_nFontWeight,					   // nWeight
		FALSE,							   // bItalic
		FALSE,							   // bUnderline
		0,								   // cStrikeOut
		ANSI_CHARSET,					   // nCharSet
		OUT_DEFAULT_PRECIS,				   // nOutPrecision
		CLIP_DEFAULT_PRECIS,			   // nClipPrecision
		DEFAULT_QUALITY,				   // nQuality
		DEFAULT_PITCH | FF_SWISS,		   // nPitchAndFamily
		m_csFontStyle));				   // lpszFacename

	VERIFY(m_cFontAMPM.CreateFont(m_nFontSize / 2, // nHeight
		0,										   // nWidth
		0,										   // nEscapement
		0,										   // nOrientation
		m_nFontWeight,							   // nWeight
		FALSE,									   // bItalic
		FALSE,									   // bUnderline
		0,										   // cStrikeOut
		ANSI_CHARSET,							   // nCharSet
		OUT_DEFAULT_PRECIS,						   // nOutPrecision
		CLIP_DEFAULT_PRECIS,					   // nClipPrecision
		DEFAULT_QUALITY,						   // nQuality
		DEFAULT_PITCH | FF_SWISS,				   // nPitchAndFamily
		m_csFontStyle));

	VERIFY(m_cFontDate.CreateFont(m_nFontSize / 3, // nHeight
		0,										   // nWidth
		0,										   // nEscapement
		0,										   // nOrientation
		m_nFontWeight,							   // nWeight
		FALSE,									   // bItalic
		FALSE,									   // bUnderline
		0,										   // cStrikeOut
		ANSI_CHARSET,							   // nCharSet
		OUT_DEFAULT_PRECIS,						   // nOutPrecision
		CLIP_DEFAULT_PRECIS,					   // nClipPrecision
		DEFAULT_QUALITY,						   // nQuality
		DEFAULT_PITCH | FF_SWISS,				   // nPitchAndFamily
		m_csFontStyle));
}
void CCustomClock::DestroyClock() {
	m_cfont.DeleteObject();
	m_cFontAMPM.DeleteObject();
	m_cFontDate.DeleteObject();
}
CString CCustomClock::GetDateTime() {
	SYSTEMTIME sysTime;
	WORD wHour = 0;
	CString csDateTime;

	GetLocalTime(&sysTime);
	wHour = sysTime.wHour;
	if (sysTime.wHour == 0)
		wHour = 12;
	else if (sysTime.wHour > 12)
		wHour = sysTime.wHour - 12;

	csDateTime.Format(_T("%s %02d, %04d "),
		pCCustomClock->GetMonthName(sysTime.wMonth).GetBuffer(),
		sysTime.wDay, sysTime.wYear);

	if (sysTime.wHour >= 12)
		csDateTime.AppendFormat(_T("%d:%02d:%02d PM"), wHour, sysTime.wMinute,
			sysTime.wSecond);
	else
		csDateTime.AppendFormat(_T("%d:%02d:%02d AM"), wHour, sysTime.wMinute,
			sysTime.wSecond);

	return csDateTime;
}
void CCustomClock::DrawClock(CClientDC* dcSrc, int x, int y) {
	// 1. Clear previous clock area
	CRect inflatedRect = m_rectClock;
	inflatedRect.InflateRect(15, 10); // Expand by 5 pixels on all sides
	dcSrc->FillSolidRect(&inflatedRect, m_textBKColor);

	SYSTEMTIME sysTime;
	SIZE sizeTime = {}, sizeAMPM = {}, sizeDate = {};
	TEXTMETRIC tmTime = {}, tmDate = {};
	CString csTime = _T(""), csAMPM = _T(""), csDate = _T("");
	WORD wHour = 0;

	// 2. Get current time
	GetLocalTime(&sysTime);
	wHour = sysTime.wHour;
	if (wHour == 0)
		wHour = 12;
	else if (wHour > 12)
		wHour -= 12;

	// Format time, AM/PM, and date strings
	csTime.Format(_T("%d:%02d:%02d"), wHour, sysTime.wMinute, sysTime.wSecond);
	csAMPM = (sysTime.wHour >= 12) ? _T(" PM") : _T(" AM");
	csDate.Format(
		_T("%s, %s %02d, %04d"),
		CalcDayOfWeek(sysTime.wYear, sysTime.wMonth, sysTime.wDay).GetBuffer(),
		GetMonthName(sysTime.wMonth).GetBuffer(), sysTime.wDay, sysTime.wYear
	);

	// 3. Draw Time
	dcSrc->SetBkMode(TRANSPARENT);
	dcSrc->SetTextColor(m_textColor);
	dcSrc->SetBkColor(m_textBKColor);
	CFont* pOldFont = dcSrc->SelectObject(&m_cfont);

	dcSrc->TextOut(x, y, csTime);

	GetTextExtentPoint32(dcSrc->GetSafeHdc(), csTime, csTime.GetLength(), &sizeTime);
	GetTextMetrics(dcSrc->GetSafeHdc(), &tmTime);
	dcSrc->SelectObject(pOldFont);

	// 4. Draw AM/PM
	dcSrc->SetBkMode(TRANSPARENT);
	dcSrc->SetTextColor(m_textColor);
	pOldFont = dcSrc->SelectObject(&m_cFontAMPM);
	dcSrc->TextOut(x + sizeTime.cx, y + (tmTime.tmAscent / 2), csAMPM);

	GetTextExtentPoint32(dcSrc->GetSafeHdc(), csAMPM, csAMPM.GetLength(), &sizeAMPM);
	dcSrc->SelectObject(pOldFont);

	// 5. Draw Date
	dcSrc->SetBkMode(TRANSPARENT);
	dcSrc->SetTextColor(m_textColor);
	pOldFont = dcSrc->SelectObject(&m_cFontDate);
	dcSrc->TextOut(x, y + tmTime.tmHeight, csDate);

	GetTextExtentPoint32(dcSrc->GetSafeHdc(), csDate, csDate.GetLength(), &sizeDate);
	GetTextMetrics(dcSrc->GetSafeHdc(), &tmDate);
	dcSrc->SelectObject(pOldFont);

	// 6. Update m_rectClock to wrap entire area
	int totalWidth = max(sizeTime.cx + sizeAMPM.cx, sizeDate.cx);
	int totalHeight = tmTime.tmHeight + tmDate.tmHeight;

	m_rectClock.left = x;
	m_rectClock.top = y;
	m_rectClock.right = x + totalWidth;
	m_rectClock.bottom = y + totalHeight;
}

ULONG CCustomClock::CalcDayNumFromDate(UINT y, UINT m, UINT d) {
	m = (m + 9) % 12;
	y -= m / 10;
	ULONG dn = 365 * y + y / 4 - y / 100 + y / 400 + (m * 306 + 5) / 10 + (d - 1);

	return dn;
}

// ----------------------------------------------------------------------
// Given year, month, day, return the day of week (string).
// ----------------------------------------------------------------------
CString CCustomClock::CalcDayOfWeek(WORD y, WORD m, WORD d) {
	CString day[] = { _T("Wednesday"), _T("Thursday"), _T("Friday"),
		_T("Saturday"), _T("Sunday"), _T("Monday"),
		_T("Tuesday") };

	ULONG dn = CalcDayNumFromDate(y, m, d);

	return day[dn % 7];
}

CString CCustomClock::GetMonthName(WORD wMonth) {
	CString day[] = { _T("Error"), _T("January"), _T("February"),
		_T("March"), _T("April"), _T("May"),
		_T("June"), _T("July"), _T("August"),
		_T("September"), _T("October"), _T("November"),
		_T("December") };

	return day[wMonth];
}