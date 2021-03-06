#include "stdafx.h"
#include <iostream>
#include "windows.h"
#include "tlhelp32.h"
#include "Psapi.h"


using namespace std;


VOID PrintModuleList64(HANDLE CONST hStdOut, DWORD CONST dwProcessId) {
	MODULEENTRY32 meModuleEntry;
	TCHAR szBuff[1024];
	DWORD dwTemp;
	HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, dwProcessId);
	if (INVALID_HANDLE_VALUE == hSnapshot) {
		cout << "CreateToolhelp32Snapshot (of modules) " << GetLastError() << endl;
		return;
	}

	meModuleEntry.dwSize = sizeof(MODULEENTRY32);
	if (!Module32First(hSnapshot, &meModuleEntry)) {
		CloseHandle(hSnapshot);
		return;
	}

	do {
		wsprintf(szBuff, L"\t%s\n", meModuleEntry.szModule);
		WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dwTemp, NULL);
	} while (Module32Next(hSnapshot, &meModuleEntry));

	CloseHandle(hSnapshot);
}

VOID PrintModuleList32(HANDLE CONST hStdOut, DWORD CONST dwProcessId) {
	HANDLE Process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, dwProcessId);
	HMODULE hMods[1024];
	DWORD cbNeeded;

	if (EnumProcessModulesEx(Process, hMods, sizeof(hMods), &cbNeeded, LIST_MODULES_32BIT)) {
		for (int i = 0; i < (cbNeeded / sizeof(HMODULE)); i++)
		{
			TCHAR szModName[MAX_MODULE_NAME32 + 1];

			if (GetModuleBaseName(Process, hMods[i], szModName, sizeof(szModName) / sizeof(TCHAR)))
			{
				// Print the module name.
				_tprintf(TEXT("\t%s\n"), szModName);
			}
		}
	}
}

VOID PrintProcessList(HANDLE CONST hStdOut) {
	PROCESSENTRY32 peProcessEntry;
	TCHAR szBuff[1024];
	BOOL isWOW64;
	DWORD dwTemp;
	HANDLE CONST hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

	if (INVALID_HANDLE_VALUE == hSnapshot) {
		return;
	}

	peProcessEntry.dwSize = sizeof(PROCESSENTRY32);
	Process32First(hSnapshot, &peProcessEntry);
	do {
		TCHAR FilePath[MAX_PATH];

		HANDLE Process = OpenProcess(PROCESS_QUERY_INFORMATION | PROCESS_VM_READ, 0, peProcessEntry.th32ProcessID);
		if (Process) {
			GetModuleFileNameEx(Process, 0, FilePath, MAX_PATH);
		}

		IsWow64Process(Process, &isWOW64);
		//cout << isWOW64 << endl;
		cout << endl;
		if (isWOW64 == 0 | isWOW64 == 1) {
			wsprintf(szBuff, L"=== %08X %s %d ===\r\n",
				peProcessEntry.th32ProcessID, FilePath, peProcessEntry.cntThreads);
		}
		else {
			wsprintf(szBuff, L"=== %08X %s %d ===\r\n",
				peProcessEntry.th32ProcessID, peProcessEntry.szExeFile, peProcessEntry.cntThreads);
		}	
		
		WriteConsole(hStdOut, szBuff, lstrlen(szBuff), &dwTemp, NULL);
		if (isWOW64 == 1) {
			cout << "  WOW64-process" << endl << "  32-bit modules:" << endl;
			PrintModuleList32(hStdOut, peProcessEntry.th32ProcessID);
			cout << "  64-bit modules:" << endl;
		}
		PrintModuleList64(hStdOut, peProcessEntry.th32ProcessID);

		CloseHandle(Process);
	} while (Process32Next(hSnapshot, &peProcessEntry));

	CloseHandle(hSnapshot);
}

void main() {
	HANDLE CONST hStdOut = GetStdHandle(STD_OUTPUT_HANDLE);
	PrintProcessList(hStdOut);
	
	cin.get();

	return;
}
