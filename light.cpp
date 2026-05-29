#include <chrono>
#include "light.hpp"
#include <unistd.h>
#include <thread>
#include <iostream>

// Конструктор класса light
// Инициализирует все значения по умолчанию и создает новый объект параметров
light::light() : finalResult(0),
                 winner_period(1e9),
                 winner_id(-1),
                 maxtime(5000),
                 filename(nullptr),
                 opt(nullptr),
                 pre(nullptr),
                 configure_name(nullptr),
                 configure_val(nullptr)
{
    // Создание и инициализация объекта параметров конфигурации
    opt = new paras();
    opt->init_paras();
}

// Деструктор класса light
// Освобождает все выделенную память и очищает вектора
light::~light()
{
    // Удаление всех рабочих процессов
    for (int i = 0; i < workers.size(); i++)
        delete (workers[i]);
    workers.clear(true);

    // Удаление имен параметров конфигурации для каждого потока
    if (configure_name)
    {
        for (int i = 0; i < OPT(nThreads); i++)
        {
            for (int j = 0; j < configure_name[i].size(); j++)
                delete[] configure_name[i][j];
            configure_name[i].clear(true);
        }
        delete[] configure_name;
    }

    // Удаление значений параметров конфигурации
    if (configure_val)
    {
        for (int i = 0; i < OPT(nThreads); i++)
            configure_val[i].clear(true);
        delete[] configure_val;
    }

    // Удаление объектов параметров и препроцессора
    delete opt;
    delete pre;
}

// Загрузка конфигурации решателей из файла
// file - путь к файлу конфигурации
void light::configure_from_file(const char *file)
{
    // Если файл пуст, инициализируем пустые вектора конфигурации
    if (!strcmp(file, ""))
    {
        configure_name = new vec<char *>[OPT(nThreads)];
        configure_val = new vec<double>[OPT(nThreads)];
        return;
    }
    
    // Открытие и чтение файла конфигурации
    std::ifstream fin(file);
    char buf[1000];
    fin.getline(buf, 1000);
    
    // Парсинг первой строки для получения количества потоков и решателей
    char *p = buf + 6;
    int ws, ss, id = 0;
    p = read_int(p, &ws);
    p = read_int(p, &ss);
    opt->set_para("nThreads", ws);
    
    // Выделение памяти для конфигурационных вводов
    configure_name = new vec<char *>[ws];
    configure_val = new vec<double>[ws];
    
    // Чтение конфигурации каждого потока
    while (fin.getline(buf, 1000))
    {
        p = strtok(buf, " ");
        solver_type.push(0);
        
        // Парсинг пар "параметр=значение" для текущего потока
        while (p)
        {
            p = strtok(NULL, " ");
            if (!p)
                break;
                
            int l = strlen(p), pos = 0;
            for (int i = 1; i < l; i++)
                if (p[i] == '=')
                    pos = i;
                    
            // Выделение памяти и копирование имени параметра
            char *name = new char[pos];
            strncpy(name, p, pos);
            configure_name[id].push(name);
            
            // Парсинг и добавление значения параметра
            char *val = p + pos + 1;
            double v = atof(val);
            configure_val[id].push(v);
        }
        id++;
    }
}

// Парсинг аргументов командной строки
// argc - количество аргументов
// argv - массив аргументов
void light::arg_parse(int argc, char **argv)
{
    // Циклический обход всех аргументов командной строки
    for (int i = 1; i < argc; i++)
    {
        char *arg = argv[i];
        
        // Проверка, является ли аргумент флагом (начинается с --)
        if (arg[0] != '-' || arg[1] != '-')
        {
            // Если не флаг, это имя входного файла
            filename = arg;
            continue;
        }
        
        // Поиск позиции знака = в названии параметра
        int l = strlen(arg), pos = 0;
        for (int i = 2; i < l; i++)
            if (arg[i] == '=')
                pos = i;
                
        // Если = не найден, пропускаем обработку
        if (!pos)
            continue;
            
        // Извлечение имени параметра
        char name[50];
        strncpy(name, arg + 2, pos - 2);
        name[pos - 2] = '\0';
        
        // Извлечение значения параметра
        char *val = arg + pos + 1;
        
        // Установка параметра с преобразованием типа
        if (opt->map_int.count(name))
            opt->set_para(name, atoi(val));
        else if (opt->map_double.count(name))
            opt->set_para(name, atof(val));
        else
            opt->set_para(name, val);
    }
    
    // Синхронизация и применение всех установленных параметров
    opt->sync_paras();
}