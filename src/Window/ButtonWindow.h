#pragma once

#include "BaseWindow.h"

class CButtonWindow : public CBaseWindow
{
  public:
    CButtonWindow();
    virtual ~CButtonWindow();

    LRESULT CALLBACK _WindowProcCallback(_In_ HWND wndHandle, UINT uMsg, _In_ WPARAM wParam, _In_ LPARAM lParam);

    virtual void _OnPaint(_In_ HDC dcHandle, _In_ PAINTSTRUCT *pps);
    virtual void _OnLButtonDown(POINT pt);
    virtual void _OnLButtonUp(POINT pt);

  protected:
    UINT typeOfControl; // DFCS_PUSHED, DFCS_INACTIVE or others for DrawFrameControl
};
