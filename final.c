#include <stdio.h>    // Стандартна библиотека за входно-изходни операции
#include <stdlib.h>   // Библиотека за управление на паметта (malloc, realloc, free)
#include <string.h>   // Библиотека за манипулация на низове (strcmp, strcpy, strcspn)
#include <stdbool.h>  // Библиотека за работа с булеви променливи (true, false)
#include <ctype.h>    // Библиотека за проверка на символи (isdigit)

// Дефиниране на глобални константи
#define MAX_NAME 100               // Максимална дължина на имената
#define FILE_NAME "clinic_data.csv" // Име на файла за съхранение на данните

// Структура за един медицински запис
typedef struct {
    int id;                        // Уникален идентификатор на записа
    char patient_name[MAX_NAME];   // Име на пациента
    char doctor_name[MAX_NAME];    // Име на лекаря
    char date[15];                 // Дата във формат ГГГГ-ММ-ДД
    char time[10];                 // Час във формат ЧЧ:ММ
    char status[20];               // Статус (Active, Cancelled, Completed)
} Appointment;

// Глобални променливи за управление на базата данни
Appointment *db = NULL;            // Указател към динамичния масив от записи
int cap = 10;                      // Начален капацитет на масива
int count = 0;                     // Текущ брой записани часове

// Прототипи на всички функции в програмата
void clear_screen();               // Изчистване на терминала
void show_header(const char *t);   // Показване на декоративно заглавие
void load_data();                  // Зареждане на данни от файл при старт
void save_data();                  // Запис на данни във файл при изход
void add_appointment();            // Функция за добавяне на нов час
void list_all();                   // Функция за извеждане на всички часове
void sort_by_datetime();           // Алгоритъм за сортиране (Bubble Sort)
int binary_search_by_date(char *d); // Алгоритъм за бързо търсене (Binary Search)
void update_status();              // Промяна на статуса на час (анулиране)
void show_stats();                 // Показване на статистика за клиниката
void expand_db();                  // Динамично увеличаване на паметта
int generate_id();                 // Генериране на ново уникално ID
int validate_time(char *t);        // Валидация на формата на часа

int main(void) {
    // Първоначално заделяне на памет за базата данни
    db = malloc(cap * sizeof(Appointment));
    if (db == NULL) return 1;      // Прекратяване при липса на памет

    load_data();                   // Автоматично зареждане на данните от диска

    int choice;                    // Променлива за избора от менюто
    while (true) {                 // Основен цикъл на програмата
        clear_screen();            // Изчистване на екрана
        show_header("ADVANCED CLINIC SCHEDULER V3.0"); // Заглавие
        printf("1. New Appointment\n");               // Опция 1
        printf("2. List All (Auto-Sorted)\n");        // Опция 2
        printf("3. Binary Search by Date\n");         // Опция 3
        printf("4. Update/Cancel Appointment\n");      // Опция 4
        printf("5. System Statistics\n");             // Опция 5
        printf("6. Save and Exit\n");                 // Опция 6
        printf("\nEnter Choice (1-6): ");             // Подкана

        if (scanf("%d", &choice) != 1) {              // Проверка дали входът е число
            while (getchar() != '\n');                // Изчистване на буфера при грешка
            continue;                                 // Повторение на цикъла
        }
        getchar();                                    // Изчистване на символа за нов ред

        if (choice == 1) add_appointment();           // Извикване на добавяне
        else if (choice == 2) { sort_by_datetime(); list_all(); } // Сортиране и листинг
        else if (choice == 3) {                       // Логика за бързо търсене
            char d[15];                               // Буфер за търсената дата
            printf("Search Date (YYYY-MM-DD): ");     // Инструкция
            scanf("%s", d); getchar();                // Четене и чистене на буфера
            sort_by_datetime();                       // Сортиране преди търсене (задължително!)
            int idx = binary_search_by_date(d);       // Изпълнение на двоично търсене
            if (idx != -1) printf("\nFound! ID %d: %s at %s\n", db[idx].id, db[idx].patient_name, db[idx].time);
            else printf("\nNo appointments found on this date.\n");
        }
        else if (choice == 4) update_status();        // Извикване на промяна на статус
        else if (choice == 5) show_stats();           // Извикване на статистика
        else if (choice == 6) { save_data(); break; } // Запис и изход от цикъла

        printf("\nPress Enter to continue...");       // Пауза за потребителя
        getchar();                                    // Изчакване на натискане на клавиш
    }

    free(db);                                         // Освобождаване на заетата памет
    return 0;                                         // Успешен край
}

// Помощна функция за почистване на екрана
void clear_screen() {
#ifdef _WIN32                                         // Проверка за Windows среда
    system("cls");                                    // Windows команда
#else                                                 // Проверка за Unix/Linux/macOS
    system("clear");                                  // Unix команда
#endif
}

// Функция за визуализиране на заглавна лента
void show_header(const char *t) {
    printf("==================================================\n");
    printf("  %s\n", t);                              // Печат на заглавието
    printf("==================================================\n");
}

// Функция за добавяне на нов запис в системата
void add_appointment() {
    if (count == cap) expand_db();                    // Проверка за свободна памет
    Appointment a;                                    // Временна променлива
    a.id = generate_id();                             // Присвояване на ID
    printf("Patient Name: ");
    fgets(a.patient_name, MAX_NAME, stdin);           // Четене на име с интервали
    a.patient_name[strcspn(a.patient_name, "\n")] = 0; // Премахване на знака за нов ред
    printf("Doctor Name: ");
    fgets(a.doctor_name, MAX_NAME, stdin);           // Четене на име на лекар
    a.doctor_name[strcspn(a.doctor_name, "\n")] = 0; // Премахване на знака за нов ред
    printf("Date (YYYY-MM-DD): "); scanf("%s", a.date); // Четене на дата
    while (true) {                                    // Цикъл за валидация на часа
        printf("Time (HH:MM): "); scanf("%s", a.time); // Четене на час
        if (validate_time(a.time)) break;             // Проверка на формата
        printf("Invalid HH:MM format!\n");            // Грешка при лош формат
    }
    getchar();                                        // Изчистване на буфера
    strcpy(a.status, "Active");                       // Начален статус
    db[count++] = a;                                  // Записване в масива и инкрементиране
    printf("[SUCCESS] Appointment %d created.\n", a.id);
}

// Сортиране по метода на мехурчето (Bubble Sort)
void sort_by_datetime() {
    for (int i = 0; i < count - 1; i++) {             // Външен цикъл през масива
        for (int j = 0; j < count - i - 1; j++) {     // Вътрешен цикъл за сравнение
            char dt1[35], dt2[35];                    // Буфери за сравнение
            sprintf(dt1, "%s %s", db[j].date, db[j].time);     // Сливане на дата и час 1
            sprintf(dt2, "%s %s", db[j+1].date, db[j+1].time); // Сливане на дата и час 2
            if (strcmp(dt1, dt2) > 0) {               // Ако първият е след втория
                Appointment temp = db[j];             // Размяна чрез временна променлива
                db[j] = db[j+1];                      // Местене на по-малкия напред
                db[j+1] = temp;                       // Местене на по-големия назад
            }
        }
    }
}

// Реализация на Двоично търсене (Binary Search)
int binary_search_by_date(char *target_date) {
    int low = 0;                                      // Начален индекс
    int high = count - 1;                             // Краен индекс
    while (low <= high) {                             // Докато има диапазон за търсене
        int mid = low + (high - low) / 2;             // Намиране на средата (безопасно)
        int cmp = strcmp(db[mid].date, target_date);  // Сравняване на датите
        if (cmp == 0) return mid;                     // Намерено съвпадение!
        if (cmp < 0) low = mid + 1;                   // Преместване надясно
        else high = mid - 1;                          // Преместване наляво
    }
    return -1;                                        // Връщане на -1 при липса
}

// Функция за визуализация на таблица с данни
void list_all() {
    printf("\n%-5s | %-15s | %-12s | %-10s | %-5s | %-10s\n", "ID", "Patient", "Doctor", "Date", "Time", "Status");
    printf("----------------------------------------------------------------------\n");
    for (int i = 0; i < count; i++) {                 // Цикъл през всички записи
        printf("%-5d | %-15.15s | %-12.12s | %-10s | %-5s | %-10s\n",
               db[i].id, db[i].patient_name, db[i].doctor_name,
               db[i].date, db[i].time, db[i].status); // Форматиран печат
    }
}

// Функция за актуализация на статуса на час
void update_status() {
    int target_id;                                    // ID за промяна
    printf("Enter Appointment ID: "); scanf("%d", &target_id); getchar();
    for (int i = 0; i < count; i++) {                 // Линейно търсене по ID
        if (db[i].id == target_id) {                  // При намиране
            printf("New Status (Active/Cancelled/Done): ");
            scanf("%19s", db[i].status); getchar();    // Запис на нов статус
            printf("Update complete.\n"); return;      // Изход
        }
    }
    printf("ID %d not found.\n", target_id);          // При липса на такова ID
}

// Функция за извеждане на обобщена информация
void show_stats() {
    int act = 0;                                      // Брояч за активни часове
    for (int i = 0; i < count; i++) {                 // Цикъл за проверка
        if (strcmp(db[i].status, "Active") == 0) act++; // Броене на активни
    }
    show_header("CLINIC REPORT");                     // Заглавие на репорта
    printf("Total Records: %d\n", count);             // Общо записи
    printf("Active Appointments: %d\n", act);         // Активни
    printf("Inactive/Cancelled: %d\n", count - act);  // Други
}

// Функция за динамично разширяване на капацитета на масива
void expand_db() {
    cap *= 2;                                         // Удвояване на размера
    Appointment *tmp = realloc(db, cap * sizeof(Appointment)); // Релокация
    if (!tmp) exit(1);                                // Фатална грешка при липса на RAM
    db = tmp;                                         // Присвояване на новия адрес
}

// Функция за автоматично генериране на ID
int generate_id() {
    if (count == 0) return 1001;                      // Начално ID за първия запис
    int max = 0;                                      // Търсене на най-високото ID
    for(int i=0; i<count; i++) if(db[i].id > max) max = db[i].id;
    return max + 1;                                   // Следващо ID
}

// Валидация на формат на часа чрез символна проверка
int validate_time(char *t) {
    if (strlen(t) != 5 || t[2] != ':') return 0;      // Проверка на дължина и разделител
    for (int i=0; i<5; i++) if (i != 2 && !isdigit(t[i])) return 0; // Проверка за цифри
    return 1;                                         // Валиден формат
}

// Постоянно съхранение: Запис в CSV файл
void save_data() {
    FILE *f = fopen(FILE_NAME, "w");                  // Отваряне за писане (презаписване)
    if (!f) return;                                   // Прекратяване при грешка
    for (int i = 0; i < count; i++) {                 // Запис на всеки ред
        fprintf(f, "%d,%s,%s,%s,%s,%s\n",
                db[i].id, db[i].patient_name, db[i].doctor_name,
                db[i].date, db[i].time, db[i].status); // Формат за CSV
    }
    fclose(f);                                        // Затваряне на дескриптора
    printf("\n[SYSTEM] %d records saved to %s.\n", count, FILE_NAME);
}

// Постоянно съхранение: Зареждане от CSV файл
void load_data() {
    FILE *f = fopen(FILE_NAME, "r");                  // Отваряне за четене
    if (!f) return;                                   // Ако файла го няма, продължаваме на чисто
    while (!feof(f)) {                                // До края на файла
        if (count == cap) expand_db();                // Проверка на паметта
        Appointment t;                                // Временна структура
        // Форматирано четене на CSV редове
        if (fscanf(f, "%d,%[^,],%[^,],%[^,],%[^,],%[^\n]\n",
            &t.id, t.patient_name, t.doctor_name, t.date, t.time, t.status) == 6) {
            db[count++] = t;                          // Добавяне в масива
        }
    }
    fclose(f);                                        // Затваряне на файла
}
//Мариан Пелтеков
