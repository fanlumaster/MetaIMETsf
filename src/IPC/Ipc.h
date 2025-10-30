#pragma once
#include <Windows.h>
#include <string>
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
    WCHAR wch;
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
//   4: FanyLangbarRightClickEvent
//
struct FanyImeNamedpipeData
{
    UINT event_type;
    UINT keycode;
    WCHAR wch;
    UINT modifiers_down = 0;
    int point[2] = {100, 100};
    int pinyin_length = 0;
    wchar_t pinyin_string[128];
};

//
// Data received from server end
//
// msg_type
//   0: success
//   1: candidate index out of range error
//
struct FanyImeNamedpipeDataToTsf
{
    UINT msg_type;
    wchar_t candidate_string[200]; // 200 chars at most
};

int InitIpc();
int InitNamedpipe();
int CloseIpc();
int CloseNamedpipe();

//
// For shared memory
//
int WriteDataToSharedMemory(           //
    UINT keycode,                      // VkCode
    WCHAR wch,                         // Unicode character converted from vkcode
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
int SendLangbarRightClickEventToUIProcess(const RECT *prcArea);

//
// For named pipe
//
int WriteDataToNamedPipe(              //
    UINT keycode,                      //
    WCHAR wch,                         //
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
int SendLangbarRightClickEventToUIProcessViaNamedPipe(const RECT *prcArea);
void ClearNamedpipeDataIfExists();
struct FanyImeNamedpipeDataToTsf *TryReadDataFromServerPipeWithTimeout();
struct FanyImeNamedpipeDataToTsf *ReadDataFromServerViaNamedPipe();

//
// Modifiers:
//     0b00000001: Shift
//     0b00000010: Control
//     0b00000100: Alt
// TODO: Make it able to denote explicit modifiers, e.g. LShift, RShift, we could use left keys
//
namespace Global
{
inline UINT Keycode = 0;
inline WCHAR wch = L'\0';
inline UINT ModifiersDown = 0;
inline int Point[2] = {100, 100};
inline int PinyinLength = 0;
inline std::wstring PinyinString = L"";

inline int firefox_like_cnt = 0; // Apps like firefox, e.g. firefox, zen...
inline std::wstring current_process_name = L"";

inline wchar_t app_name[512] = {0};

namespace DataFromServerMsgType
{
constexpr UINT Normal = 0;
constexpr UINT OutofRange = 1;
constexpr UINT NeedToCreateWord = 2;
constexpr UINT Preedit = 3;
} // namespace DataFromServerMsgType
} // namespace Global