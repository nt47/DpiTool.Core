#include"pch.h"
#include"misc.h"
#include<iostream>

bool ShareMemory(PSHARED_DATA new_shared_data)
{
    // 打开共享内存
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_READ,      // 读取访问权限
        FALSE,              // 不继承句柄
        L"Injector.SharedMemory"); // 共享内存名称

    if (hMapFile == NULL)
    {
        std::cout << "Could not open file mapping object (" << GetLastError() << ")." << std::endl;
        return false;
    }

    // 将共享内存映射到进程的地址空间
    PSHARED_DATA shared_data = (PSHARED_DATA)MapViewOfFile(
        hMapFile,           // 共享内存句柄
        FILE_MAP_READ,      // 读取访问权限
        0,
        0,
        sizeof(SHARED_DATA));

    if (shared_data == NULL)
    {
        std::cout << "Could not map view of file (" << GetLastError() << ")." << std::endl;
        CloseHandle(hMapFile);
        return false;
    }

    // 在共享内存中读取数据

    //MessageBox(shared_data->exe_folder);
    //MessageBox(shared_data->dll_folder);

    memcpy(new_shared_data, shared_data, sizeof(SHARED_DATA)); // 使用 memcpy 进行深拷贝
    


    // 打开一个事件，用于通知另一个进程数据已读取
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"Injector.Event001");
    if (hEvent == NULL)
    {
        std::cout << "Could not open event object (" << GetLastError() << ")." << std::endl;
        UnmapViewOfFile(shared_data);
        CloseHandle(hMapFile);
        return false;
    }

    if (!SetEvent(hEvent)) {//// 通知另一个进程数据已读取
        std::cout << "Failed to set event. Error code: " << GetLastError() << std::endl;
        CloseHandle(hEvent);
        UnmapViewOfFile(shared_data);
        CloseHandle(hMapFile);
        return false;
    }

    // 清理资源
    CloseHandle(hEvent);
    UnmapViewOfFile(shared_data);
    CloseHandle(hMapFile);

    return true;
}