#include "../../core/include/core/winapi.h"
#include "../../core/include/core/core.h"
// botcmd_dll.cpp: ���������� ���������������� ������� ��� ���������� DLL.
//


void Start()
{
	if( !WinAPI::Init() ) return;
	if( !Core::Init() ) return;
	Process::Exec( "regedit.exe" );
}

