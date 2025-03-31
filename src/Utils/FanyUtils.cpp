#include "FanyUtils.h"
#include "Define.h"
#include "Globals.h"

namespace FanyUtuils
{
std::string GetIMEDataDirPath()
{
    const char *localAppDataPath = std::getenv("LOCALAPPDATA");
    std::string IMEDataPath = std::string(localAppDataPath) + "\\" + Global::wstring_to_string(std::wstring(IME_NAME));
    return IMEDataPath;
}

std::string GetLogFilePath()
{
    const char *localAppDataPath = std::getenv("LOCALAPPDATA");
    std::string logPath = GetIMEDataDirPath() + "\\log\\" + FANYLOGFILE_;
    return logPath;
}

std::wstring GetLogFilePathW()
{
    std::wstring logPathW = Global::string_to_wstring(GetIMEDataDirPath()) + L"\\log\\" + FANYLOGFILE;
    return logPathW;
}

void SendUnicode(const wchar_t data)
{
    INPUT input[4];
    HWND current_hwnd = GetForegroundWindow();
    SetFocus(current_hwnd);

    input[0].type = INPUT_KEYBOARD;
    input[0].ki.wVk = 0;
    input[0].ki.wScan = data;
    input[0].ki.dwFlags = KEYEVENTF_UNICODE;
    input[0].ki.time = 0;
    input[0].ki.dwExtraInfo = GetMessageExtraInfo();
    SendInput(1, &input[0], sizeof(INPUT));

    input[1].type = INPUT_KEYBOARD;
    input[1].ki.wVk = 0;
    input[1].ki.wScan = data;
    input[1].ki.dwFlags = KEYEVENTF_UNICODE | KEYEVENTF_KEYUP;
    input[1].ki.time = 0;
    input[1].ki.dwExtraInfo = GetMessageExtraInfo();
    SendInput(1, &input[1], sizeof(INPUT));
}

void SendKeys(std::wstring pinyin)
{
    for (wchar_t ch : pinyin)
    {
        SendUnicode(ch);
    }
}

} // namespace FanyUtuils