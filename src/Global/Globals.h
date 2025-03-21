// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#pragma once

#ifndef UNICODE
#define UNICODE
#endif

#include "Private.h"
#include "define.h"
#include "SampleIMEBaseStructure.h"
#include <iostream>
#include <string>
#include "FanDictionaryDbUtils.h"
#include <wrl.h>
#include "WebView2.h"

void DllAddRef();
void DllRelease();

using namespace Microsoft::WRL;

namespace Global
{
//---------------------------------------------------------------------
// inline
//---------------------------------------------------------------------

inline void SafeRelease(_In_ IUnknown *punk)
{
    if (punk != nullptr)
    {
        punk->Release();
    }
}

inline void QuickVariantInit(_Inout_ VARIANT *pvar)
{
    pvar->vt = VT_EMPTY;
}

inline void QuickVariantClear(_Inout_ VARIANT *pvar)
{
    switch (pvar->vt)
    {
    // some ovbious VTs that don't need to call VariantClear.
    case VT_EMPTY:
    case VT_NULL:
    case VT_I2:
    case VT_I4:
    case VT_R4:
    case VT_R8:
    case VT_CY:
    case VT_DATE:
    case VT_I1:
    case VT_UI1:
    case VT_UI2:
    case VT_UI4:
    case VT_I8:
    case VT_UI8:
    case VT_INT:
    case VT_UINT:
    case VT_BOOL:
        break;

        // Call release for VT_UNKNOWN.
    case VT_UNKNOWN:
        SafeRelease(pvar->punkVal);
        break;

    default:
        // we call OleAut32 for other VTs.
        VariantClear(pvar);
        break;
    }
    pvar->vt = VT_EMPTY;
}

//+---------------------------------------------------------------------------
//
// IsTooSimilar
//
//  Return TRUE if the colors cr1 and cr2 are so similar that they
//  are hard to distinguish. Used for deciding to use reverse video
//  selection instead of system selection colors.
//
//----------------------------------------------------------------------------

inline BOOL IsTooSimilar(COLORREF cr1, COLORREF cr2)
{
    if ((cr1 | cr2) & 0xFF000000) // One color and/or the other isn't RGB, so algorithm doesn't apply
    {
        return FALSE;
    }

    LONG DeltaR = abs(GetRValue(cr1) - GetRValue(cr2));
    LONG DeltaG = abs(GetGValue(cr1) - GetGValue(cr2));
    LONG DeltaB = abs(GetBValue(cr1) - GetBValue(cr2));

    return DeltaR + DeltaG + DeltaB < 80;
}

//---------------------------------------------------------------------
// extern
//---------------------------------------------------------------------
extern HINSTANCE dllInstanceHandle;

extern ATOM AtomCandidateWindow;
extern ATOM AtomShadowWindow;
extern ATOM AtomScrollBarWindow;

BOOL RegisterWindowClass();

extern LONG dllRefCount;

extern CRITICAL_SECTION CS;
extern HFONT defaultlFontHandle; // Global font object we use everywhere

extern const CLSID SampleIMECLSID;
extern const CLSID SampleIMEGuidProfile;
extern const CLSID SampleIMEGuidImeModePreserveKey;
extern const CLSID SampleIMEGuidDoubleSingleBytePreserveKey;
extern const CLSID SampleIMEGuidPunctuationPreserveKey;

LRESULT CALLBACK ThreadKeyboardProc(int nCode, WPARAM wParam, LPARAM lParam);
BOOL CheckModifiers(UINT uModCurrent, UINT uMod);
BOOL UpdateModifiers(WPARAM wParam, LPARAM lParam);
void LogMessage(const char *message);
void LogMessageW(const wchar_t *message);
void LogWideString(const WCHAR *pwch, DWORD_PTR dwLength);

extern USHORT ModifiersValue;
extern BOOL IsShiftKeyDownOnly;
extern BOOL IsControlKeyDownOnly;
extern BOOL IsAltKeyDownOnly;

extern const GUID SampleIMEGuidCompartmentDoubleSingleByte;
extern const GUID SampleIMEGuidCompartmentPunctuation;

extern const WCHAR FullWidthCharTable[];
extern const struct _PUNCTUATION PunctuationTable[14];

extern const GUID SampleIMEGuidLangBarIMEMode;
extern const GUID SampleIMEGuidLangBarDoubleSingleByte;
extern const GUID SampleIMEGuidLangBarPunctuation;

extern const GUID SampleIMEGuidDisplayAttributeInput;
extern const GUID SampleIMEGuidDisplayAttributeConverted;

extern const GUID SampleIMEGuidCandUIElement;

extern const WCHAR UnicodeByteOrderMark;
extern const WCHAR KeywordDelimiter;
extern const WCHAR StringDelimiter;

extern const WCHAR ImeModeDescription[];
extern const int ImeModeOnIcoIndex;
extern const int ImeModeOffIcoIndex;

extern const WCHAR DoubleSingleByteDescription[];
extern const int DoubleSingleByteOnIcoIndex;
extern const int DoubleSingleByteOffIcoIndex;

extern const WCHAR PunctuationDescription[];
extern const int PunctuationOnIcoIndex;
extern const int PunctuationOffIcoIndex;

extern const WCHAR LangbarImeModeDescription[];
extern const WCHAR LangbarDoubleSingleByteDescription[];
extern const WCHAR LangbarPunctuationDescription[];

std::wstring string_to_wstring(const std::string &str);
std::string wstring_to_string(const std::wstring &wstr);

inline std::vector<FanDictionaryDb::DbWordItem> CandidateList;
inline std::vector<std::wstring> WStringCandidateList;
inline std::wstring FindKeyCode;

// Global Window
inline HWND MainWindowHandle;

// Webview2
static ComPtr<ICoreWebView2Controller> UIWebview2Control;
static ComPtr<ICoreWebView2> UIWebview2;

inline std::wstring HTMLString = LR"(
<!DOCTYPE html>
<html lang="zh-CN">

<head>
  <meta charset="UTF-8">
  <meta name="viewport" content="width=device-width, initial-scale=1.0">
  <title>垂直候选框</title>
  <style>
    body {
      font-family: Arial, sans-serif;
      height: 100vh;
      margin: 0;
      overflow: hidden;
      border-radius: 6px;
      background: transparent;
    }

    .container {
      /* margin-top: 50px;
      margin-left: 50px; */
      margin-top: 0px;
      margin-left: 0px;
      background-color: #202020;
      padding: 2px;
      border-radius: 6px;
      box-shadow: 5px 5px 10px rgba(0, 0, 0, 0.5);
      width: 100px;
      user-select: none;
      border: 2px solid #9b9b9b2e;
    }

    .row {
      justify-content: space-between;
      padding: 2px;
      margin-top: 2px;
    }

    .cand:hover {
      border-radius: 6px;
      background-color: #414141;
    }

    .row-wrapper {
      position: relative;
    }

    /* .cand:hover::before {
      content: "";
      position: absolute;
      left: 0;
      top: 50%;
      transform: translateY(-50%);
      height: 16px;
      width: 3px;
      background: linear-gradient(to bottom, #ff7eb3, #ff758c, #ff5a5f);
      border-radius: 8px;
    } */

    .first {
      background-color: #3e3e3eb9;
      border-radius: 6px;
    }

    .first::before {
      content: "";
      position: absolute;
      left: 0;
      top: 50%;
      transform: translateY(-50%);
      height: 16px;
      width: 3px;
      background: linear-gradient(to bottom, #ff7eb3, #ff758c, #ff5a5f);
      border-radius: 8px;
    }

    .text {
      padding-left: 8px;
      color: #e9e8e8;
    }
  </style>

  </script>
</head>

<body>
  <div class="container">
    <div class="row pinyin">
      <div class="text">ni'uo</div>
    </div>
    <div class="row-wrapper">
      <div class="row cand first">
        <div class="text">1. 你说</div>
      </div>
    </div>
    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">2. 笔画</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">3. 量子</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">4. 牛魔</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">5. 仙人</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">6. 可恨</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">7. 木槿</div>
      </div>
    </div>

    <div class="row-wrapper">
      <div class="row cand">
        <div class="text">8. 无量</div>
      </div>
    </div>
  </div>
</body>

</html>
)";
} // namespace Global
