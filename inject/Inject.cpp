#include "Inject.h"

/*------------------DLL_инъекция------------------*/
bool Inject::inject_dll()
{
	// Path to hook.dll
	char DllPath[MAX_PATH] = { 0 };
	GetModuleFileNameA(NULL, DllPath, MAX_PATH);
	for (int i = strlen(DllPath); DllPath[i] != '\\'; i--)
		DllPath[i] = 0;
	strcat_s(DllPath, "hook.dll");

	// Open a handle to target process
	HANDLE hProcess = OpenProcess(PROCESS_ALL_ACCESS, FALSE, Target_Process_PID);
	if (hProcess == NULL)
	{
		cout << "ERROR: Process with this name or PID not found, code: " << GetLastError() << endl;
		return false;
	}

	// Allocate memory for the dllpath in the target process
	// length of the path string + null terminator
	LPVOID pDllPath = VirtualAllocEx(hProcess, 0, strlen(DllPath) + 1,
		MEM_COMMIT, PAGE_READWRITE);
	if (pDllPath == NULL)
	{
		cout << "ERROR: VirtualAllocEx, code: " << GetLastError() << endl;
		CloseHandle(hProcess);
		return false;
	}

	// Write the path to the address of the memory we just allocated
	// in the target process
	if (WriteProcessMemory(hProcess, pDllPath, (LPVOID)DllPath,
		strlen(DllPath) + 1, 0) == NULL)
	{
		cout << "ERROR: WriteProcessMemory, code: " << GetLastError() << endl;
		CloseHandle(hProcess);
		return false;
	}

	// Create a Remote Thread in the target process which
	// calls LoadLibraryA as our dllpath as an argument -> program loads our dll
	if (CreateRemoteThread(hProcess, 0, 0,
		(LPTHREAD_START_ROUTINE)GetProcAddress(GetModuleHandleA("Kernel32.dll"),
			"LoadLibraryA"), pDllPath, 0, 0) == NULL)
	{
		cout << "ERROR: CreateRemoteThread, code: " << GetLastError() << endl;
		CloseHandle(hProcess);
		return false;
	}

	CloseHandle(hProcess);
	return true;
}

/*------------------Настройка_подключения------------------*/
bool Inject::network()
{
	WSADATA wsaData;
	sockaddr_in server_addr; // Адрес сервера
	// Initialize Winsock
	WSAStartup(MAKEWORD(2, 2), &wsaData);
	// Create a SOCKET for connecting to server
	listen_socket = socket(AF_INET, SOCK_STREAM, IPPROTO_TCP);
	// Setup the TCP listening socket
	server_addr.sin_family = AF_INET;
	InetPton(AF_INET, _T("127.0.0.1"), &server_addr.sin_addr.s_addr);
	server_addr.sin_port = htons(9000);
	if (bind(listen_socket, (sockaddr*)&server_addr, sizeof(server_addr)) == SOCKET_ERROR)
	{
		cout << "ERROR: bind, code: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}
	if (listen(listen_socket, 50) == SOCKET_ERROR)
	{
		cout << "ERROR: listen, code: " << WSAGetLastError() << endl;
		closesocket(listen_socket);
		WSACleanup();
		return false;
	}
	return true;
}

/*------------------Обмен_сообщениями_с_клиентом------------------*/
void Inject::work_with_client()
{
	// Accept a client socket
	client_socket = accept(listen_socket, NULL, NULL);
	char buff[512] = { 0 }; // Буфер приема и отправки
	int size = recv(client_socket, buff, 512, 0);
	if (size > 0)
		cout << buff << endl;
	memset(buff, 0, 512); // Обнуляем буфер
	strcat_s(buff, Function_File_Name.c_str()); // Записываем в начало имя функции или файла
	// Операция: -func
	if (func_hide == 0)
	{
		strcat_s(buff, "0"); // Номер операции заносится в конец
		send(client_socket, buff, strlen(buff) + 1, 0); // Отправляем команду

		while (1) // Необходимо принимать сообщения от клиента о вызовах функции
		{
			size = recv(client_socket, buff, 512, 0);
			if (size > 0) // Выводим время
			{
				cout << "func called: " << buff << endl;
				memset(buff, 0, 512);
			}
			else if (size == 0) // Соединение закрыто
			{
				cout << "Clode connection..." << endl;
				break;
			}
			else // Программа была закрыта
			{
				cout << "ERROR: recv, code: " << WSAGetLastError() << endl;
				break;
			}
		}
	}
	// Операция: -hide
	else
	{
		strcat_s(buff, "1"); // Номер операции заносится в конец
		send(client_socket, buff, strlen(buff) + 1, 0); // Отправляем команду
		size = recv(client_socket, buff, 512, 0); // Получаем информацию о выполнении операции
		if (size > 0)
			cout << buff << endl;
		// Нет необходимости принимать сообщения от клиента
	}
	closesocket(client_socket);
	WSACleanup();
}

/*------------------Главная_функция------------------*/
int Inject::start()
{
	// Необходимость привилегий администратора
	if (IsUserAnAdmin() == false)
	{
		cout << "ERROR: Administrator privileges required" << endl;
		return 0;
	}
	// Подготовка к подключению
	if (network() == false)
		return 0;
	// DLL инъекция
	if (inject_dll() == false)
		return 0;
	// Работа с клиентом
	work_with_client();
	return 0;
}

/*------------------Получение_PID_по_имени------------------*/
DWORD Inject::get_PID_by_name(string processname)
{
	HANDLE hProcessSnap;
	PROCESSENTRY32 pe32;
	DWORD result = NULL;

	// Take a snapshot of all processes in the system.
	hProcessSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);
	if (INVALID_HANDLE_VALUE == hProcessSnap) return(FALSE);

	pe32.dwSize = sizeof(PROCESSENTRY32); // <----- IMPORTANT

	// Retrieve information about the first process,
	// and exit if unsuccessful
	if (!Process32First(hProcessSnap, &pe32))
	{
		CloseHandle(hProcessSnap);          // clean the snapshot object
		printf("!!! Failed to gather information on system processes! \n");
		return(NULL);
	}

	do
	{
		_bstr_t dummy(pe32.szExeFile);
		const char* c = dummy;
		if (0 == strcmp(processname.c_str(), c))
		{
			result = pe32.th32ProcessID;
			break;
		}
	} while (Process32Next(hProcessSnap, &pe32));

	CloseHandle(hProcessSnap);
	return result;
}

Inject::Inject(list <string> input)
{
	list <string> :: iterator it = input.begin();
	// Целевой процесс выбирается по значению PID
	if (*it == "-pid")
		pid_name = 0;
	else // Целевой процесс выбирается по имени
		pid_name = 1;
	it++;
	// Сохранение значения PID
	if (pid_name == 0)
		Target_Process_PID = atoi((*it).c_str());
	else
		Target_Process_PID = get_PID_by_name(*it);
	it++;
	// Необходимо отследить функцию
	if (*it == "-func")
		func_hide = 0;
	else // Необходимо скрыть процесс
		func_hide = 1;
	it++;
	// Сохраняем название функции или процесса
	Function_File_Name = *it;
	input.clear();
}

Inject::~Inject()
{
}