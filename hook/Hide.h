#pragma once
#include <string>
#include <Windows.h>
#include <string>
#include <windows.h>
#include <detours.h>
#include <iostream>

using namespace std;

/*	
	Изменяет поведение функции FindFirstFile, FindNextFile, CreateFile таким образом, 
	что для процесса пропадает указанный заранее файл. Для изменения поведения измены
	значения, возвращаемые этими функциями в соответствии с определенными входными параметрами.
*/
class Hide
{
private:
	string File_Name; // Имя файла
public:
	bool HideFile(); // Сокрытие файла
	Hide(string& FileName); // На вход конструктора имя файла
	~Hide();
};