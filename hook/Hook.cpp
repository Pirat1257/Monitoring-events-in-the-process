#include "Hook.h"

SOCKET client_socket;
bool msg_ready = false;
char timer[512] = { 0 };
char buff_time[512] = { 0 };
extern "C" void Lab2Hook_x64();
extern "C" void Lab2Hook_x86();
extern "C" LPVOID DynamicTarget = NULL;

/*------------------Отправка_времени_вызова_функции------------------*/
void send_msg()
{
	// Если функция перехватчик поставила true, то на проверку
	if (msg_ready == true)
	{
		// Получаем время
		SYSTEMTIME time;
		GetLocalTime(&time);
		sprintf_s(buff_time, "%d-%02d-%02d %02d:%02d:%02d", time.wYear, time.wMonth,
			time.wDay, time.wHour, time.wMinute, time.wSecond);
		if (timer == NULL || strcmp(buff_time, timer)) // Чтобы избежать спама сообщений
		{
			sprintf_s(timer, "%d-%02d-%02d %02d:%02d:%02d", time.wYear, time.wMonth,
				time.wDay, time.wHour, time.wMinute, time.wSecond);
			send(client_socket, buff_time, strlen(buff_time), 0);
			memset(buff_time, 0, 512);
		}
		msg_ready = false;
	}
}

/*------------------Функция-перехватчик------------------*/
extern "C" VOID DynamicDetour()
{
	msg_ready = true;
	send_msg();
}

/*------------------Подключение_к_серверу------------------*/
bool Hook::connect_to()
{
	// Инициализируем Winsock
	WSAData wsaData;
	sockaddr_in server_addr;
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	client_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	server_addr.sin_family = AF_INET;
	InetPton(AF_INET, _T("127.0.0.1"), &server_addr.sin_addr.s_addr);
	server_addr.sin_port = htons(9000);
	// Коннектимся
	if (connect(client_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		closesocket(client_socket);
		WSACleanup();
		return false;
	}
	return true;
}

/*------------------Главная_функция------------------*/
int Hook::start()
{
	/*
		DLL_PROCESS_ATTACH - The DLL is being loaded into the virtual
		address space of the current process as a result of the
		process starting up or as a result of a call to LoadLibrary.
	*/
	if (ul_reason_for_call == DLL_PROCESS_ATTACH)
	{
		// Пытаемся подключиться
		if (connect_to() == false)
			return 0;
		char buff[512] = { 0 };
		// Приветствуем сервер
		sprintf_s(buff, "HEY BRO, NICE DICK!");
		send(client_socket, buff, strlen(buff) + 1, 0);
		memset(buff, 0, 512);
		// Принимаем запрос от него
		recv(client_socket, buff, 512, 0); // Пример пакета: notepad.exe0
		// Действие: -func
		if (buff[strlen(buff) - 1] == '0')
		{
			// Избавляемся от операции, оставляем только имя
			buff[strlen(buff) - 1] = '\0';
			DynamicTarget = DetourFindFunction("kernel32.dll", buff);
			DetourTransactionBegin();
			DetourUpdateThread(GetCurrentThread());
			// Производим замену функций, в .asm файле произойдет вызов DynamicDetour функции,
			// затем управление вернется изначальной функции, если этого не сделать то она заблокируется
#ifdef _WIN64
			DetourAttach(&(PVOID&)DynamicTarget, Lab2Hook_x64);
#else
			DetourAttach(&(PVOID&)DynamicTarget, Lab2Hook_x86);
#endif
			if (DetourTransactionCommit() != NO_ERROR)
			{
				memset(buff, 0, 512);
				sprintf_s(buff, "ERROR: DetourTransactionCommit");
				send(client_socket, buff, strlen(buff) + 1, 0);
				// Отправляем сообщение об ошибке и только потом закрываем
				closesocket(client_socket);
				WSACleanup();
				return 1;
			}
			// Если все прошло хорошо, то не закрываем соединение
			return 1;
		}
		// Действие: -hide
		else
		{
			// Избавляемся от операции, оставляем только имя
			buff[strlen(buff) - 1] = '\0';
			std::string hide_name(buff);
			// Производим подмену
			Hide hide(hide_name);
			if (hide.HideFile() == true)
			{
				memset(buff, 0, 512);
				sprintf_s(buff, "The file is hidden");
			}
			else
			{
				memset(buff, 0, 512);
				sprintf_s(buff, "The file is not hidden");
			}
			// Сообщаем об удаче или же провале операции
			send(client_socket, buff, strlen(buff) + 1, 0);
			closesocket(client_socket);
			WSACleanup();
			return 1;
		}
	}
	return 0;
}

Hook::Hook(DWORD reason)
{
	ul_reason_for_call = reason;
}

Hook::~Hook()
{
}