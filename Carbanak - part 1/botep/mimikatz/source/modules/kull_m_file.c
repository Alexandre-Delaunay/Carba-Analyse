#include "../../../core/include/core/api_funcs_type.h"
#include "../../../core/include/core/winapi_simple.h"
/*	Benjamin DELPY `gentilkiwi`
	http://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : http://creativecommons.org/licenses/by/3.0/fr/
*/
//#include "kull_m_file.h"
//#include "winapi.h"

BOOL kull_m_file_getCurrentDirectory(wchar_t ** ppDirName)
{
	BOOL reussite = FALSE;
	DWORD tailleRequise = GetCurrentDirectory(0, NULL);
	*ppDirName = (wchar_t *) API(KERNEL32, LocalAlloc)(LPTR, tailleRequise * sizeof(wchar_t));
	
	reussite = (tailleRequise > 0 && (GetCurrentDirectory(tailleRequise, *ppDirName) == tailleRequise - 1));
	
	if(!reussite)
		API(KERNEL32, LocalFree)(*ppDirName);

	return reussite;
}

BOOL kull_m_file_getAbsolutePathOf(wchar_t *thisData, wchar_t ** reponse)
{
	BOOL reussite = FALSE;
	wchar_t *monRep;
	*reponse = (wchar_t *) API(KERNEL32, LocalAlloc)(LPTR, MAX_PATH);

	if(PathIsRelative(thisData))
	{
		if(kull_m_file_getCurrentDirectory(&monRep))
		{
			reussite = (PathCombine(*reponse , monRep, thisData) != NULL);
			API(KERNEL32, LocalFree)(monRep);
		}
	}
	else
		reussite = PathCanonicalize(*reponse, thisData);

	if(!reussite)
		API(KERNEL32, LocalFree)(*reponse);

	return reussite;
}

BOOL kull_m_file_isFileExist(wchar_t *fileName)
{
	BOOL reussite = FALSE;
	HANDLE monFichier = CreateFile(fileName, 0, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	reussite = (monFichier && monFichier != INVALID_HANDLE_VALUE);
	if(reussite)
		API(KERNEL32, CloseHandle)(monFichier);

	return reussite;
}

BOOL kull_m_file_writeData(PCWCHAR fileName, PBYTE data, DWORD lenght)
{
	BOOL reussite = FALSE;
	DWORD dwBytesWritten;
	HANDLE hFile = CreateFile(fileName, GENERIC_WRITE, 0, NULL, CREATE_ALWAYS, 0, NULL);
	
	if(hFile && hFile != INVALID_HANDLE_VALUE)
	{
		if(API(KERNEL32, WriteFile)(hFile, data, lenght, &dwBytesWritten, NULL) && (lenght == dwBytesWritten))
			reussite = FlushFileBuffers(hFile);
		API(KERNEL32, CloseHandle)(hFile);
	}
	return reussite;
}

BOOL kull_m_file_readData(PCWCHAR fileName, PBYTE * data, PDWORD lenght)	// for little files !
{
	BOOL reussite = FALSE;
	DWORD dwBytesReaded;
	LARGE_INTEGER filesize;

	HANDLE hFile = CreateFile(fileName, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, 0, NULL);
	if(hFile && hFile != INVALID_HANDLE_VALUE)
	{
		if(GetFileSizeEx(hFile, &filesize) && !filesize.HighPart)
		{
			*lenght = filesize.LowPart;
			if(*data = (PBYTE) API(KERNEL32, LocalAlloc)(LPTR, *lenght))
			{
				if(!(reussite = API(KERNEL32, ReadFile)(hFile, *data, *lenght, &dwBytesReaded, NULL) && (*lenght == dwBytesReaded)))
					API(KERNEL32, LocalFree)(*data);
			}
		}
		API(KERNEL32, CloseHandle)(hFile);
	}
	return reussite;
}

const wchar_t kull_m_file_forbiddenChars[] = {L'\\', L'/', L':', L'*', L'?', L'\"', L'<', L'>', L'|'};
void kull_m_file_cleanFilename(wchar_t *fileName)
{
	DWORD i, j;
	for(i = 0; fileName[i]; i++)
		for(j = 0; j < sizeof(kull_m_file_forbiddenChars) / sizeof(wchar_t); j++)
			if(fileName[i] == kull_m_file_forbiddenChars[j])
				fileName[i] = L'~';
}