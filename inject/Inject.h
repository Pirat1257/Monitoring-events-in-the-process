#define WIN32_LEAN_AND_MEAN 
#include <iostream>
#include <Windows.h>
#include <ShlObj.h>
#include <list>
#include <tlhelp32.h>
#include <comdef.h>
#include <ws2tcpip.h>
#pragma comment(lib, "ws2_32.lib")
#include <tchar.h>

using namespace std;

#pragma once
class Inject
{
private:
	DWORD Target_Process_PID; // Целевой процесс выбирается по значению PID;
	string Function_File_Name; // Имя функции, вызов которой требуется отследить, или Имя файла, который надо скрыть от процесса;
	bool func_hide; // Операция: отследить функцию (0) или скрыть файл (1);
	bool pid_name; // Выбор процесса: по PID (0) или по имени (1);
	SOCKET listen_socket, client_socket; // Прослушивающий и клиентски сокеты
public:
	int start(); // Главная функция
	Inject(list <string> input);
	~Inject();
private:
	DWORD get_PID_by_name(string processname); // Получение PID по имени
	bool inject_dll(); // DLL инъекция
	bool network(); // Настройка подключения
	void work_with_client(); // Обмен сообщениями с клиентом
};