#pragma once
#include <string>
#include<Psapi.h>
#include <Shlwapi.h>
#include <unordered_map>
#include <tchar.h>
#include <tlhelp32.h>
#include <fstream>
#include <iostream>
#include <iomanip>
#include <Locale.h>
#include <thread>
#include<winternl.h>

#pragma comment(lib, "Shlwapi.lib") //ÎÄ¼þ²Ù×÷ÒÀÀµ
#pragma comment(lib, "Shcore.lib") //dpiÒÀÀµ


#ifdef _WIN64
typedef DWORD64 qw;
#else
typedef DWORD qw;
#endif

static std::wstring fmt(const wchar_t* format, ...);
void MessageBox(const wchar_t* format, ...);
void Console();
std::wstring GetCurrentProcessName();
void Logfile(const std::wstring& message, const std::wstring& filename = L"logfile.txt");
void Log(const wchar_t* format, ...);
std::wstring GetLastErrorAsString();
bool IsAllModulesLoaded();
void simulateKeyPress(WORD keyCode);
std::wstring trim(const std::wstring& s);