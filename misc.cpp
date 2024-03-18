#include"pch.h"
#include"misc.h"
#include<iostream>

bool ShareMemory(PSHARED_DATA new_shared_data)
{
    // �򿪹����ڴ�
    HANDLE hMapFile = OpenFileMapping(
        FILE_MAP_READ,      // ��ȡ����Ȩ��
        FALSE,              // ���̳о��
        L"Injector.SharedMemory"); // �����ڴ�����

    if (hMapFile == NULL)
    {
        std::cout << "Could not open file mapping object (" << GetLastError() << ")." << std::endl;
        return false;
    }

    // �������ڴ�ӳ�䵽���̵ĵ�ַ�ռ�
    PSHARED_DATA shared_data = (PSHARED_DATA)MapViewOfFile(
        hMapFile,           // �����ڴ���
        FILE_MAP_READ,      // ��ȡ����Ȩ��
        0,
        0,
        sizeof(SHARED_DATA));

    if (shared_data == NULL)
    {
        std::cout << "Could not map view of file (" << GetLastError() << ")." << std::endl;
        CloseHandle(hMapFile);
        return false;
    }

    // �ڹ����ڴ��ж�ȡ����

    //MessageBox(shared_data->exe_folder);
    //MessageBox(shared_data->dll_folder);

    memcpy(new_shared_data, shared_data, sizeof(SHARED_DATA)); // ʹ�� memcpy �������
    


    // ��һ���¼�������֪ͨ��һ�����������Ѷ�ȡ
    HANDLE hEvent = OpenEvent(EVENT_MODIFY_STATE, FALSE, L"Injector.Event001");
    if (hEvent == NULL)
    {
        std::cout << "Could not open event object (" << GetLastError() << ")." << std::endl;
        UnmapViewOfFile(shared_data);
        CloseHandle(hMapFile);
        return false;
    }

    if (!SetEvent(hEvent)) {//// ֪ͨ��һ�����������Ѷ�ȡ
        std::cout << "Failed to set event. Error code: " << GetLastError() << std::endl;
        CloseHandle(hEvent);
        UnmapViewOfFile(shared_data);
        CloseHandle(hMapFile);
        return false;
    }

    // ������Դ
    CloseHandle(hEvent);
    UnmapViewOfFile(shared_data);
    CloseHandle(hMapFile);

    return true;
}