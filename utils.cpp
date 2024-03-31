#include "pch.h"
#include"utils.h"


std::wstring fmt(const wchar_t* format, ...) {
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, bufferSize, format, args);
    va_end(args);
    return std::wstring(buffer);
}


void MessageBox(const wchar_t* format, ...)
{
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, bufferSize, format, args);
    va_end(args);

    MessageBoxW(NULL, buffer, L"Formatted MessageBox", MB_OK | MB_ICONINFORMATION);
}


void Console()
{
    wchar_t title[56];
    swprintf_s(title, L"Debug Window - %d", GetCurrentProcessId());

    setlocale(LC_ALL, "chs");
    AllocConsole();
    SetConsoleTitle(title);
    //freopen_s("CON", "w", stdout);
    FILE* consoleOutput;
    freopen_s(&consoleOutput, "CONOUT$", "w", stdout);
}




std::wstring GetCurrentProcessName() {
    // ��ȡ��ǰ���̾��
    HANDLE hProcess = GetCurrentProcess();

    // ��ȡ��ǰ����ģ����
    HMODULE hModule;
    DWORD dwNeeded;
    if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwNeeded)) {
        // ��ȡ����ģ���ļ���
        TCHAR szProcessPath[MAX_PATH];
        GetModuleFileNameEx(hProcess, hModule, szProcessPath, sizeof(szProcessPath) / sizeof(TCHAR));

        // ʹ�� PathFindFileName ��ȡ�ļ�������

        std::wstring szProcessName = PathFindFileName(szProcessPath);

        // �رս��̾��
        CloseHandle(hProcess);

        return szProcessName;
    }
    else {
        //std::cout << "Error getting process modules." << std::endl;
        // �رս��̾��
        CloseHandle(hProcess);
        return L"";
    }
}



// ��¼ Unicode ��־�ļ򵥺���
void Logfile(const std::wstring& message, const std::wstring& filename)
{

    // ��ȡ��ǰʱ��
    std::time_t currentTime;
    std::time(&currentTime);

    // ʹ�� localtime_s ��� localtime
    std::tm localTime = {};
    localtime_s(&localTime, &currentTime);

    // ��ʽ��ʱ��
    wchar_t timeBuffer[80];
    std::wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", &localTime);

    // ����־�ļ�����׷�ӷ�ʽд��

                // ��ȡDLL·��
    TCHAR dllPath[MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), dllPath, MAX_PATH);

    // ȥ���ļ����ͺ�׺��ֻ����Ŀ¼
    PathRemoveFileSpec(dllPath);

    std::wstring fullFilename = std::wstring(dllPath) + L"\\" + GetCurrentProcessName() + L".log";

    std::wofstream logFile(fullFilename, std::ios::app);

    if (logFile.is_open())
    {
        logFile.imbue(std::locale("", std::locale::all ^ std::locale::numeric));//֧������
        // �����ʽ������־��Ϣ������̨���ļ�
        std::wcout << L"[" << timeBuffer << L"] " << message << std::endl;
        logFile << L"[" << timeBuffer << L"] " << message << std::endl;

        // �ر���־�ļ�
        logFile.close();
    }
    else
    {
        MessageBox(NULL, L"����־�ļ�ʧ��!", L"Error", MB_ICONERROR);
    }

}


void Log(const wchar_t* format, ...)
{
    const int bufferSize = 1024;
    wchar_t buffer[bufferSize];

    va_list args;
    va_start(args, format);
    vswprintf_s(buffer, bufferSize, format, args);
    va_end(args);


    Logfile(buffer);
}

// ��ȡ���һ�δ������Ϣ
std::wstring GetLastErrorAsString() {
    DWORD errorCode = GetLastError();
    LPVOID errorMsgBuffer = nullptr;

    DWORD result = FormatMessage(
        FORMAT_MESSAGE_ALLOCATE_BUFFER | FORMAT_MESSAGE_FROM_SYSTEM,
        NULL,
        errorCode,
        0,
        (LPWSTR)&errorMsgBuffer,
        0,
        NULL
    );

    std::wstring errorMessage;

    if (result != 0) {
        // ����������Ϣ�ַ���
        errorMessage = L"Error Code: " + std::to_wstring(errorCode) + L"\n";
        errorMessage += L"Error Message: " + std::wstring(static_cast<LPCWSTR>(errorMsgBuffer)) + L"\n";

        // �ͷŷ���Ļ�����
        LocalFree(errorMsgBuffer);
    }
    else {
        errorMessage = L"Failed to get error message.\n";
    }

    return errorMessage;
}


bool IsModuleLoaded(HMODULE hModule) {

    MEMORY_BASIC_INFORMATION mbi;

    // ���ģ���Ƿ���Ч
    if (!hModule) {
        std::cout << "EnumProcessModules failed." << std::endl;
        return false;
    }

    // ���ģ��� PE ͷ
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cout << "pDosHeader->e_magic != IMAGE_DOS_SIGNATURE." << std::endl;
        return false;
    }

    // ���ģ��� NT ͷ
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);//��Ŀ�ΪʲôҪǿ��8�ֽڶ��룿���ֽڶ���Ҳ�У���֮���ڴ�����й�
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cout << "pNtHeaders->Signature != IMAGE_NT_SIGNATURE." << std::endl;
        return false;
    }

    // ���ģ��ĸ�����
    for (DWORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((BYTE*)pNtHeaders + sizeof(IMAGE_NT_HEADERS) +
            i * sizeof(IMAGE_SECTION_HEADER));
        if (pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            // ����ڿ�ִ�У�������Ƿ���ص��ڴ���
            if (!VirtualQuery((LPVOID)pSectionHeader->VirtualAddress, &mbi, sizeof(mbi))) {
                std::wcout << "VirtualQuery failed." << GetLastErrorAsString().c_str() <<std::endl;
                return false;
            }
        }
    }

    // ģ��������
    return true;
}

bool IsAllModulesLoadedEx() {
    HANDLE hProcess = GetCurrentProcess();
    // ��ȡ��ǰ���̵�����ģ��
    DWORD cbNeeded;
    HMODULE hModules[1024];
    if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        std::cout << "EnumProcessModules failed." << std::endl;
        return false;
    }

    // ����ģ������
    int moduleCount = cbNeeded / sizeof(HMODULE);

    // ��������ģ�飬����Ƿ�������
    for (DWORD i = 0; i < moduleCount; i++) {
        if (!IsModuleLoaded(hModules[i])) {
            std::cout << "IsModuleLoaded failed." << std::endl;
            return false;
        }
    }

    // ����ģ�鶼�������
    return true;
}


bool IsAllModulesLoaded() {
    HANDLE hProcess = GetCurrentProcess();
    if (hProcess == NULL) {
        std::cout << "Failed to get current process handle." << std::endl;
        return false;
    }

    std::vector<HMODULE> modules(1024);
    DWORD cbNeeded;
    int previousModuleCount = 0;
    int sameCount = 0;

local_1:

    while (1)
    {
        if (IsAllModulesLoadedEx())
            break;

        Sleep(100);
    }

    std::cout << "Check for AreAllModulesLoadedEx!" << std::endl;

    for (int attempt = 0; attempt < 5; ++attempt) {


        if (!EnumProcessModules(hProcess, modules.data(), modules.size(), &cbNeeded)) {
            std::cout << "Failed to enumerate process modules." << std::endl;
            return false;
        }

        int moduleCount = cbNeeded / sizeof(HMODULE);
        if (previousModuleCount == 0)
            previousModuleCount = moduleCount;

        if (moduleCount == previousModuleCount) {
            ++sameCount;
            if (sameCount == 5) {
                std::cout << "All Modules Loaded!" << std::endl;
                return true;
            }
        }
        else {
            previousModuleCount = 0;
            sameCount = 0;
            goto local_1;
        }

        previousModuleCount = moduleCount;

        Sleep(100);
    }

    return false;
}

void simulateKeyPress(WORD keyCode) {
    // Simulate key press
    keybd_event(keyCode, 0, 0, 0);
    // Simulate key release
    keybd_event(keyCode, 0, KEYEVENTF_KEYUP, 0);
}

// ȥ���ַ�����ͷ�Ŀհ��ַ�
std::wstring ltrim(const std::wstring& s) {
    size_t start = 0;
    while (start < s.length() && std::isspace(s[start])) {
        ++start;
    }
    return s.substr(start);
}

// ȥ���ַ�����β�Ŀհ��ַ�
std::wstring rtrim(const std::wstring& s) {
    size_t end = s.length();
    while (end > 0 && std::isspace(s[end - 1])) {
        --end;
    }
    return s.substr(0, end);
}

// ȥ���ַ������˵Ŀհ��ַ�
std::wstring trim(const std::wstring& s) {
    return rtrim(ltrim(s));
}
