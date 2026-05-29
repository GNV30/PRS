#ifndef _light_hpp_INCLUDED
#define _light_hpp_INCLUDED

// Заголовочный файл основного контроллера многопоточного SAT-solver
// Содержит определение главного класса light, управляющего рабочими потоками

#include "utils/paras.hpp"
#include "utils/parse.hpp"
#include "preprocess/preprocess.hpp"
#include <atomic>
#include <iostream>
#include <boost/thread.hpp>
#include <boost/thread/thread.hpp>
#include <boost/lockfree/spsc_queue.hpp>

typedef long long ll;

class basesolver;
class sharer;

extern std::atomic<int> terminated;
extern std::mutex mtx;

// Тип решателя используемого в системе
enum Solver_Type
{
	KISSAT
};

// Структура для сохранения информации о состоянии потока
struct thread_inf
{
	int id;           // Идентификатор потока
	int inf;          // Информационное значение (например, количество конфликтов)
	bool operator<(const thread_inf &other) const
	{
		return inf > other.inf;
	}
};

// Главный контроллер многопоточного SAT-solver
// Управляет инициализацией рабочих потоков, распределением задач и обменом информацией
struct light
{
public:
	light();
	~light();

	// Имя файла с входной задачей (формула CNF)
	char *filename;
	
	// Указатель на объект параметров конфигурации
	paras *opt;
	
	// Указатель на объект препроцессора
	preprocess *pre;
	
	// Тип каждого решателя в системе
	vec<int> solver_type;
	
	// Вектор рабочих процессов (решателей)
	vec<basesolver *> workers;
	
	// Вектор обменивателей информацией между потоками
	vec<sharer *> sharers;

	// Массив имен параметров конфигурации для каждого потока
	vec<char *> *configure_name;
	
	// Массив значений параметров конфигурации для каждого потока
	vec<double> *configure_val;

	// Финальный результат решения (10=SAT, 20=UNSAT, 0=UNKNOWN)
	int finalResult;
	
	// Период успешного решения и ID потока-победителя
	int winner_period;
	int winner_id;
	
	// Мьютекс для синхронизации доступа к данным о победителе
	mutable boost::mutex winner_mtx;
	
	// Максимальное время выполнения в секундах
	int maxtime;
	
	// Метод обновления информации о потоке-победителе
	// Обновляет winner_period и winner_id, если найден лучший результат
	void update_winner(int id, int period)
	{
		boost::mutex::scoped_lock lock(winner_mtx);
		if (period < winner_period || (period == winner_period && id < winner_id))
		{
			winner_period = period;
			winner_id = id;
		}
	}
	
	// Получение периода решения у потока-победителя (потокобезопасно)
	int get_winner_period()
	{
		boost::mutex::scoped_lock lock(winner_mtx);
		return winner_period;
	}
	
	// Методы для управления жизненным циклом решателя
	void arg_parse(int argc, char **argv);         // Парсинг аргументов командной строки
	void configure_from_file(const char *file);    // Загрузка конфигурации из файла
	void init_workers();                            // Инициализация рабочих процессов
	void diversity_workers();                       // Установка разнообразной конфигурации для каждого потока
	void parse_input();                             // Парсинг входного CNF файла во всех потоках
	int run();                                      // Запуск процесса решения
	void share();                                   // Инициирование обмена информацией между потоками
	int solve();                                    // Основной процесс решения SAT задачи
	void terminate_workers();                       // Корректное завершение всех рабочих потоков
};

#endif
