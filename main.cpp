// Основной модуль SAT-solver программы
// Инициализирует и запускает процесс решения задачи выполнимости формули (SAT)

#include "solve.hpp"
#include <chrono>
#include <thread>
#include <iostream>

// Главная функция приложения
// argc - количество аргументов командной строки
// argv - массив аргументов командной строки
int main(int argc, char **argv)
{
	// Замер времени выполнения программы
	auto начало_выполнения = std::chrono::high_resolution_clock::now();
	
	// Запуск основной процедуры решения SAT задачи
	solve(argc, argv);
	
	// Замер времени окончания выполнения
	auto конец_выполнения = std::chrono::high_resolution_clock::now();
	
	// Вычисление полного времени исполнения в миллисекундах
	auto длительность_мс = std::chrono::duration_cast<std::chrono::milliseconds>(
		конец_выполнения - начало_выполнения
	);
	
	// Вывод информационного сообщения о времени выполнения (может быть полезным для отладки)
	if (argc > 1 && std::string(argv[argc - 1]) == "--verbose")
	{
		std::cerr << "Общее время выполнения: " << длительность_мс.count() << " ms\n";
	}
	
	return 0;
}
