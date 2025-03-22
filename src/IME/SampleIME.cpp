// THIS CODE AND INFORMATION IS PROVIDED "AS IS" WITHOUT WARRANTY OF
// ANY KIND, EITHER EXPRESSED OR IMPLIED, INCLUDING BUT NOT LIMITED TO
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND/OR FITNESS FOR A
// PARTICULAR PURPOSE.
//
// Copyright (c) Microsoft Corporation. All rights reserved

#include "Private.h"
#include "Globals.h"
#include "SampleIME.h"
#include "CandidateListUIPresenter.h"
#include "CompositionProcessorEngine.h"
#include "Compartment.h"
#include "define.h"
#include <winnt.h>
#include <winuser.h>
#include <Windows.h>

//+---------------------------------------------------------------------------
//
// CreateInstance
//
//----------------------------------------------------------------------------

/* static */
HRESULT CSampleIME::CreateInstance(_In_ IUnknown *pUnkOuter, REFIID riid, _Outptr_ void **ppvObj)
{
    CSampleIME *pSampleIME = nullptr;
    HRESULT hr = S_OK;

    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (nullptr != pUnkOuter)
    {
        return CLASS_E_NOAGGREGATION;
    }

    pSampleIME = new (std::nothrow) CSampleIME();
    if (pSampleIME == nullptr)
    {
        return E_OUTOFMEMORY;
    }

    hr = pSampleIME->QueryInterface(riid, ppvObj);

    pSampleIME->Release();

    return hr;
}

//+---------------------------------------------------------------------------
//
// ctor
//
//----------------------------------------------------------------------------

CSampleIME::CSampleIME()
{
    DllAddRef();

    _pThreadMgr = nullptr;

    _threadMgrEventSinkCookie = TF_INVALID_COOKIE;

    _pTextEditSinkContext = nullptr;
    _textEditSinkCookie = TF_INVALID_COOKIE;

    _activeLanguageProfileNotifySinkCookie = TF_INVALID_COOKIE;

    _dwThreadFocusSinkCookie = TF_INVALID_COOKIE;

    _pComposition = nullptr;

    _pCompositionProcessorEngine = nullptr;

    _candidateMode = CANDIDATE_NONE;
    _pCandidateListUIPresenter = nullptr;
    _isCandidateWithWildcard = FALSE;

    _pDocMgrLastFocused = nullptr;

    _pSIPIMEOnOffCompartment = nullptr;
    _dwSIPIMEOnOffCompartmentSinkCookie = 0;
    _msgWndHandle = nullptr;

    _pContext = nullptr;

    _refCount = 1;
}

//+---------------------------------------------------------------------------
//
// dtor
//
//----------------------------------------------------------------------------

CSampleIME::~CSampleIME()
{
    if (_pCandidateListUIPresenter)
    {
        delete _pCandidateListUIPresenter;
        _pCandidateListUIPresenter = nullptr;
    }
    DllRelease();
}

//+---------------------------------------------------------------------------
//
// QueryInterface
//
//----------------------------------------------------------------------------

STDAPI CSampleIME::QueryInterface(REFIID riid, _Outptr_ void **ppvObj)
{
    if (ppvObj == nullptr)
    {
        return E_INVALIDARG;
    }

    *ppvObj = nullptr;

    if (IsEqualIID(riid, IID_IUnknown) || IsEqualIID(riid, IID_ITfTextInputProcessor))
    {
        *ppvObj = (ITfTextInputProcessor *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextInputProcessorEx))
    {
        *ppvObj = (ITfTextInputProcessorEx *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadMgrEventSink))
    {
        *ppvObj = (ITfThreadMgrEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfTextEditSink))
    {
        *ppvObj = (ITfTextEditSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfKeyEventSink))
    {
        *ppvObj = (ITfKeyEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfActiveLanguageProfileNotifySink))
    {
        *ppvObj = (ITfActiveLanguageProfileNotifySink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfCompositionSink))
    {
        *ppvObj = (ITfKeyEventSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfDisplayAttributeProvider))
    {
        *ppvObj = (ITfDisplayAttributeProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfThreadFocusSink))
    {
        *ppvObj = (ITfThreadFocusSink *)this;
    }
    else if (IsEqualIID(riid, IID_ITfFunctionProvider))
    {
        *ppvObj = (ITfFunctionProvider *)this;
    }
    else if (IsEqualIID(riid, IID_ITfFunction))
    {
        *ppvObj = (ITfFunction *)this;
    }
    else if (IsEqualIID(riid, IID_ITfFnGetPreferredTouchKeyboardLayout))
    {
        *ppvObj = (ITfFnGetPreferredTouchKeyboardLayout *)this;
    }

    if (*ppvObj)
    {
        AddRef();
        return S_OK;
    }

    return E_NOINTERFACE;
}

//+---------------------------------------------------------------------------
//
// AddRef
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CSampleIME::AddRef()
{
    return ++_refCount;
}

//+---------------------------------------------------------------------------
//
// Release
//
//----------------------------------------------------------------------------

STDAPI_(ULONG) CSampleIME::Release()
{
    LONG cr = --_refCount;

    assert(_refCount >= 0);

    if (_refCount == 0)
    {
        delete this;
    }

    return cr;
}

void InitWebview(HWND hWnd)
{
    CreateCoreWebView2EnvironmentWithOptions(
        nullptr, nullptr, nullptr,
        Callback<ICoreWebView2CreateCoreWebView2EnvironmentCompletedHandler>([hWnd](HRESULT result,
                                                                                    ICoreWebView2Environment *env)
                                                                                 -> HRESULT {
            if (result != S_OK)
            {
                Global::LogMessageW(L"Failed to create WebView2 environment.");
                return result;
            }

            // Create a CoreWebView2Controller and get the associated
            // CoreWebView2
            env->CreateCoreWebView2Controller(
                hWnd, Callback<ICoreWebView2CreateCoreWebView2ControllerCompletedHandler>(
                          [hWnd](HRESULT result, ICoreWebView2Controller *controller) -> HRESULT {
                              if (controller == nullptr || FAILED(result))
                              {
#ifdef FANY_DEBUG
                                  Global::LogMessageW(L"Failed to create WebView2 controller.");
#endif
                                  return E_FAIL;
                              }
                              else
                              {
#ifdef FANY_DEBUG
                                  Global::LogMessageW(L"Succeeded to create WebView2 controller.");
#endif
                              }

                              Global::UIWebview2Control = controller;
                              Global::UIWebview2Control->get_CoreWebView2(Global::UIWebview2.GetAddressOf());

                              if (Global::UIWebview2 == nullptr)
                              {
                                  Global::LogMessageW(L"Failed to get WebView2 instance.");
                                  return E_FAIL;
                              }

                              ComPtr<ICoreWebView2_3> webview3;
                              // Add a few settings for the webview
                              ICoreWebView2Settings *settings;
                              Global::UIWebview2->get_Settings(&settings);
                              settings->put_IsScriptEnabled(TRUE);
                              settings->put_AreDefaultScriptDialogsEnabled(TRUE);
                              settings->put_IsWebMessageEnabled(TRUE);
                              settings->put_AreHostObjectsAllowed(TRUE);

                              if (Global::UIWebview2->QueryInterface(IID_PPV_ARGS(&webview3)) == S_OK)
                              {
                                  webview3->SetVirtualHostNameToFolderMapping(
                                      L"appassets", // 虚拟主机名
                                      L"C:"
                                      L"\\Users\\SonnyCalcr\\AppData\\Roaming\\Po"
                                      L"tPla"
                                      L"yerMini64\\Capture",
                                      COREWEBVIEW2_HOST_RESOURCE_ACCESS_KIND_DENY_CORS);
                              }

                              ComPtr<ICoreWebView2Controller2> webviewController2;
                              if (controller->QueryInterface(IID_PPV_ARGS(&webviewController2)) == S_OK)
                              {
                                  COREWEBVIEW2_COLOR backgroundColor = {0, 0, 0, 0};
                                  webviewController2->put_DefaultBackgroundColor(backgroundColor);
                              }

                              // Resize WebView to fit the bounds of the parent
                              // window
                              RECT bounds;
                              GetClientRect(hWnd, &bounds);
                              Global::UIWebview2Control->put_Bounds(bounds);
                              // Navigate to a simple HTML string
                              auto hr = Global::UIWebview2->NavigateToString(Global::HTMLString.c_str());
                              if (FAILED(hr))
                              {
                                  Global::LogMessageW(L"Failed to navigate to string.");
                              }

                              return S_OK;
                          })
                          .Get());
            return S_OK;
        }).Get());
}

static LRESULT CALLBACK GlobalCandidateWndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
    switch (message)
    {
    case WM_ACTIVATE:
        if (LOWORD(wParam) != WA_INACTIVE)
        {
            ShowWindow(hWnd, SW_SHOWNOACTIVATE);
        }
        break;
    case WM_SHOW_MAIN_WINDOW: {
        ShowWindow(hWnd, SW_SHOWNOACTIVATE);
        return 0;
    }
    case WM_HIDE_MAIN_WINDOW: {
        ShowWindow(hWnd, SW_HIDE);
        return 0;
    }
    case WM_MOVE_CANDIDATE_WINDOW: {
        POINT *pt = (POINT *)lParam;
        MoveWindow(hWnd, pt->x, pt->y, (108 + 15) * 1.5, (246 + 15) * 1.5, TRUE);
        if (pt != nullptr)
        {
            delete pt;
        }
        return 0;
    }
    // case WM_SET_PARENT_HWND: {
    //     SetParent(hWnd, (HWND)lParam);
    //     return 0;
    // }
    default:
        return DefWindowProc(hWnd, message, wParam, lParam);
    }

    return 0;
}

HWND CreateGlobalCandidateWindow()
{
    WNDCLASSEX wcex;

    WCHAR szWindowClass[] = L"global_candidate_window";

    wcex.cbSize = sizeof(WNDCLASSEX);
    wcex.style = CS_HREDRAW | CS_VREDRAW;
    wcex.lpfnWndProc = GlobalCandidateWndProc;
    wcex.cbClsExtra = 0;
    wcex.cbWndExtra = 0;
    wcex.hInstance = Global::dllInstanceHandle;
    wcex.hIcon = nullptr;
    wcex.hCursor = LoadCursor(NULL, IDC_ARROW);
    wcex.hbrBackground = NULL;
    wcex.lpszMenuName = NULL;
    wcex.lpszClassName = szWindowClass;
    wcex.hIconSm = LoadIcon(wcex.hInstance, IDI_APPLICATION);

    if (Global::dllInstanceHandle == nullptr)
    {
        Global::LogMessageW(L"Global::dllInstanceHandle is nullptr!");
    }

    if (GetClassInfoEx(Global::dllInstanceHandle, szWindowClass, &wcex))
    {
#ifdef FANY_DEBUG
        Global::LogMessageW(L"Class already registered!");
#endif
    }
    else
    {
        if (!RegisterClassEx(&wcex))
        {
            Global::LogMessageW(L"RegisterClassEx failed!");
        }
    }

    HWND hParent = GetForegroundWindow();
    HWND hwnd = CreateWindowEx(                                              //
        WS_EX_LAYERED | WS_EX_TOOLWINDOW | WS_EX_NOACTIVATE | WS_EX_TOPMOST, //
        szWindowClass,                                                       //
        L"fanycandidatewindow",                                              //
        WS_POPUP | WS_CHILDWINDOW,                                           //
        100,                                                                 //
        100,                                                                 //
        (108 + 15) * 1.5,                                                    //
        (246 + 15) * 1.5,                                                    //
        hParent,                                                             //
        nullptr,                                                             //
        wcex.hInstance,                                                      //
        nullptr                                                              //
    );                                                                       //

    if (!hwnd)
    {
        Global::LogMessageW(L"CreateWindow failed!");
    }

    MoveWindow(hwnd, 100, 100, (108 + 15) * 1.5, (246 + 15) * 1.5, TRUE);
    ShowWindow(hwnd, SW_SHOWNOACTIVATE);
    UpdateWindow(hwnd);
    InitWebview(hwnd);
    return hwnd;
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::ActivateEx
//
//----------------------------------------------------------------------------

STDAPI CSampleIME::ActivateEx(ITfThreadMgr *pThreadMgr, TfClientId tfClientId, DWORD dwFlags)
{
    _pThreadMgr = pThreadMgr;
    _pThreadMgr->AddRef();

    _tfClientId = tfClientId;
    _dwActivateFlags = dwFlags;

    if (!_InitThreadMgrEventSink())
    {
        goto ExitError;
    }

    ITfDocumentMgr *pDocMgrFocus = nullptr;
    if (SUCCEEDED(_pThreadMgr->GetFocus(&pDocMgrFocus)) && (pDocMgrFocus != nullptr))
    {
        _InitTextEditSink(pDocMgrFocus);
        pDocMgrFocus->Release();
    }

    if (!_InitKeyEventSink())
    {
        goto ExitError;
    }

    if (!_InitActiveLanguageProfileNotifySink())
    {
        goto ExitError;
    }

    if (!_InitThreadFocusSink())
    {
        goto ExitError;
    }

    if (!_InitDisplayAttributeGuidAtom())
    {
        goto ExitError;
    }

    if (!_InitFunctionProviderSink())
    {
        goto ExitError;
    }

    if (!_AddTextProcessorEngine())
    {
        goto ExitError;
    }

    // Reset to Chinese mode whenever switch back to this IME
    _pCompositionProcessorEngine->InitializeSampleIMECompartment(pThreadMgr, tfClientId);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_PER_MONITOR_AWARE_V2);

    if (Global::MainWindowHandle == nullptr)
    {
        Global::MainWindowHandle = CreateGlobalCandidateWindow();
    }
    return S_OK;

ExitError:
    Deactivate();
    return E_FAIL;
}

//+---------------------------------------------------------------------------
//
// ITfTextInputProcessorEx::Deactivate
//
//----------------------------------------------------------------------------

STDAPI CSampleIME::Deactivate()
{
    if (_pCompositionProcessorEngine)
    {
        delete _pCompositionProcessorEngine;
        _pCompositionProcessorEngine = nullptr;
    }

    ITfContext *pContext = _pContext;
    if (_pContext)
    {
        pContext->AddRef();
        _EndComposition(_pContext);
    }

    if (_pCandidateListUIPresenter)
    {
        delete _pCandidateListUIPresenter;
        _pCandidateListUIPresenter = nullptr;

        if (pContext)
        {
            pContext->Release();
        }

        _candidateMode = CANDIDATE_NONE;
        _isCandidateWithWildcard = FALSE;
    }

    _UninitFunctionProviderSink();

    _UninitThreadFocusSink();

    _UninitActiveLanguageProfileNotifySink();

    _UninitKeyEventSink();

    _UninitThreadMgrEventSink();

    CCompartment CompartmentKeyboardOpen(_pThreadMgr, _tfClientId, GUID_COMPARTMENT_KEYBOARD_OPENCLOSE);
    CompartmentKeyboardOpen._ClearCompartment();

    CCompartment CompartmentDoubleSingleByte(_pThreadMgr, _tfClientId,
                                             Global::SampleIMEGuidCompartmentDoubleSingleByte);
    CompartmentDoubleSingleByte._ClearCompartment();

    CCompartment CompartmentPunctuation(_pThreadMgr, _tfClientId, Global::SampleIMEGuidCompartmentPunctuation);
    CompartmentDoubleSingleByte._ClearCompartment();

    if (_pThreadMgr != nullptr)
    {
        _pThreadMgr->Release();
    }

    _tfClientId = TF_CLIENTID_NULL;

    if (_pDocMgrLastFocused)
    {
        _pDocMgrLastFocused->Release();
        _pDocMgrLastFocused = nullptr;
    }

    if (Global::MainWindowHandle != nullptr)
    {
        DestroyWindow(Global::MainWindowHandle);
        Global::MainWindowHandle = nullptr;
    }

    if (Global::UIWebview2Control != nullptr)
    {
        Global::UIWebview2Control->Close();
        Global::UIWebview2Control = nullptr;
    }

    if (Global::UIWebview2 != nullptr)
    {
        Global::UIWebview2 = nullptr;
    }

    return S_OK;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::GetType
//
//----------------------------------------------------------------------------
HRESULT CSampleIME::GetType(__RPC__out GUID *pguid)
{
    HRESULT hr = E_INVALIDARG;
    if (pguid)
    {
        *pguid = Global::SampleIMECLSID;
        hr = S_OK;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::::GetDescription
//
//----------------------------------------------------------------------------
HRESULT CSampleIME::GetDescription(__RPC__deref_out_opt BSTR *pbstrDesc)
{
    HRESULT hr = E_INVALIDARG;
    if (pbstrDesc != nullptr)
    {
        *pbstrDesc = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunctionProvider::::GetFunction
//
//----------------------------------------------------------------------------
HRESULT CSampleIME::GetFunction(__RPC__in REFGUID rguid, __RPC__in REFIID riid, __RPC__deref_out_opt IUnknown **ppunk)
{
    HRESULT hr = E_NOINTERFACE;

    if ((IsEqualGUID(rguid, GUID_NULL)) && (IsEqualGUID(riid, __uuidof(ITfFnSearchCandidateProvider))))
    {
        hr = _pITfFnSearchCandidateProvider->QueryInterface(riid, (void **)ppunk);
    }
    else if (IsEqualGUID(rguid, GUID_NULL))
    {
        hr = QueryInterface(riid, (void **)ppunk);
    }

    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFunction::GetDisplayName
//
//----------------------------------------------------------------------------
HRESULT CSampleIME::GetDisplayName(_Out_ BSTR *pbstrDisplayName)
{
    HRESULT hr = E_INVALIDARG;
    if (pbstrDisplayName != nullptr)
    {
        *pbstrDisplayName = nullptr;
        hr = E_NOTIMPL;
    }
    return hr;
}

//+---------------------------------------------------------------------------
//
// ITfFnGetPreferredTouchKeyboardLayout::GetLayout
// The tkblayout will be Optimized layout.
//----------------------------------------------------------------------------
HRESULT CSampleIME::GetLayout(_Out_ TKBLayoutType *ptkblayoutType, _Out_ WORD *pwPreferredLayoutId)
{
    HRESULT hr = E_INVALIDARG;
    if ((ptkblayoutType != nullptr) && (pwPreferredLayoutId != nullptr))
    {
        *ptkblayoutType = TKBLT_OPTIMIZED;
        *pwPreferredLayoutId = TKBL_OPT_SIMPLIFIED_CHINESE_PINYIN;
        hr = S_OK;
    }
    return hr;
}
