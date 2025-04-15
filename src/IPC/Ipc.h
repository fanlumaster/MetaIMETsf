#pragma once
#include <Windows.h>
#include <string>
#include <vector>

inline const wchar_t *FANY_IME_SHARED_MEMORY = L"Local\\FanyImeSharedMemory";
inline const int BUFFER_SIZE = 4096;

inline const std::vector<std::wstring> FANY_IME_EVENT_ARRAY = {
    L"FanyImeKeyEvent" // Event sent to UI process to notify time to update UI by new pinyin_string
};

struct FanyImeSharedMemoryData
{
    UINT keycode;
    UINT modifiers = 0;
    int point[2] = {100, 100};
    int pinyin_length = 0;
    wchar_t pinyin_string[128];
    wchar_t candidate_string[1024];
    wchar_t selected_candiate_string[128];
};

int InitIpc();
int CloseIpc();
int WriteDataToSharedMemory(           //
    UINT keycode,                      //
    UINT modifiers,                    //
    const int point[2],                //
    int pinyin_length,                 //
    const std::wstring &pinyin_string, //
    UINT write_flag                    //
);