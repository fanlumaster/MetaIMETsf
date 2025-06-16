#pragma once
#include <string>
#include <windows.h>
#include <vector>

inline const wchar_t *FANY_IME_SHARED_MEMORY = L"Local\\FanyImeSharedMemory";
inline const int BUFFER_SIZE = 4096;

inline const wchar_t *FANY_IME_NAMED_PIPE = L"\\\\.\\pipe\\FanyImeNamedPipe";
inline const wchar_t *FANY_IME_TO_TSF_NAMED_PIPE = L"\\\\.\\pipe\\FanyImeToTsfNamedPipe";
inline const wchar_t *FANY_IME_AUX_NAMED_PIPE = L"\\\\.\\pipe\\FanyImeAuxNamedPipe";

inline const std::vector<std::wstring> FANY_IME_EVENT_ARRAY = {
    L"FanyImeKeyEvent",           // Event sent to UI process to notify time to update UI by new pinyin_string
    L"FanyHideCandidateWndEvent", // Event sent to UI process to notify time to hide candidate window
    L"FanyShowCandidateWndEvent", // Event sent to UI process to notify time to show candidate window
    L"FanyMoveCandidateWndEvent", // Event sent to UI process to notify time to move candidate window
};

//
// modifiers:
//   0: non
//   1: shift
//   2: control
//   3: alt
//   4: win
//   5: to be supplemented
//
struct FanyImeSharedMemoryData
{
    UINT keycode;
    UINT modifiers_down = 0;
    int point[2] = {100, 100};
    int pinyin_length = 0;
    wchar_t pinyin_string[128];
    wchar_t candidate_string[1024];
    wchar_t selected_candiate_string[128];
};

//
// For uwp/metro apps, here we do not need candidate_string and selected_candiate_string,
// just let server process to handle them
//
// event_type
//   0: FanyImeKeyEvent
//   1: FanyHideCandidateWndEvent
//   2: FanyShowCandidateWndEvent
//   3: FanyMoveCandidateWndEvent
//
struct FanyImeNamedpipeData
{
    UINT event_type;
    UINT keycode;
    UINT modifiers_down = 0;
    int point[2] = {100, 100};
    int pinyin_length = 0;
    wchar_t pinyin_string[128];
};

int InitIpc();
int InitNamedpipe();
int CloseIpc();
int CloseNamedpipe();

//
// For shared memory
//
int WriteDataToSharedMemory(           //
    UINT keycode,                      //
    UINT modifiers_down,               //
    const int point[2],                //
    int pinyin_length,                 //
    const std::wstring &pinyin_string, //
    UINT write_flag                    //
);
int SendKeyEventToUIProcess();
int SendHideCandidateWndEventToUIProcess();
int SendShowCandidateWndEventToUIProcess();
int SendMoveCandidateWndEventToUIProcess();

//
// For named pipe
//
int WriteDataToNamedPipe(              //
    UINT keycode,                      //
    UINT modifiers_down,               //
    const int point[2],                //
    int pinyin_length,                 //
    const std::wstring &pinyin_string, //
    UINT write_flag                    //
);
int SendKeyEventToUIProcessViaNamedPipe();
int SendHideCandidateWndEventToUIProcessViaNamedPipe();
int SendShowCandidateWndEventToUIProcessViaNamedPipe();
int SendMoveCandidateWndEventToUIProcessViaNamedPipe();
std::wstring ReadDataFromServerViaNamedPipe();

//
// Modifiers:
//     0b00000001: Shift
//     0b00000010: Control
//     0b00000100: Alt
//
namespace Global
{
inline UINT Keycode = 0;
inline UINT ModifiersDown = 0;
inline int Point[2] = {100, 100};
inline int PinyinLength = 0;
inline std::wstring PinyinString = L"";

inline int firefox_like_cnt = 0; // Apps like firefox, e.g. firefox, zen...
inline std::wstring current_process_name = L"";

inline wchar_t app_name[512] = {0};
} // namespace Global