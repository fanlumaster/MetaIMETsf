#include "Ipc.h"

static HANDLE hMapFile;
static void *pBuf;

int InitIpc()
{
    //
    // Shared memory
    //
    hMapFile = CreateFileMappingW(       //
        INVALID_HANDLE_VALUE,            //
        nullptr,                         //
        PAGE_READWRITE,                  //
        0,                               //
        static_cast<DWORD>(BUFFER_SIZE), //
        FANY_IME_SHARED_MEMORY           //
    );                                   //

    if (!hMapFile)
    {
        // Error handling
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
    }

    // Only initialize the shared memory when first created
    if (!alreadyExists)
    {
        FanyImeSharedMemoryData *sharedData = static_cast<FanyImeSharedMemoryData *>(pBuf);

        // Initialize
        *sharedData = {};
        sharedData->point[0] = 100;
        sharedData->point[1] = 100;
    }

    //
    // Events
    //
    for (const auto &eventName : FANY_IME_EVENT_ARRAY)
    {
        HANDLE hEvent = CreateEventW( //
            nullptr,                  //
            FALSE,                    //
            FALSE,                    // Auto reset
            eventName.c_str()         //
        );                            //
        if (!hEvent)
        {
            // Error handling
        }
    }

    return 0;
}

int CloseIpc()
{
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