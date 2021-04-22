#include <Windows.h>
#include <iostream>
#include <TlHelp32.h>

#include <thread>
#include <string>

#pragma comment(lib, "ntdll.lib")

extern "C" NTSYSCALLAPI NTSTATUS NTAPI NtSuspendProcess(IN HANDLE ProcessHandle);
extern "C" NTSYSCALLAPI NTSTATUS NTAPI NtResumeProcess(IN HANDLE ProcessHandle);

void SuspendResumeProcessBulk(std::wstring processName, bool resume = false)
{
	HANDLE handleProcessIterator = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

	if (!handleProcessIterator) return;

	PROCESSENTRY32 pe = {};
	pe.dwSize = sizeof(PROCESSENTRY32);
	
	Process32First(handleProcessIterator, &pe);
	do
	{
		if (std::wstring(pe.szExeFile) == processName)
		{
			HANDLE processHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pe.th32ProcessID);
            if (processHandle && processHandle != INVALID_HANDLE_VALUE)
            {
                (resume ? NtResumeProcess : NtSuspendProcess)(processHandle);
                CloseHandle(processHandle);
            }
		}
	} while (Process32Next(handleProcessIterator, &pe));

	CloseHandle(handleProcessIterator);
}

void KillProcesses(std::wstring processName)
{
    HANDLE handleProcessIterator = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, NULL);

    if (!handleProcessIterator) return;

    PROCESSENTRY32 pe = {};
    pe.dwSize = sizeof(PROCESSENTRY32);

    Process32First(handleProcessIterator, &pe);
    do
    {
        if (std::wstring(pe.szExeFile) == processName)
        {
            HANDLE processHandle = OpenProcess(PROCESS_SUSPEND_RESUME, FALSE, pe.th32ProcessID);
            if (processHandle && processHandle != INVALID_HANDLE_VALUE)
            {
                TerminateProcess(processHandle, 0);
                CloseHandle(processHandle);
            }
        }
    } while (Process32Next(handleProcessIterator, &pe));

    CloseHandle(handleProcessIterator);
}

bool last_resume = true;
VOID CALLBACK WinEventProcCallback(HWND _hwnd)
{
    
    wchar_t buff[256];
    GetClassNameW(_hwnd, buff, sizeof(buff) / sizeof(buff[0]));

    bool is_desktop = GetDesktopWindow() == _hwnd;
    bool is_shell = GetShellWindow() == _hwnd;
    bool is_child_desktop = FindWindowW(L"Shell_TrayWnd", NULL) == _hwnd;
    bool is_child_desktop2 = !wcscmp(buff, L"WorkerW");
    bool resume = is_desktop || is_shell || is_child_desktop || is_child_desktop2;

    if (resume == last_resume) return;

    SuspendResumeProcessBulk(L"webwallpaper32.exe", resume);
    last_resume = resume;

}

void _cdecl atExit()
{
    system("taskkill /f /im webwallpaper32.exe");
}

void Main() {
    atexit(atExit);

    while (true)
    {
        WinEventProcCallback(GetForegroundWindow());
        Sleep(10);
    }
}

extern "C" __declspec(dllexport) void* CreateWPExtPlugin(void* a1) {
    //Plugin API :s
    return 0;
}

extern "C" __declspec(dllexport) const char* GetWPExtPluginVersion() {
    return "pluginAlphaDev0006";
}

BOOL APIENTRY DllMain( HMODULE hModule,
                       DWORD  ul_reason_for_call,
                       LPVOID lpReserved
                     )
{
    switch (ul_reason_for_call)
    {
    case DLL_PROCESS_ATTACH:
        std::thread(Main).detach();
        break;
    case DLL_THREAD_ATTACH:
    case DLL_THREAD_DETACH:
    case DLL_PROCESS_DETACH:
        break;
    }
    return TRUE;
}

