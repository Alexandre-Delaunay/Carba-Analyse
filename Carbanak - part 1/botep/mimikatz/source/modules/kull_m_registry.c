#include "../../../core/include/core/winapi_simple.h"
#include "../../include/mimikatz/modules/kull_m_registry.h"
/*	Benjamin DELPY `gentilkiwi`
	http://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : http://creativecommons.org/licenses/by/3.0/fr/
*/
//#include "kull_m_registry.h"

const wchar_t * KULL_M_REGISTRY_TYPE_WSTRING[] = {
	L"NONE",
	L"SZ",
	L"EXPAND_SZ",
	L"BINARY",
	L"DWORD",
	L"DWORD_BIG_ENDIAN",
	L"LINK",
	L"MULTI_SZ",
	L"RESOURCE_LIST",
	L"FULL_RESOURCE_DESCRIPTOR",
	L"RESOURCE_REQUIREMENTS_LIST",
	L"QWORD",
};

BOOL kull_m_registry_open(IN KULL_M_REGISTRY_TYPE Type, IN HANDLE hAny, OUT PKULL_M_REGISTRY_HANDLE *hRegistry)
{
	BOOL status = FALSE;
	PKULL_M_REGISTRY_HIVE_HEADER pFh;
	PKULL_M_REGISTRY_HIVE_BIN_HEADER pBh;

	*hRegistry = (PKULL_M_REGISTRY_HANDLE) API(KERNEL32, LocalAlloc)(LPTR, sizeof(KULL_M_REGISTRY_HANDLE));
	if(*hRegistry)
	{
		(*hRegistry)->type = Type;
		switch (Type)
		{
		case KULL_M_REGISTRY_TYPE_OWN:
			status = TRUE;
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			(*hRegistry)->pHandleHive = (PKULL_M_REGISTRY_HIVE_HANDLE) API(KERNEL32, LocalAlloc)(LPTR, sizeof(KULL_M_REGISTRY_HIVE_HANDLE));
			if((*hRegistry)->pHandleHive)
			{
				(*hRegistry)->pHandleHive->hFileMapping = API(KERNEL32, CreateFileMappingW)(hAny, NULL, PAGE_READONLY, 0, 0, NULL);
				if((*hRegistry)->pHandleHive->hFileMapping)
				{
					(*hRegistry)->pHandleHive->pMapViewOfFile = API(KERNEL32, MapViewOfFile)((*hRegistry)->pHandleHive->hFileMapping, FILE_MAP_READ, 0, 0, 0);
					if(pFh = (PKULL_M_REGISTRY_HIVE_HEADER) (*hRegistry)->pHandleHive->pMapViewOfFile)
					{
						if((pFh->tag == 'fger') && (pFh->fileType == 0))
						{
							pBh = (PKULL_M_REGISTRY_HIVE_BIN_HEADER) ((PBYTE) pFh + sizeof(KULL_M_REGISTRY_HIVE_HEADER));
							if(pBh->tag == 'nibh')
							{
								(*hRegistry)->pHandleHive->pStartOf = (PBYTE) pBh;
								(*hRegistry)->pHandleHive->pRootNamedKey = (PKULL_M_REGISTRY_HIVE_KEY_NAMED) ((PBYTE) pBh + sizeof(KULL_M_REGISTRY_HIVE_BIN_HEADER) + pBh->offsetHiveBin);
								status = (((*hRegistry)->pHandleHive->pRootNamedKey->tag == 'kn') && ((*hRegistry)->pHandleHive->pRootNamedKey->flags & (KULL_M_REGISTRY_HIVE_KEY_NAMED_FLAG_ROOT | KULL_M_REGISTRY_HIVE_KEY_NAMED_FLAG_LOCKED)));
							}
						}
						if(!status)
						{
							API(KERNEL32, UnmapViewOfFile)((*hRegistry)->pHandleHive->pMapViewOfFile);
							API(KERNEL32, CloseHandle)((*hRegistry)->pHandleHive->hFileMapping);
						}
					}
				}
			}
			break;
		default:
			break;
		}
		if(!status)
			API(KERNEL32, LocalFree)(*hRegistry);
	}
	return status;
}

PKULL_M_REGISTRY_HANDLE kull_m_registry_close(IN PKULL_M_REGISTRY_HANDLE hRegistry)
{
	if(hRegistry)
	{
		switch (hRegistry->type)
		{
		case KULL_M_REGISTRY_TYPE_HIVE:
			if(hRegistry->pHandleHive)
			{
				if(hRegistry->pHandleHive->pMapViewOfFile)
					API(KERNEL32, UnmapViewOfFile)(hRegistry->pHandleHive->pMapViewOfFile);
				if(hRegistry->pHandleHive->hFileMapping)
					API(KERNEL32, CloseHandle)(hRegistry->pHandleHive->hFileMapping);
				API(KERNEL32, LocalFree)(hRegistry->pHandleHive);
			}
		default:
			break;
		}
		return (PKULL_M_REGISTRY_HANDLE) API(KERNEL32, LocalFree)(hRegistry);
	}
	else return NULL;
}

BOOL kull_m_registry_RegOpenKeyEx(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN HKEY hKey, IN OPTIONAL LPCWSTR lpSubKey, IN DWORD ulOptions, IN REGSAM samDesired, OUT PHKEY phkResult)
{
	BOOL status = FALSE;
	DWORD dwErrCode;
	PKULL_M_REGISTRY_HIVE_KEY_NAMED pKn;
	PKULL_M_REGISTRY_HIVE_BIN_CELL pHbC;
	wchar_t * ptrF, * buffer;
	
	*phkResult = 0;
	switch(hRegistry->type)
	{
		case KULL_M_REGISTRY_TYPE_OWN:
			dwErrCode = RegOpenKeyEx(hKey, lpSubKey, ulOptions, samDesired, phkResult);
			if(!(status = (dwErrCode == ERROR_SUCCESS)))
				SetLastError(dwErrCode);
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			pKn = hKey ? (PKULL_M_REGISTRY_HIVE_KEY_NAMED) hKey : hRegistry->pHandleHive->pRootNamedKey;
			if(pKn->tag == 'kn')
			{
				if(lpSubKey)
				{
					if(pKn->nbSubKeys && (pKn->offsetSubKeys != -1))
					{
						pHbC = (PKULL_M_REGISTRY_HIVE_BIN_CELL) (hRegistry->pHandleHive->pStartOf + pKn->offsetSubKeys);
						if(ptrF = (wchar_t*)wcschr(lpSubKey, L'\\'))
						{
							if(buffer = (wchar_t *) API(KERNEL32, LocalAlloc)(LPTR, (ptrF - lpSubKey + 1) * sizeof(wchar_t)))
							{
								RtlCopyMemory(buffer, lpSubKey, (ptrF - lpSubKey) * sizeof(wchar_t));
								if(*phkResult = (HKEY) kull_m_registry_searchKeyNamedInList(hRegistry, pHbC, buffer))
									kull_m_registry_RegOpenKeyEx(hRegistry, *phkResult, ptrF + 1, ulOptions, samDesired, phkResult);
								API(KERNEL32, LocalFree)(buffer);
							}
						}
						else *phkResult = (HKEY) kull_m_registry_searchKeyNamedInList(hRegistry, pHbC, lpSubKey);
					}
				}
				else *phkResult = (HKEY) pKn;
			}
			status = (*phkResult != 0);
			break;
		default:
			break;
	}
	return status;
}

PKULL_M_REGISTRY_HIVE_KEY_NAMED kull_m_registry_searchKeyNamedInList(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN PKULL_M_REGISTRY_HIVE_BIN_CELL pHbC, IN LPCWSTR lpSubKey)
{
	PKULL_M_REGISTRY_HIVE_KEY_NAMED pKn, result = NULL;
	PKULL_M_REGISTRY_HIVE_LF_LH pLfLh;
	DWORD i;
	wchar_t * buffer;

	switch(pHbC->tag)
	{
	case 'fl':
	case 'hl':
		pLfLh = (PKULL_M_REGISTRY_HIVE_LF_LH) pHbC;
		for(i = 0 ; i < pLfLh->nbElements && !result; i++)
		{
			pKn = (PKULL_M_REGISTRY_HIVE_KEY_NAMED) (hRegistry->pHandleHive->pStartOf + pLfLh->elements[i].offsetNamedKey);
			if(pKn->tag == 'kn')
			{
				if(pKn->flags & KULL_M_REGISTRY_HIVE_KEY_NAMED_FLAG_ASCII_NAME)
					buffer = kull_m_string_qad_ansi_c_to_unicode((char *) pKn->keyName, pKn->szKeyName);
				else if(buffer = (wchar_t *) API(KERNEL32, LocalAlloc)(LPTR, pKn->szKeyName + sizeof(wchar_t)))
					RtlCopyMemory(buffer, pKn->keyName, pKn->szKeyName);

				if(buffer)
				{
					if(API(NTDLL, _wcsicmp)(lpSubKey, buffer) == 0)
						result = pKn;
					API(KERNEL32, LocalFree)(buffer);
				}
			}
		}
		break;
	case 'il':
	case 'ir':
	default:
		break;
	}
	return result;
}

BOOL kull_m_registry_RegQueryInfoKey(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN HKEY hKey, OUT OPTIONAL LPWSTR lpClass, IN OUT OPTIONAL LPDWORD lpcClass, IN OPTIONAL LPDWORD lpReserved, OUT OPTIONAL LPDWORD lpcSubKeys, OUT OPTIONAL LPDWORD lpcMaxSubKeyLen, OUT OPTIONAL LPDWORD lpcMaxClassLen, OUT OPTIONAL LPDWORD lpcValues, OUT OPTIONAL LPDWORD lpcMaxValueNameLen, OUT OPTIONAL LPDWORD lpcMaxValueLen, OUT OPTIONAL LPDWORD lpcbSecurityDescriptor, OUT OPTIONAL PFILETIME lpftLastWriteTime)
{
	BOOL status = FALSE;
	DWORD dwErrCode;
	PKULL_M_REGISTRY_HIVE_KEY_NAMED pKn;
	DWORD szInCar;

	switch(hRegistry->type)
	{
		case KULL_M_REGISTRY_TYPE_OWN:
			dwErrCode = RegQueryInfoKey(hKey, lpClass, lpcClass, lpReserved, lpcSubKeys, lpcMaxSubKeyLen, lpcMaxClassLen, lpcValues, lpcMaxValueNameLen, lpcMaxValueLen, lpcbSecurityDescriptor, lpftLastWriteTime);
			if(!(status = (dwErrCode == ERROR_SUCCESS)))
				SetLastError(dwErrCode);
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			pKn = hKey ? (PKULL_M_REGISTRY_HIVE_KEY_NAMED) hKey : hRegistry->pHandleHive->pRootNamedKey;
			if(status = (pKn->tag == 'kn'))
			{
				if(lpcSubKeys)
					*lpcSubKeys = pKn->nbSubKeys;

				if(lpcMaxSubKeyLen)
					*lpcMaxSubKeyLen = pKn->szMaxSubKeyName / sizeof(wchar_t);

				if(lpcMaxClassLen)
					*lpcMaxClassLen = pKn->szMaxSubKeyClassName / sizeof(wchar_t);

				if(lpcValues)
					*lpcValues = pKn->nbValues;

				if(lpcMaxValueNameLen)
					*lpcMaxValueNameLen = pKn->szMaxValueName / sizeof(wchar_t);

				if(lpcMaxValueLen)
					*lpcMaxValueLen = pKn->szMaxValueData;

				if(lpcbSecurityDescriptor)
					*lpcbSecurityDescriptor = 0; /* NOT SUPPORTED */

				if(lpftLastWriteTime)
					*lpftLastWriteTime = pKn->lastModification;

				if(lpcClass)
				{
					szInCar = pKn->szClassName / sizeof(wchar_t);
					if(lpClass)
					{
						if(status = (*lpcClass > szInCar))
						{
							RtlCopyMemory(lpClass, &((PKULL_M_REGISTRY_HIVE_BIN_CELL) (hRegistry->pHandleHive->pStartOf + pKn->offsetClassName))->data , pKn->szClassName);
							lpClass[szInCar] = L'\0';
						}
					}
					*lpcClass = szInCar;
				}
			}
			break;
		default:
			break;
	}
	
	return status;
}

BOOL kull_m_registry_RegQueryValueEx(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN HKEY hKey, IN OPTIONAL LPCWSTR lpValueName, IN LPDWORD lpReserved, OUT OPTIONAL LPDWORD lpType, OUT OPTIONAL LPBYTE lpData, IN OUT OPTIONAL LPDWORD lpcbData)
{
	BOOL status = FALSE;
	DWORD dwErrCode;
	PKULL_M_REGISTRY_HIVE_KEY_NAMED pKn;
	PKULL_M_REGISTRY_HIVE_VALUE_LIST pVl;
	PKULL_M_REGISTRY_HIVE_VALUE_KEY pVk, pFvk = NULL;
	wchar_t * buffer;
	DWORD i, szData;
	PVOID dataLoc;

	switch(hRegistry->type)
	{
		case KULL_M_REGISTRY_TYPE_OWN:
			dwErrCode = RegQueryValueEx(hKey, lpValueName, lpReserved, lpType, lpData, lpcbData);
			if(!(status = (dwErrCode == ERROR_SUCCESS)))
				SetLastError(dwErrCode);
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			pKn = hKey ? (PKULL_M_REGISTRY_HIVE_KEY_NAMED) hKey : hRegistry->pHandleHive->pRootNamedKey;
			if(pKn->tag == 'kn')
			{
				if(pKn->nbValues && (pKn->offsetValues != -1))
				{
					pVl = (PKULL_M_REGISTRY_HIVE_VALUE_LIST) (hRegistry->pHandleHive->pStartOf + pKn->offsetValues);
					for(i = 0 ; i < pKn->nbValues && !pFvk; i++)
					{
						pVk = (PKULL_M_REGISTRY_HIVE_VALUE_KEY) (hRegistry->pHandleHive->pStartOf + pVl->offsetValue[i]);
						if(pVk->tag == 'kv')
						{
							if(lpValueName)
							{
								if(pVk->szValueName)
								{
									if(pVk->flags & KULL_M_REGISTRY_HIVE_VALUE_KEY_FLAG_ASCII_NAME)
										buffer = kull_m_string_qad_ansi_c_to_unicode((char *) pVk->valueName, pVk->szValueName);
									else if(buffer = (wchar_t *) API(KERNEL32, LocalAlloc)(LPTR, pKn->szKeyName + sizeof(wchar_t)))
										RtlCopyMemory(buffer, pKn->keyName, pKn->szKeyName);

									if(buffer)
									{
										if(API(NTDLL, _wcsicmp)(lpValueName, buffer) == 0)
											pFvk = pVk;
										API(KERNEL32, LocalFree)(buffer);
									}
								}
							}
							else if(!pVk->szValueName)
								pFvk = pVk;

							if(status = (pFvk != NULL))
							{
								szData = pFvk->szData & ~0x80000000;
								if(lpType)
									*lpType = pFvk->typeData;
								
								if(lpcbData)
								{
									if(lpData)
									{
										if(status = (*lpcbData >= szData))
										{
											dataLoc = (pFvk->szData & 0x80000000) ? &pFvk->offsetData : (PVOID) &(((PKULL_M_REGISTRY_HIVE_BIN_CELL) (hRegistry->pHandleHive->pStartOf + pFvk->offsetData))->data);
											RtlCopyMemory(lpData, dataLoc , szData);
										}
									}
									*lpcbData = szData;
								}
							}
						}
					}
				}
			}
			break;
		default:
			break;
	}
	return status;
}

BOOL kull_m_registry_RegEnumKeyEx(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN HKEY hKey, IN DWORD dwIndex, OUT LPWSTR lpName, IN OUT LPDWORD lpcName, IN LPDWORD lpReserved, OUT OPTIONAL LPWSTR lpClass, IN OUT OPTIONAL LPDWORD lpcClass, OUT OPTIONAL PFILETIME lpftLastWriteTime)
{
	BOOL status = FALSE;
	DWORD dwErrCode, szInCar;
	PKULL_M_REGISTRY_HIVE_KEY_NAMED pKn, pCandidateKn;
	PKULL_M_REGISTRY_HIVE_BIN_CELL pHbC;
	PKULL_M_REGISTRY_HIVE_LF_LH pLfLh;
	wchar_t * buffer;

	switch(hRegistry->type)
	{
		case KULL_M_REGISTRY_TYPE_OWN:
			dwErrCode = RegEnumKeyEx(hKey, dwIndex, lpName, lpcName, lpReserved, lpClass, lpcClass, lpftLastWriteTime);
			if(!(status = (dwErrCode == ERROR_SUCCESS)))
				SetLastError(dwErrCode);
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			pKn = (PKULL_M_REGISTRY_HIVE_KEY_NAMED) hKey;
			if(pKn->nbSubKeys && (dwIndex < pKn->nbSubKeys) && (pKn->offsetSubKeys != -1))
			{
				pHbC = (PKULL_M_REGISTRY_HIVE_BIN_CELL) (hRegistry->pHandleHive->pStartOf + pKn->offsetSubKeys);
				switch(pHbC->tag)
				{
				case 'fl':
				case 'hl':
					pLfLh = (PKULL_M_REGISTRY_HIVE_LF_LH) pHbC;
					if(pLfLh->nbElements && (dwIndex < pLfLh->nbElements))
					{
						pCandidateKn = (PKULL_M_REGISTRY_HIVE_KEY_NAMED) (hRegistry->pHandleHive->pStartOf + pLfLh->elements[dwIndex].offsetNamedKey);
						if(pCandidateKn->tag == 'kn')
						{
							if(lpftLastWriteTime)
								*lpftLastWriteTime = pKn->lastModification;
							
							if(pCandidateKn->flags & KULL_M_REGISTRY_HIVE_KEY_NAMED_FLAG_ASCII_NAME)
							{
								szInCar = pCandidateKn->szKeyName;
								if(status = (*lpcName > szInCar))
								{
									if(buffer = kull_m_string_qad_ansi_c_to_unicode((char *) pCandidateKn->keyName, szInCar))
									{
										RtlCopyMemory(lpName, buffer, szInCar * sizeof(wchar_t));
										API(KERNEL32, LocalFree)(buffer);
									}
								}
							}
							else
							{
								szInCar = pCandidateKn->szKeyName / sizeof(wchar_t);
								if(status = (*lpcName > szInCar))
									RtlCopyMemory(lpName, pCandidateKn->keyName, pKn->szKeyName);
							}
							if(status)
								lpName[szInCar] = L'\0';
							*lpcName = szInCar;
							
							if(lpcClass)
							{
								szInCar = pCandidateKn->szClassName / sizeof(wchar_t);
								if(lpClass)
								{
									if(status = (*lpcClass > szInCar))
									{
										RtlCopyMemory(lpClass, &((PKULL_M_REGISTRY_HIVE_BIN_CELL) (hRegistry->pHandleHive->pStartOf + pCandidateKn->offsetClassName))->data , pCandidateKn->szClassName);
										lpClass[szInCar] = L'\0';
									}
								}
								*lpcClass = szInCar;
							}
						}
					}
					break;
				case 'il':
				case 'ir':
				default:
					break;
				}
			}
			break;
		default:
			break;
	}
	return status;
}

BOOL kull_m_registry_RegCloseKey(IN PKULL_M_REGISTRY_HANDLE hRegistry, IN HKEY hKey)
{
	BOOL status = FALSE;
	DWORD dwErrCode;
	switch(hRegistry->type)
	{
		case KULL_M_REGISTRY_TYPE_OWN:
			dwErrCode = RegCloseKey(hKey);
			if(!(status = (dwErrCode == ERROR_SUCCESS)))
				SetLastError(dwErrCode);
			break;
		case KULL_M_REGISTRY_TYPE_HIVE:
			status = TRUE;
			break;
		default:
			break;
	}
	return status;
}