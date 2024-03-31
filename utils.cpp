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
    // 获取当前进程句柄
    HANDLE hProcess = GetCurrentProcess();

    // 获取当前进程模块句柄
    HMODULE hModule;
    DWORD dwNeeded;
    if (EnumProcessModules(hProcess, &hModule, sizeof(hModule), &dwNeeded)) {
        // 获取进程模块文件名
        TCHAR szProcessPath[MAX_PATH];
        GetModuleFileNameEx(hProcess, hModule, szProcessPath, sizeof(szProcessPath) / sizeof(TCHAR));

        // 使用 PathFindFileName 提取文件名部分

        std::wstring szProcessName = PathFindFileName(szProcessPath);

        // 关闭进程句柄
        CloseHandle(hProcess);

        return szProcessName;
    }
    else {
        //std::cout << "Error getting process modules." << std::endl;
        // 关闭进程句柄
        CloseHandle(hProcess);
        return L"";
    }
}



// 记录 Unicode 日志的简单函数
void Logfile(const std::wstring& message, const std::wstring& filename)
{

    // 获取当前时间
    std::time_t currentTime;
    std::time(&currentTime);

    // 使用 localtime_s 替代 localtime
    std::tm localTime = {};
    localtime_s(&localTime, &currentTime);

    // 格式化时间
    wchar_t timeBuffer[80];
    std::wcsftime(timeBuffer, sizeof(timeBuffer) / sizeof(wchar_t), L"%Y-%m-%d %H:%M:%S", &localTime);

    // 打开日志文件，以追加方式写入

                // 获取DLL路径
    TCHAR dllPath[MAX_PATH];
    GetModuleFileName(GetModuleHandle(NULL), dllPath, MAX_PATH);

    // 去掉文件名和后缀，只留下目录
    PathRemoveFileSpec(dllPath);

    std::wstring fullFilename = std::wstring(dllPath) + L"\\" + GetCurrentProcessName() + L".log";

    std::wofstream logFile(fullFilename, std::ios::app);

    if (logFile.is_open())
    {
        logFile.imbue(std::locale("", std::locale::all ^ std::locale::numeric));//支持中文
        // 输出格式化的日志信息到控制台和文件
        std::wcout << L"[" << timeBuffer << L"] " << message << std::endl;
        logFile << L"[" << timeBuffer << L"] " << message << std::endl;

        // 关闭日志文件
        logFile.close();
    }
    else
    {
        MessageBox(NULL, L"打开日志文件失败!", L"Error", MB_ICONERROR);
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

// 获取最后一次错误的信息
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
        // 构建错误信息字符串
        errorMessage = L"Error Code: " + std::to_wstring(errorCode) + L"\n";
        errorMessage += L"Error Message: " + std::wstring(static_cast<LPCWSTR>(errorMsgBuffer)) + L"\n";

        // 释放分配的缓冲区
        LocalFree(errorMsgBuffer);
    }
    else {
        errorMessage = L"Failed to get error message.\n";
    }

    return errorMessage;
}


bool IsModuleLoaded(HMODULE hModule) {

    MEMORY_BASIC_INFORMATION mbi;

    // 检查模块是否有效
    if (!hModule) {
        std::cout << "EnumProcessModules failed." << std::endl;
        return false;
    }

    // 检查模块的 PE 头
    PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hModule;
    if (pDosHeader->e_magic != IMAGE_DOS_SIGNATURE) {
        std::cout << "pDosHeader->e_magic != IMAGE_DOS_SIGNATURE." << std::endl;
        return false;
    }

    // 检查模块的 NT 头
    PIMAGE_NT_HEADERS pNtHeaders = (PIMAGE_NT_HEADERS)((BYTE*)pDosHeader + pDosHeader->e_lfanew);//真的坑为什么要强制8字节对齐？单字节对齐也行，总之和内存对齐有关
    if (pNtHeaders->Signature != IMAGE_NT_SIGNATURE) {
        std::cout << "pNtHeaders->Signature != IMAGE_NT_SIGNATURE." << std::endl;
        return false;
    }

    // 检查模块的各个节
    for (DWORD i = 0; i < pNtHeaders->FileHeader.NumberOfSections; i++) {
        PIMAGE_SECTION_HEADER pSectionHeader = (PIMAGE_SECTION_HEADER)((BYTE*)pNtHeaders + sizeof(IMAGE_NT_HEADERS) +
            i * sizeof(IMAGE_SECTION_HEADER));
        if (pSectionHeader->Characteristics & IMAGE_SCN_MEM_EXECUTE) {
            // 如果节可执行，则检查节是否加载到内存中
            if (!VirtualQuery((LPVOID)pSectionHeader->VirtualAddress, &mbi, sizeof(mbi))) {
                std::wcout << "VirtualQuery failed." << GetLastErrorAsString().c_str() <<std::endl;
                return false;
            }
        }
    }

    // 模块加载完毕
    return true;
}

bool IsAllModulesLoadedEx() {
    HANDLE hProcess = GetCurrentProcess();
    // 获取当前进程的所有模块
    DWORD cbNeeded;
    HMODULE hModules[1024];
    if (!EnumProcessModules(hProcess, hModules, sizeof(hModules), &cbNeeded)) {
        std::cout << "EnumProcessModules failed." << std::endl;
        return false;
    }

    // 计算模块数量
    int moduleCount = cbNeeded / sizeof(HMODULE);

    // 遍历所有模块，检查是否加载完毕
    for (DWORD i = 0; i < moduleCount; i++) {
        if (!IsModuleLoaded(hModules[i])) {
            std::cout << "IsModuleLoaded failed." << std::endl;
            return false;
        }
    }

    // 所有模块都加载完毕
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

// 去除字符串开头的空白字符
std::wstring ltrim(const std::wstring& s) {
    size_t start = 0;
    while (start < s.length() && std::isspace(s[start])) {
        ++start;
    }
    return s.substr(start);
}

// 去除字符串结尾的空白字符
std::wstring rtrim(const std::wstring& s) {
    size_t end = s.length();
    while (end > 0 && std::isspace(s[end - 1])) {
        --end;
    }
    return s.substr(0, end);
}

// 去除字符串两端的空白字符
std::wstring trim(const std::wstring& s) {
    return rtrim(ltrim(s));
}
