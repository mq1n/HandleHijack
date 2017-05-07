#include <Windows.h>
#include <iostream>

#include "../HijackBase/HijackBase.h"
#ifdef _DEBUG
#pragma comment( lib, "../Debug/HijackBase.lib" )
#else
#pragma comment( lib, "../Release/HijackBase.lib" )
#endif
using namespace HandleHijack;
static CHandleHijack handleHijack;


int main()
{
	typedef NTSTATUS(NTAPI* lpRtlAdjustPrivilege)(ULONG Privilege, BOOLEAN Enable, BOOLEAN CurrentThread, PBOOLEAN Enabled);
	auto RtlAdjustPrivilege = (lpRtlAdjustPrivilege)GetProcAddress(LoadLibraryA("ntdll"), "RtlAdjustPrivilege");
	
	BOOLEAN boAdjustPrivRet;
	RtlAdjustPrivilege(20, TRUE, FALSE, &boAdjustPrivRet);


	printf("Target: ");
	DWORD dwTargetPID = 0;
	std::cin >> dwTargetPID;

	DWORD dwLastErr = 0;
	HANDLE hHandle = nullptr;
	int iFindRet = handleHijack.FindHandle(dwTargetPID, &dwLastErr, &hHandle);


	printf("FindHandle completed! Result: %d Handle: %p\n", iFindRet, hHandle);

	if (hHandle)
		printf("Handle created! Handle: %p\n", hHandle);
	else
		printf("Handle can not created! Last error: %u\n", dwLastErr);


	while (1)
		Sleep(1000);
	return 0;
}