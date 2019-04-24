#include "../include/core/memory.h"
#pragma once

namespace Core
{

bool Init()
{
	if( !Mem::Init() ) return false;
	if( !Str::Init() ) return false;
#ifdef WIN64
#else
	if( !Hook::Init() ) return false;
#endif
	if( !Crypt::Init() ) return false;
	return true;
}

void Release()
{
	Crypt::Release();
#ifdef WIN64
#else
	Hook::Release();
#endif
	Str::Release();
	Mem::Release();
}

}

bool InitBot()
{
	if( !WinAPI::Init() ) return false;
	if( !WinAPI::InitBotImportTable() ) return false;
	return Core::Init();
}

void ReleaseBot()
{
	Core::Release();
}
