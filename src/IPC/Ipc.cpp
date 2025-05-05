#include "Ipc.h"
#include <debugapi.h>
#include <handleapi.h>
#include <minwindef.h>
#include <winnt.h>
#include <winuser.h>
#include "fmt/xchar.h"
#include "spdlog/spdlog.h"
#include "Globals.h"

static HANDLE hMapFile = nullptr;
static void *pBuf;
static FanyImeSharedMemoryData *sharedData;
static bool canUseSharedMemory = true;

static HANDLE hPipe = nullptr;
static bool canUseNamedPipe = true;

static FanyImeNamedpipeData namedpipeData;

int InitIpc()
{
    //
    // Shared memory, open here
    //
    hMapFile = OpenFileMappingW( //
        FILE_MAP_ALL_ACCESS,     //
        FALSE,                   //
        FANY_IME_SHARED_MEMORY   //
    );

    //
    // Shared memory is not available, try to use namedpipe
    //
    InitNamedpipe();

    if (!hMapFile)
    {
        OutputDebugString(L"CreateFileMapping failed\n");
        // Error handling
        canUseSharedMemory = false;
        spdlog::info("CreateFileMapping error: {}", GetLastError());

        return 0;
    }

    bool alreadyExists = (GetLastError() == ERROR_ALREADY_EXISTS);

    pBuf = MapViewOfFile(    //
        hMapFile,            //
        FILE_MAP_ALL_ACCESS, //
        0,                   //
        0,                   //
        BUFFER_SIZE          //
    );                       //

    if (!pBuf)
    {
        // Error handling
        spdlog::info("MapViewOfFile error when activated: {}", GetLastError());
    }

    sharedData = static_cast<FanyImeSharedMemoryData *>(pBuf);
    // Only initialize the shared memory when first created
    if (!alreadyExists)
    {
        // Initialize
        *sharedData = {};
        sharedData->point[0] = 100;
        sharedData->point[1] = 100;
    }

    return 0;
}

int InitNamedpipe()
{
    hPipe = CreateFile(               //
        FANY_IME_NAMED_PIPE,          //
        GENERIC_READ | GENERIC_WRITE, //
        0,                            //
        nullptr,                      //
        OPEN_EXISTING,                //
        0,                            //
        nullptr                       //
    );

    if (!hPipe)
    {
        canUseNamedPipe = false;
        OutputDebugString(L"Connect to namedpipe failed\n");
        return -1;
    }
    else
    {
    }

    return 0;
}

int CloseIpc()
{
    //
    // Namedpipe
    //
    CloseNamedpipe();

    if (!canUseSharedMemory)
    {
        return -1;
    }

    //
    // Shared memory
    //
    if (pBuf)
    {
        UnmapViewOfFile(pBuf);
        pBuf = nullptr;
    }

    if (hMapFile)
    {
        CloseHandle(hMapFile);
        hMapFile = nullptr;
    }

    //
    // Events
    //
    for (const auto &eventName : FANY_IME_EVENT_ARRAY)
    {
        HANDLE hEvent = OpenEventW( //
            EVENT_ALL_ACCESS,       //
            FALSE,                  //
            eventName.c_str()       //
        );                          //
        if (hEvent)
        {
            CloseHandle(hEvent);
        }
    }

    return 0;
}

int CloseNamedpipe()
{
    if (hPipe)
    {
        CloseHandle(hPipe);
        hPipe = nullptr;
    }
    return 0;
}

int WriteDataToSharedMemory(           //
    UINT keycode,                      //
    UINT modifiers_down,               //
    const int point[2],                //
    int pinyin_length,                 //
    const std::wstring &pinyin_string, //
    UINT write_flag                    //
)
{
    WriteDataToNamedPipe(keycode, modifiers_down, point, pinyin_length, pinyin_string, write_flag);
    if (!canUseSharedMemory)
    {
        if (canUseNamedPipe)
        {
            return WriteDataToNamedPipe(keycode, modifiers_down, point, pinyin_length, pinyin_string, write_flag);
        }
        return -1;
    }

    if (write_flag >> 0 & 1u)
    {
        sharedData->keycode = keycode;
    }

    if (write_flag >> 1 & 1u)
    {
        sharedData->modifiers_down = modifiers_down;
    }

    if (write_flag >> 2 & 1u)
    {
        sharedData->point[0] = point[0];
        sharedData->point[1] = point[1];
    }

    if (write_flag >> 3 & 1u)
    {
        sharedData->pinyin_length = pinyin_length;
    }

    if (write_flag >> 4 & 1u)
    {
        wcscpy_s(sharedData->pinyin_string, pinyin_string.c_str());
        sharedData->pinyin_string[pinyin_length] = L'\0';
    }

    return 0;
}

/**
 * @brief
 *
 * @param keycode
 * @param modifiers_down
 * @param point
 * @param pinyin_length
 * @param pinyin_string
 * @param write_flag
 * @return int
 */
int WriteDataToNamedPipe(              //
    UINT keycode,                      //
    UINT modifiers_down,               //
    const int point[2],                //
    int pinyin_length,                 //
    const std::wstring &pinyin_string, //
    UINT write_flag                    //
)
{
    if (!canUseNamedPipe)
    {
        return -1;
    }

    if (write_flag >> 0 & 1u)
    {
        namedpipeData.keycode = keycode;
    }

    if (write_flag >> 1 & 1u)
    {
        namedpipeData.modifiers_down = modifiers_down;
    }

    if (write_flag >> 2 & 1u)
    {
        namedpipeData.point[0] = point[0];
        namedpipeData.point[1] = point[1];
    }

    if (write_flag >> 3 & 1u)
    {
        namedpipeData.pinyin_length = pinyin_length;
    }

    if (write_flag >> 4 & 1u)
    {
        wcscpy_s(namedpipeData.pinyin_string, pinyin_string.c_str());
        namedpipeData.pinyin_string[pinyin_length] = L'\0';
    }

    return 0;
}

int SendKeyEventToUIProcess()
{
    // OutputDebugString(fmt::format(L"SendKeyEventToUIProcess, canUseSharedMemory = {}, canUseNamedPipe = {}\n",
    //                               canUseSharedMemory, canUseNamedPipe)
    //                       .c_str());
    if (!canUseSharedMemory)
    {
        // OutputDebugString(L"SendKeyEventToUIProcessViaNamedPipe\n");
        if (canUseNamedPipe)
        {
            // OutputDebugString(L"Really SendKeyEventToUIProcessViaNamedPipe \n");
            return SendKeyEventToUIProcessViaNamedPipe();
        }
    }

    HANDLE hEvent = OpenEventW(         //
        EVENT_MODIFY_STATE,             //
        FALSE,                          //
        FANY_IME_EVENT_ARRAY[0].c_str() //
    );                                  //

    if (!hEvent)
    {
        // TODO: Error handling
    }

    if (!SetEvent(hEvent))
    {
        // TODO: Error handling
        DWORD err = GetLastError();
        spdlog::info("SetEvent error: {}", err);
    }

    CloseHandle(hEvent);
    return 0;
}

int SendHideCandidateWndEventToUIProcess()
{
    if (!canUseSharedMemory)
    {
        if (canUseNamedPipe)
        {
            return SendHideCandidateWndEventToUIProcessViaNamedPipe();
        }
    }

    HANDLE hEvent = OpenEventW(         //
        EVENT_MODIFY_STATE,             //
        FALSE,                          //
        FANY_IME_EVENT_ARRAY[1].c_str() // FanyHideCandidateWndEvent
    );                                  //

    if (!hEvent)
    {
        // TODO: Error handling
    }

    if (!SetEvent(hEvent))
    {
        // TODO: Error handling
        DWORD err = GetLastError();
        spdlog::info("SetEvent error: {}", err);
    }

    CloseHandle(hEvent);
    return 0;
}

int SendShowCandidateWndEventToUIProcess()
{
    if (!canUseSharedMemory)
    {
        if (canUseNamedPipe)
        {
            return SendShowCandidateWndEventToUIProcessViaNamedPipe();
        }
    }

    HANDLE hEvent = OpenEventW(         //
        EVENT_MODIFY_STATE,             //
        FALSE,                          //
        FANY_IME_EVENT_ARRAY[2].c_str() // FanyShowCandidateWndEvent
    );                                  //

    if (!hEvent)
    {
        // TODO: Error handling
    }

    if (!SetEvent(hEvent))
    {
        // TODO: Error handling
        DWORD err = GetLastError();
        spdlog::info("SetEvent error: {}", err);
    }

    CloseHandle(hEvent);
    return 0;
}

int SendMoveCandidateWndEventToUIProcess()
{
    if (!canUseSharedMemory)
    {
        if (canUseNamedPipe)
        {
            return SendMoveCandidateWndEventToUIProcessViaNamedPipe();
        }
    }

    HANDLE hEvent = OpenEventW(         //
        EVENT_MODIFY_STATE,             //
        FALSE,                          //
        FANY_IME_EVENT_ARRAY[3].c_str() // FanyMoveCandidateWndEvent
    );                                  //

    if (!hEvent)
    {
        // TODO: Error handling
        spdlog::info("Open FanyMoveCandidateWnd Event error");
    }

    if (!SetEvent(hEvent))
    {
        // TODO: Error handling
        DWORD err = GetLastError();
        spdlog::info("SetEvent error: {}", err);
    }

    CloseHandle(hEvent);
    return 0;
}

//
// Named pipe
//

void SendToNamedpipe()
{
    if (!canUseNamedPipe)
    {
        return;
    }
    if (!hPipe)
    {
        return;
    }
    DWORD bytesWritten = 0;
    BOOL ret = WriteFile(      //
        hPipe,                 //
        &namedpipeData,        //
        sizeof(namedpipeData), //
        &bytesWritten,         //
        NULL                   //
    );
    if (!ret || bytesWritten != sizeof(namedpipeData))
    {
        // TODO: Error handling
        DWORD err = GetLastError();
        spdlog::info("WriteFile error: {}", err);
    }
}

/**
 * event_type
 *   0: FanyImeKeyEvent
 *   1: FanyHideCandidateWndEvent
 *   2: FanyShowCandidateWndEvent
 *   3: FanyMoveCandidateWndEvent
 */
int SendKeyEventToUIProcessViaNamedPipe()
{
    // OutputDebugString(L"first SendKeyEventToUIProcessViaNamedPipe\n");
    if (!canUseNamedPipe)
    {
        return -1;
    }

    namedpipeData.event_type = 0;
    SendToNamedpipe();

    return 0;
}

int SendHideCandidateWndEventToUIProcessViaNamedPipe()
{
    if (!canUseNamedPipe)
    {
        return -1;
    }

    namedpipeData.event_type = 1;
    SendToNamedpipe();

    return 0;
}

int SendShowCandidateWndEventToUIProcessViaNamedPipe()
{
    if (!canUseNamedPipe)
    {
        return -1;
    }

    namedpipeData.event_type = 2;
    SendToNamedpipe();

    return 0;
}

int SendMoveCandidateWndEventToUIProcessViaNamedPipe()
{
    if (!canUseNamedPipe)
    {
        return -1;
    }

    namedpipeData.event_type = 3;
    SendToNamedpipe();

    return 0;
}