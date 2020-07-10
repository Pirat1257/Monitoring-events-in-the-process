#include "Inject.h"

int main(int argc, char* argv[])
{
	// Проверка на правильность аргументов
	if (argc < 5)
	{
		cout << "ERROR: Bad arguments" << endl;
		return 0;
	}
	// Подготовка аргуемнтов
	list <string> input;
	for (int i = 0; i < argc - 1; i++)
		input.push_back(argv[i + 1]);
	// Запуск inject
	Inject inject(input);
	return inject.start();
}
