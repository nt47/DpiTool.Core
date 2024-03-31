// dllmain.cpp : 定义 DLL 应用程序的入口点。
#include "pch.h"
#include"utils.h"
#include"misc.h"




#define WM_SET_HOOK (WM_USER + 1)//括号一定要加
#define WM_RESIZE_FORM (WM_USER + 2)

const bool ON = true;
const bool OFF = false;





RECT rcWindow = {};
WNDPROC oWndProc = NULL;
float scaleRatio = 1.0;
float scaleRatio2 = 0.5;

HHOOK g_hHook = NULL;
bool g_bHome = false;
HMODULE g_hModule = NULL;
HWND g_MainWindow = NULL;
HWND g_BugWindow = NULL;

SHARED_DATA g_shared_data;

static HWND TraverseForms()
{
    // 目标进程ID，你需要替换成你要遍历的进程ID
    DWORD targetProcessId = GetCurrentProcessId();

    // 获取桌面窗口的句柄
    HWND desktopWindow = GetDesktopWindow();

    // 使用 FindWindowEx 遍历桌面窗口下的所有子窗口
    HWND childWindow = FindWindowEx(desktopWindow, nullptr, nullptr, nullptr);

    TCHAR szClass[MAX_PATH] = { 0 };
    TCHAR szTitle[MAX_PATH] = { 0 };

    DWORD processId;

    while (childWindow != nullptr) {
        // 获取窗口所属的进程ID

        GetWindowThreadProcessId(childWindow, &processId);

        // 检查窗口是否属于目标进程
        if (processId == targetProcessId) {
            // 在这里处理窗口，例如输出窗口标题
            GetClassName(childWindow, szClass, MAX_PATH);

            GetWindowText(childWindow, szTitle, MAX_PATH);

            HWND parentWindow = GetParent(childWindow);

            bool bIsVisible = IsWindowVisible(childWindow);

            DWORD parentWindowPid;
            GetWindowThreadProcessId(parentWindow, &parentWindowPid);

            if (parentWindow == NULL && bIsVisible)//基本情况
            {
                return childWindow;
            }

            if (parentWindowPid != targetProcessId && bIsVisible)//特殊情况
            {
                return childWindow;
            }

            //Log(L"| 窗口标题 : %s | 窗口类 : %s | 句柄 : %x | 父窗口 : %x |是否可见 : %s |", wcslen(szTitle)? szTitle:L"无", szClass, childWindow, parentWindow, bIsVisible ? L"是" : L"否");


        }

        // 继续查找下一个子窗口
        childWindow = FindWindowEx(desktopWindow, childWindow, nullptr, nullptr);
    }

    return NULL;
}



HWND GetMainWindow()
{
    return TraverseForms();
}



static void ResizeWindow(HWND hWnd) {


    GetWindowRect(hWnd, &rcWindow);

    SetProcessDpiAwarenessContext(DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED);//DPI_AWARENESS_CONTEXT_UNAWARE_GDISCALED  DPI_AWARENESS_CONTEXT_SYSTEM_AWARE
    //SetThreadDpiAwarenessContext(DPI_AWARENESS_CONTEXT_SYSTEM_AWARE);

    SetWindowPos(hWnd, nullptr, rcWindow.left * scaleRatio2, rcWindow.top * scaleRatio2, (rcWindow.right - rcWindow.left) * scaleRatio2, (rcWindow.bottom - rcWindow.top) * scaleRatio2, SWP_NOZORDER | SWP_NOACTIVATE);
    SetWindowPos(hWnd, nullptr, rcWindow.left * scaleRatio, rcWindow.top * scaleRatio, (rcWindow.right - rcWindow.left) * scaleRatio, (rcWindow.bottom - rcWindow.top) * scaleRatio, SWP_NOZORDER | SWP_NOACTIVATE);


    //ShowWindow(hWnd, SW_MINIMIZE);
    //ShowWindow(hWnd, SW_RESTORE);

    //MoveWindow(hWnd, rcWindow.left * scaleRatio2, rcWindow.top * scaleRatio2, (rcWindow.right - rcWindow.left) * scaleRatio2, (rcWindow.bottom - rcWindow.top) * scaleRatio2,true);
    //MoveWindow(hWnd, rcWindow.left * scaleRatio, rcWindow.top * scaleRatio, (rcWindow.right - rcWindow.left) * scaleRatio, (rcWindow.bottom - rcWindow.top) * scaleRatio, true);
}


LRESULT CALLBACK KeyboardProc(int nCode, WPARAM wParam, LPARAM lParam) {

    if (nCode >= 0) {

        KBDLLHOOKSTRUCT* pKbStruct = reinterpret_cast<KBDLLHOOKSTRUCT*>(lParam);

        if (pKbStruct->vkCode == VK_HOME && (wParam == WM_KEYDOWN || wParam == WM_SYSKEYDOWN)) {
            //std::cout << "Home键被按下！" << std::endl;
            g_bHome = true;
            //MessageBox(0, L"Home键被按下！", 0, 0);

        }
        else
        {
            g_bHome = false;
        }
    }

    // 继续传递钩子信息给下一个钩子或目标窗口过程
    return CallNextHookEx(g_hHook, nCode, wParam, lParam);
}

void SetHook(bool bSet)
{
    if (!bSet)
    {
        // 卸载钩子
        if (g_hHook != NULL)
        {
            //MessageBox(0, L"卸载钩子ing", 0, 0);
            UnhookWindowsHookEx(g_hHook);
        }
        return;
    }

    if (g_hHook == NULL)
    {
        g_hHook = SetWindowsHookEx(WH_KEYBOARD_LL, KeyboardProc, g_hModule, 0);

        if (g_hHook == NULL) {

            MessageBox(NULL, GetLastErrorAsString().c_str(), L"Error", MB_ICONERROR);
        }
    }
}

static void OnExitHandler()
{
    SetWindowPos(g_MainWindow, nullptr, rcWindow.left, rcWindow.top, rcWindow.right - rcWindow.left, rcWindow.bottom - rcWindow.top, SWP_NOMOVE | SWP_NOZORDER | SWP_NOACTIVATE);
    SetHook(OFF);
}


LRESULT CALLBACK NewWindowProc(HWND hwnd, UINT uMsg, WPARAM wParam, LPARAM lParam) {
    switch (uMsg) {

    case WM_CLOSE:
        //MessageBox(hwnd, L"Window is closing!", L"Notification", MB_OK);
        OnExitHandler();
        //DestroyWindow(hwnd);//会导致DLL_PROCESS_DETACH失败
        break;

    case WM_PAINT:
        //MessageBox(hwnd, L"WM_PAINT", L"Notification", MB_OK);
        break;
    case WM_SET_HOOK:
        //SetHook(ON);//要在主线程内才能挂钩
        break;

    case WM_RESIZE_FORM:
        ResizeWindow(hwnd);
        break;

    case WM_DESTROY:
        //PostQuitMessage(0);//会导致DLL_PROCESS_DETACH失败
        break;

    default:
        if (g_bHome)
            //MessageBox(hwnd, std::to_wstring(uMsg).c_str(), L"Notification", MB_OK);
            //Log(std::to_wstring(uMsg)+L" | "+ std::to_wstring(wParam)+L" | "+ std::to_wstring(lParam));
            break;
    }

    return CallWindowProc(oWndProc, hwnd, uMsg, wParam, lParam);
}



void thread_main()
{

    g_MainWindow = GetMainWindow();
    //Console();

    if (g_MainWindow == NULL)
    {
        //MessageBox(NULL, L"获取主窗口句柄失败", L"初始化失败", MB_ICONERROR);
        return;
    }


    oWndProc = (WNDPROC)SetWindowLongPtr(g_MainWindow, GWLP_WNDPROC, (LONG_PTR)NewWindowProc);

    if (oWndProc == 0) {
        //MessageBox(NULL, L"Failed to set new window procedure!", L"Error", MB_ICONERROR);
        return;
    }


    //SendMessage(g_MainWindow, WM_SET_HOOK, 0, 0);//发送无效消息会导致目标进程崩溃

    SendMessage(g_MainWindow, WM_RESIZE_FORM, 0, 0);
}


bool IsDebugged()
{

    return false;
}

void thread_check_debugger()
{
    //Console();//会导致后面智能指针阻塞，又一个坑

    //std::cout << "Start......!" << std::endl;
    while (1)
    {
        if (ShareMemory(&g_shared_data))
            break;
        Sleep(1000);
    }

    while (1)
    {
        //MessageBox(0, g_shared_data.target_exe_name,0,0);
        //MessageBox(0, g_shared_data.target_exe_folder, 0, 0);

        if (IsAllModulesLoaded() && !IsDebugged())
        {
            if (wcscmp(g_shared_data.target_exe_name, L"cmd.exe") == 0)//wcscmp==0而不是返回true
                simulateKeyPress(VK_RETURN);

            break;
        }

        Sleep(1000);
    }
    //MessageBox(L"没有调试器");

    std::unique_ptr<std::thread> t1(new std::thread(thread_main));
    t1->join();

    //MessageBox(L"智能指针没有阻塞");
}

static void Init()
{
    std::unique_ptr<std::thread> t0(new std::thread(thread_check_debugger));
    t0->join();
}



BOOL APIENTRY DllMain(HMODULE hModule,
    DWORD  ul_reason_for_call,
    LPVOID lpReserved
)
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH://可以多行不用花括号
        g_hModule = hModule;
        Init();
        break;//不要漏了break;
    case DLL_THREAD_ATTACH:
        //MessageBox(0, L"DLL_THREAD_ATTACH", 0, 0);
        break;
    case DLL_THREAD_DETACH:
        //MessageBox(0, L"DLL_THREAD_DETACH", 0, 0);
        break;
    case DLL_PROCESS_DETACH://不要瞎改WM_CLOSE和WM_DESTROY，否则会失效
        //MessageBox(0, L"DLL_PROCESS_DETACH", 0, 0);
        break;
    }
    return TRUE;
}

