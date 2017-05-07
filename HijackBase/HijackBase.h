#pragma once

#include <SDKDDKVer.h>
#define WIN32_LEAN_AND_MEAN


namespace HandleHijack
{
	class CHandleHijack
	{
		public:
			CHandleHijack();

			int FindHandle(DWORD dwTargetProcessId, LPDWORD pLastErr, LPHANDLE phHandle);
	};
}

