#include <windows.h>
#include <cstdio>
#include "minhook/MinHook.h"

typedef LPSTR(WINAPI* _GetCommandLineA)();
typedef HMODULE(WINAPI* _LoadLibraryW)(const char* m_szModule);

_GetCommandLineA fpGCLA = NULL;
_LoadLibraryW fpLLA = NULL;

int m_nLineCount = 0;
inline void SetConsoleColor(int m_nColor) {
	int m_nBlack = 0;
	m_nColor %= 16;
	m_nBlack %= 16;
	unsigned short m_shCAttributes = ((unsigned)m_nBlack << 4) | (unsigned)m_nColor;
	SetConsoleTextAttribute(GetStdHandle(STD_OUTPUT_HANDLE), m_shCAttributes);
}
LPSTR WINAPI __GetCommandLineA() {
	if (!GetModuleHandleA("serverbrowser.dll")) // Indicator for whether everything has loaded.
		return fpGCLA();
	char m_szCmdLine[1024];
	SetConsoleColor(15);
	printf("[+] Trusted Check - (Bypassed)\n");
	sprintf_s(m_szCmdLine, sizeof(m_szCmdLine), "%s -allow_third_party_software", fpGCLA());
	return m_szCmdLine;
}
HMODULE WINAPI __LoadLibaryA(const char* m_szModule) {
	if (!GetModuleHandleA("serverbrowser.dll")) // Indicator for whether everything has loaded.
		return fpLLA(m_szModule);
	SetConsoleColor(14);
	printf("[+] Library Injected: %s\n", m_szModule);
	return fpLLA(m_szModule);
}

void DetourFunctions() {
	AllocConsole();
	AttachConsole(GetCurrentThreadId());
	SetConsoleTitleA("Trusted Mode Bypass");
	freopen_s((FILE**)stdout, "CONOUT$", "w", stdout);
	SetConsoleColor(10);
	printf("[+] Injected into csgo.exe.\n");
	if (MH_Initialize() != MH_OK) {
		SetConsoleColor(4);
		printf("[-] Failed to initialize MinHook.\n");
		return;
	}
	if (MH_CreateHook(&GetCommandLineA, &__GetCommandLineA, (LPVOID*)&fpGCLA) != MH_OK) {
		SetConsoleColor(4);
		printf("[-] Failed to hook GetCommandLineA.\n");
		return;
	}
	if (MH_CreateHook(&LoadLibraryA, &__LoadLibaryA, (LPVOID*)&fpLLA) != MH_OK) {
		SetConsoleColor(4);
		printf("[-] Failed to hook LoadLibraryW.\n");
		return;
	}
	if (MH_EnableHook(MH_ALL_HOOKS) != MH_OK) {
		SetConsoleColor(4);
		printf("[-] Failed to enable hooks.\n");
		return;
	}
	printf("[+] Successfully hooked!\n");
	SetConsoleColor(0);
}
BOOL APIENTRY DllMain(HMODULE m_hInstance, DWORD m_dwReason, LPVOID m_pReserved) {
	if (m_dwReason == DLL_PROCESS_ATTACH) {
		if (MessageBoxA(FindWindowA("Valve001", nullptr), "Are you sure you want to bypass trusted mode?", "Trusted Mode Bypass", MB_YESNO) != 7)
			CreateThread(NULL, 0, (LPTHREAD_START_ROUTINE)DetourFunctions, NULL, 0, 0);
		return 1;
	}
}
