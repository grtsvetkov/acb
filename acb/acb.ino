#include <MsTimer2.h> //Библиотека таймера
#include <rim-PCD8544.h> //Библиотека дисплея
#include <Keypad.h> //Библиотека клавиатуры

#include "image.cpp" //Файлик с иконками

static PCD8544 lcd; //Дисплей. Конструктор смотреть в библиотеке. Там и пины прописаны

//Настраиваем библиотеку клавиатуры
const byte keypad_rows = 4; //Четыре строчки
const byte keypad_cols = 3; //Три колонки
char keys[keypad_rows][keypad_cols] = {
        {'1', '2', '3'},
        {'4', '5', '6'},
        {'7', '8', '9'},
        {'*', '0', '#'}
};

byte rowPins[keypad_rows] = {13, 12, 11, 10}; //Соединяем сроки с пинами для клавиатуры
byte colPins[keypad_cols] = {9, 8, 7}; //Соединяем столбцы с пинами для клавиатуры

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, keypad_rows, keypad_cols); //Инициализируем клавиатуру

unsigned long time = 0; //Текущее время (в секундах)

unsigned long alarm = 120; //Установленный будильник в секундах (по умолчанию на 00:00:05)

bool status = false; //Текущее состояние (будет ли включена кофеварка по будильнику)

bool cook_flag = false;
unsigned long cook = 10; //Установленное время приготовления (в секундах)
unsigned long cook_tmp = 0; //Текущее время приготовления (в секундах)

byte selected_menu = 0;  //Выбранный пунк меню
byte hover_menu = 0; //Подсвеченный пунк меню
byte hover_menu_count = 5; //Устанавливаем количество элементов в меню (главном)

byte currentTimeCursor = 0; //Текущая позиция курсора по отрисовке времени
char currentTime[4] = {'1', '3', '5', '6'}; //Текущие введенные цифры времени

//Коды шрифта цифр в инверсии (для дисплея)
byte inverseInt[10] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};

byte releAnalogPin = A0;

void secondPlus() {
    time++;

    if (time >= 86400) {
        time = 0;
    }
}

void setup() {

    MsTimer2::set(1000, secondPlus);
    MsTimer2::start();

    lcd.begin(84, 48); //Инициализируем дисплей
}

void loop() {

    if (time == alarm && status) {
        cookSet(true);
    }

    if (cook_flag) {
        lcd.clear();
        lcd.setCursor(0, 0);
        cook_tmp -= 1;

        lcd.drawBitmap(cook_tmp % 2 == 0 ? cofebreak1 : cofebreak2, 48, 6);

        if (cook_tmp == 0) {
            status = false;
            cookSet(false);
            lcd.clear();
        }

    } else if (hover_menu == 0 && selected_menu == 0) {
        //display_temperatureOnDHT(13); //Да, да, 13-й пин используется и для клавиатуры тоже.. Не хватает пинов...
    }

    char key = keypad.getKey(); //Смотрим, нажата ли кнопка на клавиатуре

    if (key) { //Кнопка нажата

        if (cook_flag) { //Если во время нажатия кнопки готовили
            cookSet(false); //Отменяем приготовление кофе
        }

        lcd.clear(); //Отчищаем экран

        onKeyPress(&key); //Реагируем на нажатие кнопок
    }

    display_menu(); //Отрисовывам экран
}

void display_temperatureOnDHT(unsigned char PinDHT) {

    unsigned char reply[6] = {0, 0, 0, 0, 0, 0};

    static byte model = 0;

    pinMode(PinDHT, OUTPUT);
    digitalWrite(PinDHT, LOW);
    delay(25);  //Прижимаем шину к земле на 25мс

    pinMode(PinDHT, INPUT); // Отпускаем шину

    if (pulseIn(PinDHT, HIGH, 600) == 0) { //Проверяем реакцию датчика на сигнал "старт"
        return;// -3; //Данных нет, датчик не реагирует или отсутствует
    }

    for (int i = 0, j = 0; i < 40; i++) { //Читаем ответ датчика (40бит данных)
        j = pulseIn(PinDHT, HIGH, 600);
        if (j) {
            reply[i / 8] <<= 1;
            reply[i / 8] += j > 45 ? 1 : 0;
        } else {
            return;// -2; //Данные не соответствуют заявленным в datasheet на сенсоры
        }
    }


    if (pulseIn(PinDHT, HIGH, 600) > 0) { //Проверяем не отправляет ли датчик в шину больше 40 бит данных
        return;// -2; //Данные не соответствуют заявленным в datasheet на сенсоры
    }

    if (((reply[0] + reply[1] + reply[2] + reply[3]) & 0xff) != reply[4]) { //Проверяем контрольную сумму
        return;// -1; //Ошибка контрольной суммы (данные прочтены не корректно, мешает другое устройство на той же шине или имеются помехи)
    }

    if (model == 0) {
        if (reply[1] || reply[3] || reply[2] & 0x80) { //Определяем модель сенсора
            model = 22;
        } else if (reply[0] > 3 || reply[2] > 4) {
            model = 11;
        }
    }

    //записываем полученные данные в переменные (с учётом модели и знака температуры)
    //float hum = float(reply[1]+(model==11?reply[0]: reply[0]<<8))*(model==11?1: 0.1 );
    float tem = float(reply[3] + (model == 11 ? reply[2] : (reply[2] & 0x7F) << 8)) *
                (model == 11 ? 1 : (reply[2] & 0x80 ? -0.1 : 0.1));


    lcd.setCursor(54, 0);
    lcd.drawBitmap(thermometer, 12, 2);
    lcd.print(tem, 0);
}

/**
 * Функция установки реле, отвечающее за приготовление кофе
 * @param flag
 */
void cookSet(bool flag) {
    if (cook_flag) {
        cook_tmp = cook;
        analogWrite(releAnalogPin, 0);
    } else {
        analogWrite(releAnalogPin, 255);
    }
}

/**
 * Функция отрисовки меню на дисплее
 */
void onKeyPress(char *key) {

    switch (selected_menu) {
        case 1:  //Если в меню "установка состояния"
            if (*key == '8') { //Если нажали клавишу 8, т.е. "вверх" по меню
                hover_menu++;
            } else if (*key == '2') { //Если нажали клавишу 2, т.е. "вниз" по меню
                hover_menu--;
            }

            if (hover_menu > hover_menu_count) { //Если "перелистнули" последний список меню
                hover_menu = 1; //Возвращаемся на первый
            } else if (hover_menu < 1) { //Если нажали "вверх" на первом список меню
                hover_menu = hover_menu_count; //Устанавливаем курсор на последний
            }

            if (*key == '5') { //Если нажали клавишу 5, т.е. "войти в меню"

                status = hover_menu == 1; //Записали статус

                return exitToMainMenu(&selected_menu); //Выходим в главное меню
            }
            break;

        case 2: //Если в меню "установка будильника"
            onKeyPressSetTime(key, &selected_menu, true,
                              &alarm); //Обработка события нажатий кнопки при установке времени
            break;

        case 3: //Если в меню "установка времени"
            onKeyPressSetTime(key, &selected_menu, true,
                              &time); //Обработка события нажатий кнопки при установке времени
            break;

        case 4: //Если в меню "настройка приготовления"
            onKeyPressSetTime(key, &selected_menu, false,
                              &cook); //Обработка события нажатий кнопки при установке времени
            break;

        default: //Если в главном меню
            if (*key == '8') { //Если нажали клавишу 8, т.е. "вверх" по меню
                hover_menu++;
            } else if (*key == '2') { //Если нажали клавишу 2, т.е. "вниз" по меню
                hover_menu--;
            }

            if (hover_menu > hover_menu_count) { //Если "перелистнули" последний список меню
                hover_menu = 1; //Возвращаемся на первый
            } else if (hover_menu < 1) { //Если нажали "вверх" на первом список меню
                hover_menu = hover_menu_count; //Устанавливаем курсор на последний
            }

            if (*key == '5') { //Если нажали клавишу 5, т.е. "войти в меню"

                if (hover_menu == 5) { //Если выбран пункт меню 5 (Выход)
                    hover_menu = 0; //Устанавливаем "окно состояния"
                    hover_menu_count = 5;
                }

                selected_menu = hover_menu; //Устанавливаем выбранный пункт меню
                hover_menu = 0; //Забываем указатель на меню
            }

            break;
    }
}

/**
 Функция обработки события нажатий кнопки при установке времени
*/
void onKeyPressSetTime(char *key, byte *selected_menu, bool formatHHMM, unsigned long *timer) {
    if (*key == '*') { //Сохраняем значение

        unsigned long test = getSecFromInput(&formatHHMM);

        if (test < 1000000) { //Нет ошибки
            *timer = test;
            return exitToMainMenu(selected_menu); //Выходим в главное меню
        } else { //Ошибка ввода
            currentTimeCursor = 0;
        }
    }
        //Если ввели цифру - нужно вставить его в одно из 4х полей ввода
    else if (*key == '0' || *key == '1' || *key == '2' || *key == '3' || *key == '4' || *key == '5' || *key == '6' ||
             *key == '7' || *key == '8' || *key == '9') {

        currentTime[currentTimeCursor] = *key; //Устанавливаем курсор в одно из 4х полей (хх:хх)

        currentTimeCursor++; //Перемешаем курсор на следующее поле ввода

        if (currentTimeCursor > 3) { //Если курсор (индекс, считаем от нуля) больше 3
            currentTimeCursor = 0; //Возвращаем курсор в начало
        }
    }
}

/**
 * Отобразить меню на дисплее
 */
void display_menu() {
    switch (selected_menu) {

        case 1: //установка состояния

            hover_menu_count = 2; //В этом подменю всего 2 пункта

            lcd.setCursor(0, 0);
            display_write("Установка");

            lcd.setCursor(0, 1);
            display_write("состояния:");

            if (hover_menu == 0) { //Если только вошли в меню - устанавливаем курсор в зависимости от текущего состояния
                hover_menu = status ? 1 : 2;
            }

            lcd.setCursor(0, 3);
            lcd.print((hover_menu == 1 ? ">>" : "  "));
            display_write("Заряжен");

            lcd.setCursor(0, 4);
            lcd.print((hover_menu == 2 ? ">>" : "  "));
            display_write("НE заряжен");

            break;

        case 2: //установка будильника

            lcd.setCursor(0, 0);
            display_write("Будильник:");
            display_timeSet();

            break;

        case 3: //установка времени

            lcd.setCursor(0, 0);
            display_write("Время:");
            display_timeSet();

            break;

        case 4: //установка времени приготовления

            lcd.setCursor(0, 0);
            display_write("Готовка (мин:сек):");
            display_timeSet();

            break;

        default: //главное меню, либо окно общего состояния

            if (hover_menu == 0) { //окно состояния. Отображение времени, будильника, будет ли включение.

                if (!cook_flag) {
                    lcd.setCursor(0, 0); //Будем показывать время
                    display_write("Время ");
                    lcd.setCursor(0, 1);
                    display_time(time, true); //Отображаем форматировано время

                    lcd.setCursor(0, 3); //Будем отображать будильник
                    display_time(alarm, false); //Отображаем форматировано время
                    lcd.print("  (");
                    display_time(alarm - time + (time > alarm ? 86400 : 0), false);
                    lcd.print(")");

                    //Показываем состояние
                    lcd.setCursor(0, 5);

                    display_write(status ? "Заряжен" : "НE заряжен");
                }

            } else { //Выбор пунктов в главном меню

                //Зарядить
                lcd.setCursor(0, 0);
                lcd.print((hover_menu == 1 ? ">>" : "  "));
                display_write("Зарядить");

                //Будильник
                lcd.setCursor(0, 1);
                lcd.print((hover_menu == 2 ? ">>" : "  "));
                display_write("Будильник");

                //Время
                lcd.setCursor(0, 2);
                lcd.print((hover_menu == 3 ? ">>" : "  "));
                display_write("Время");

                //Приготовление
                lcd.setCursor(0, 3);
                lcd.print((hover_menu == 4 ? ">>" : "  "));
                display_write("Приготовление");

                //Выход
                lcd.setCursor(0, 4);
                lcd.print((hover_menu == 5 ? ">>" : "  "));
                display_write("Выход");
            }

            break;
    }
}

void display_write(char *str) {

    //АБВГДЕЁЖЗИЙКЛМНОПРСТУФХЦЧШЩЪЫЬЭЮЯ
    //абвгдеёжзийклмнопрстуфхцчшщъыьэюя
    bool loopFlag = true;
    unsigned int i = 0;

    while (loopFlag) {

        unsigned char f = str[i];
        unsigned char s = str[i + 1];
        unsigned int T = (f << 8) + s;

        unsigned int gl = f;

        if (T >= 0xD090 || T == 0xD081) { //А = 53392 два байта, Ё = 53377 два байта

            unsigned char delta = 0xCFD0; //53200

            if (T == 0xD081) { //Ё
                delta = 0xCFD9;
            } else if (T == 0xD191) { //ё
                delta = 0xD0D9;
            } else if (f >= 0xD1) {
                delta = 0xD090;
            }

            gl = T - delta;

            i++;
        }

        lcd.write(gl);
        i++;

        if (f == 0 || f == '\0') {
            loopFlag = false;
        }
    }
}

void display_timeSet() {
    lcd.setCursor(0, 2);
    for (byte i = 0; i <= 3; i++) {

        byte dec = byte(currentTime[i]) - 48; //0 - 9  = 48 - 57

        if (currentTimeCursor == i) {
            lcd.write(inverseInt[dec]);
        } else {
            lcd.print(currentTime[i]);
        }

        if (i == 1) {
            lcd.print(':');
        }
    }

    lcd.setCursor(0, 4);
    lcd.print("* - ");

    display_write("Сохранить");
}

//Получить время (в секундах) из поля ввода (formatHHMM => чч:mm = true, мм:сс = false)
unsigned long getSecFromInput(bool *formatHHMM) {
    unsigned long dec[4] = {
            byte(currentTime[0]) - 48,
            byte(currentTime[1]) - 48,
            byte(currentTime[2]) - 48,
            byte(currentTime[3]) - 48
    };

    if (*formatHHMM) {
        if ((dec[0] * 10) + dec[1] <= 23) {
            if ((dec[2] * 10) + dec[3] <= 59) {
                if ((dec[0] * 36000) + (dec[1] * 3600) + (dec[2] * 600) + (dec[3] * 60) < 86400) {
                    return (dec[0] * 36000) + (dec[1] * 3600) + (dec[2] * 600) + (dec[3] * 60);
                }
            }
        }
    } else {
        if ((dec[0] * 10) + dec[1] <= 59) {
            if ((dec[2] * 10) + dec[3] <= 59) {
                if ((dec[0] * 600) + (dec[1] * 60) + (dec[2] * 10) + dec[3] < 3600) {
                    return (dec[0] * 600) + (dec[1] * 60) + (dec[2] * 10) + dec[3];
                }
            }
        }
    }

    return 100000000; //Ошибка
}

/**
 * Выход в главное меню
 * @param selected_menu 
 */
void exitToMainMenu(byte *selected_menu) {
    hover_menu = *selected_menu; //Устанавливаем курсор на нынешнем пункте
    *selected_menu = 0; //Выходим в главное меню
    hover_menu_count = 5;
}

/**
 * Отобразить время
 * @param sec количество секунд
 * @param show_sec флаг, отображать ли секунды
 */
void display_time(unsigned long sec, bool show_sec) {
    unsigned int hours = sec / 3600;
    unsigned int minutes = (sec / 60) % 60;
    unsigned int seconds = sec % 60;

    if (hours < 10) {
        lcd.print('0');
    }
    lcd.print(hours);
    lcd.print(':');

    if (minutes < 10) {
        lcd.print('0');
    }
    lcd.print(minutes);

    if (show_sec) {
        lcd.print(':');

        if (seconds < 10) {
            lcd.print('0');
        }
        lcd.print(seconds);
    }
}