#include "Hook.h"

/*
	An optional entry point into a dynamic-link library (DLL).
	When the system starts or terminates a process or thread, 
	it calls the entry-point function for each loaded DLL using
	the first thread of the process. 
	The system also calls the entry-point function for a DLL when 
	it is loaded or unloaded using the LoadLibrary and FreeLibrary functions.
*/
BOOL APIENTRY DllMain(HMODULE hModule, DWORD  ul_reason_for_call, LPVOID lpReserved)
{
	Hook hook(ul_reason_for_call);
	hook.start();
	return TRUE;
}
