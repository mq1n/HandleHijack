#include <Windows.h>
#include <iostream>

#include "main.h"
#include "HijackBase.h"


static void SafeCloseHandle(HANDLE processHandle)
{
	__try {
		if (processHandle)
			CloseHandle(processHandle);
	}
	__except (EXCEPTION_EXECUTE_HANDLER) {
	}
}


static HMODULE hNtdll = nullptr;
static lpNtQuerySystemInformation NtQuerySystemInformation = nullptr;
static lpNtDuplicateObject NtDuplicateObject = nullptr;

HandleHijack::CHandleHijack::CHandleHijack()
{
	hNtdll = LoadLibraryA("ntdll");

	NtQuerySystemInformation = (lpNtQuerySystemInformation)GetProcAddress(hNtdll, "NtQuerySystemInformation");
	NtDuplicateObject = (lpNtDuplicateObject)GetProcAddress(hNtdll, "NtDuplicateObject");
}

int HandleHijack::CHandleHijack::FindHandle(DWORD dwTargetProcessId, LPDWORD pLastErr, LPHANDLE phHandle)
{
	HANDLE hProcess = nullptr;
	NTSTATUS status = 0;
	ULONG handleInfoSize = 0x10000;
	PSYSTEM_HANDLE_INFORMATION handleInfo = 0;
	HANDLE processHandle = nullptr;


	handleInfo = (PSYSTEM_HANDLE_INFORMATION)malloc(handleInfoSize);
	ZeroMemory(handleInfo, handleInfoSize);

	while ((status = NtQuerySystemInformation(SystemHandleInformation, handleInfo, handleInfoSize, NULL)) == STATUS_INFO_LENGTH_MISMATCH)
	{
		handleInfoSize *= 2;
		handleInfo = (PSYSTEM_HANDLE_INFORMATION)realloc(handleInfo, handleInfoSize);
	}

	if (!NT_SUCCESS(status)) {
		free(handleInfo);
		*pLastErr = GetLastError();
		return -1;
	}

	for (ULONG i = 0; i < handleInfo->HandleCount; i++)
	{
		auto handle = handleInfo->Handles[i];
		HANDLE dupHandle = NULL;

		if (handle.ObjectTypeNumber != 0x5 && handle.ObjectTypeNumber != 0x7) /* Just process handles */
			continue;


		SafeCloseHandle(processHandle);

		processHandle = OpenProcess(PROCESS_DUP_HANDLE, FALSE, handle.ProcessId);
		if (!processHandle || processHandle == INVALID_HANDLE_VALUE)
			continue;


		status = NtDuplicateObject(processHandle, (HANDLE)handle.Handle, NtCurrentProcess, &dupHandle, PROCESS_ALL_ACCESS, 0, 0);
		if (!NT_SUCCESS(status))
		{
			*pLastErr = GetLastError();		
			continue;
		}


		if (GetProcessId(dupHandle) != dwTargetProcessId) {
			SafeCloseHandle(dupHandle);
			continue;
		}


		hProcess = dupHandle;
		printf("Available handle found on: %u\n", handle.ProcessId);
		break;
	}

	free(handleInfo);
	SafeCloseHandle(processHandle);

	if (!hProcess)
		return -2;

	SetLastError(ERROR_SUCCESS);
	*phHandle = hProcess;

	return 0;
}

