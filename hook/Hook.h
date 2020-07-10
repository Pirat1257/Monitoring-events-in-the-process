#define WIN32_LEAN_AND_MEAN
#include <stdio.h>
#include <iostream>
#include <Windows.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include "Hide.h"
#include <tchar.h>

#pragma once
class Hook
{
private:
	DWORD  ul_reason_for_call;
public:
	int start(); // Главная_функция
	Hook(DWORD  reason);
	~Hook();
private:
	bool connect_to(); // Подключение_к_серверу
};