#include <rim-PCD8544.h> // Библиотека дисплея
#include <Keypad.h> //Библиотека клавиатуры

static PCD8544 lcd; //Дисплей. Конструктор смотреть в библиотеке. Там и пины прописаны

//Настраиваем библиотеку клавиатуры
const byte keypad_rows = 4; //Четыре строчки
const byte keypad_cols = 3; //Три колонки
char keys[keypad_rows][keypad_cols] = {
  {'1','2','3'},
  {'4','5','6'},
  {'7','8','9'},
  {'*','0','#'}
};

byte rowPins[keypad_rows] = {13, 12, 11, 10}; //connect to the row pinouts of the keypad
byte colPins[keypad_cols] = {9, 7, 8}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad( makeKeymap(keys), rowPins, colPins, keypad_rows, keypad_cols); //Инициализируем клавиатуру

unsigned long ltime = 0; //Текущее время (в миллисекундах)

unsigned long alarm = 36000; //Установленный будильник в секундах (по умолчанию на 10:00:00)

const unsigned int global_delay = 100; //ms - глобальная задержка в каждом loop`е.

bool status = false; //Текущее состояние (будет ли включена кофеварка по будильнику)

byte selected_menu = 0;  //Выбранный пунк меню
byte hover_menu = 0; //Подсвеченный пунк меню
byte hover_menu_count = 5; //Устанавливаем количество элементов в меню

byte curretTimeCursor = 0; //Текущая позиция курсора по отрисовке времени
char currentTime[4] = {'1', '3', '5', '6'}; //Текущие введенные цифры времени
byte inverseInt[10] = {0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16, 0x17, 0x18, 0x19};
/**
 * Меню:
 * - окно состояния (0)  (отображение времени, будильника, будет ли включение)
 * - установка состояния (1)
 * - установка будильника (2)
 * - установка времени (3)
 * - настройка дисплея (4)
 * - выход (5) (только для hover_menu)
 * 
 * Выбранный пунк меню по коду такой же, как и выше.
 * Т.е. если мы находимся на главном экране, то selected_menu == 0 && hover_menu == 0
 * Если мы находимся на этапе выбора меню, то selected_menu == 0 && hover_menu == 1
 * Если мы находимся на этапе выбора меню и подсвечено "установка будильника", то selected_menu == 0 && hover_menu == 2
 * Если мы находимся в окне "установка времени", то  selected_menu == 3 && hover_menu == 0
 * И так далее
 */

/*
 * Небольшое отступление как отображать русские буквы на дисплее

   {0xC0, "А"}, {0xC1, "Б"}, {0xC2, "В"}, {0xC3, "Г"}, {0xC4, "Д"}, {0xC5, "Е"}, {0xC6, "Ж"}, {0xC7, "З"}, {0xC8, "И"}, {0xC9, "Й"}, {0xCA, "К"},
   {0xCB, "Л"}, {0xCC, "М"}, {0xCD, "Н"}, {0xCE, "О"}, {0xCF, "П"}, {0xD0, "Р"}, {0xD1, "С"}, {0xD2, "Т"}, {0xD3, "У"}, {0xD4, "Ф"}, {0xD5, "Х"},
   {0xD6, "Ц"}, {0xD7, "Ч"}, {0xD8, "Ш"}, {0xD9, "Щ"}, {0xDA, "Ъ"}, {0xDB, "Ы"}, {0xDC, "Ь"}, {0xDD, "Э"}, {0xDE, "Ю"}, {0xDF, "Я"}, {0xE0, "а"},
   {0xE1, "б"}, {0xE2, "в"}, {0xE3, "г"}, {0xE4, "д"}, {0xE5, "е"}, {0xE6, "ж"}, {0xE7, "з"}, {0xE8, "и"}, {0xE9, "й"}, {0xEA, "к"}, {0xEB, "л"},
   {0xEC, "м"}, {0xED, "н"}, {0xEE, "о"}, {0xEF, "п"}, {0xF0, "р"}, {0xF1, "с"}, {0xF2, "т"}, {0xF3, "у"}, {0xF4, "ф"}, {0xF5, "х"}, {0xF6, "ц"},
   {0xF7, "ч"}, {0xF8, "ш"}, {0xF9, "щ"}, {0xFA, "ъ"}, {0xFB, "ы"}, {0xFC, "ь"}, {0xFD, "э"}, {0xFE, "ю"}, {0xFF, "я"}

//Библиотека на js для отображения HEX кодов русских символов (для дисплея)
var trans = [];
for (var i = 0x410; i <= 0x44F; i++) trans[i] = i - 0x350; // А-Яа-я
trans[0x401] = 0xA8;    // Ё
trans[0x451] = 0xB8;    // ё

to_win_1251 = function(str) {
  var ret = [];
  for (var i = 0; i < str.length; i++) {
    var n = str.charCodeAt(i);
    if (typeof trans[n] != 'undefined') n = trans[n];
    if (n <= 0xFF) {
      ret.push('lcd.write(0x'+n.toString(16)+');');
    }
  }
  console.log(ret.join(' '));
}
to_win_1251('Привет');

Фишка в моём шрифте - инверсированные цифры 0-9
{ 0xC1, 0xAE, 0xB6, 0xBA, 0xC1},  //0x30 inverse 0 
{ 0xFF, 0xBD, 0x80, 0xBF, 0xFF},  //0x31 inverse 1
{ 0x8D, 0xB6, 0xB6, 0xB6, 0xB9},  //0x32 inverse 2
{ 0xDE, 0xBE, 0xB6, 0xB2, 0xCC},  //0x33 inverse 3
{ 0xE7, 0xEB, 0xED, 0x80, 0xEF},  //0x34 inverse 4
{ 0xD8, 0xBA, 0xBA, 0xBA, 0xC6},  //0x35 inverse 5
{ 0xC1, 0xB6, 0xB6, 0xB6, 0xCD},  //0x36 inverse 6
{ 0xBE, 0xDE, 0xEE, 0xF6, 0xF8},  //0x37 inverse 7
{ 0xC9, 0xB6, 0xB6, 0xB6, 0xC9},  //0x38 inverse 8
{ 0xD9, 0xB6, 0xB6, 0xB6, 0xC1},  //0x39 inverse 9
*/

void setup() {

  Serial.begin(9600);
  
  lcd.begin(84, 48); //Инициализируем дисплей
}


void loop() {

  ltime++; //Прибавляем время

  char key = keypad.getKey(); //Смотрим, нажата ли кнопка на клавиатуре
  
  if (key) { //Кнопка нажата
    
    lcd.clear(); //Отчищаем экранчик
    
    menuKeyPress(&key); //Реагируем на нажатие клавиши
  }

  showMenu(); //Отрисовывам экран

  delay(global_delay); //Небольшая задержка
  
}

/**
 * Функция отрисовки меню на дисплее
 */

void menuKeyPress(char *key) {

  switch(selected_menu) {
    case 1:  //Если в меню "установка состояния"
        if(*key == '8') { //Если нажали клавишу 8, т.е. "вверх" по меню
          hover_menu++;
        } else if(*key == '2') { //Если нажали клавишу 2, т.е. "вниз" по меню
          hover_menu--;
        }

        if(hover_menu > hover_menu_count) { //Если "перелистнули" последний список меню
          hover_menu = 1; //Возвращаемся на первый
        } else if(hover_menu < 1) { //Если нажали "вверх" на первом список меню
          hover_menu = hover_menu_count; //Устанавливаем курсор на последний
        }

        if(*key == '5') { //Если нажали клавишу 5, т.е. "войти в меню"

          status = hover_menu == 1 ? true : false; //Записали статус

          return exitToMainMenu( &selected_menu ); //Выходим в главное меню
        }
    break;

    case 2: //Если в меню "установка будильника"
        if(*key == '*') {
          //SAVETIME!!!!!!

          return exitToMainMenu( &selected_menu ); //Выходим в главное меню
        } else if(*key == '0' || *key == '1' || *key == '2' || *key == '3' || *key == '4' || *key == '5' || *key == '6' || *key == '7' || *key == '8' || *key == '9') {
          currentTime[curretTimeCursor] = *key;

          curretTimeCursor++;

          if(curretTimeCursor > 3) {
            curretTimeCursor = 0;
          }
        }
    break;

    case 3: //Если в меню "установка времени"

    break;

    case 4: //Если в меню "настройка дисплея"

    break;
    
    default: //Если в главном меню
        if(*key == '8') { //Если нажали клавишу 8, т.е. "вверх" по меню
          hover_menu++;
        } else if(*key == '2') { //Если нажали клавишу 2, т.е. "вниз" по меню
          hover_menu--;
        }

        if(hover_menu > hover_menu_count) { //Если "перелистнули" последний список меню
          hover_menu = 1; //Возвращаемся на первый
        } else if(hover_menu < 1) { //Если нажали "вверх" на первом список меню
          hover_menu = hover_menu_count; //Устанавливаем курсор на последний
        }

        if(*key == '5') { //Если нажали клавишу 5, т.е. "войти в меню"

          if(hover_menu == 5) { //Если выбран пункт меню 5 (Выход)
            hover_menu = 0; //Устанавливаем "окно состояния"
            hover_menu_count = 5;
          }

          selected_menu = hover_menu; //Устанавливаем выбранный пункт меню
          hover_menu = 0; //Забываем указатель на меню
        }

    break;  
  }
}

void exitToMainMenu(byte *selected_menu) {
    hover_menu = *selected_menu; //Устанавливаем курсор на нынешнем пункте
    *selected_menu = 0; //Выходим в главное меню
    hover_menu_count = 5;
}

void showMenu() {
  switch (selected_menu) {

    case 1: //установка состояния

      hover_menu_count = 2;
    
      lcd.setCursor(0, 0);
      //Пишем "Установка"
      lcd.write(0xd3); lcd.write(0xf1); lcd.write(0xf2); lcd.write(0xe0); lcd.write(0xed); lcd.write(0xee); lcd.write(0xe2); lcd.write(0xea); lcd.write(0xe0);

      lcd.setCursor(0, 1);
      //Пишем "состояния:"
      lcd.write(0xf1); lcd.write(0xee); lcd.write(0xf1); lcd.write(0xf2); lcd.write(0xee); lcd.write(0xff); lcd.write(0xed); lcd.write(0xe8); lcd.write(0xff); lcd.write(0x3a);


      if(hover_menu == 0) { //Если только вошли в меню - устанавливаем курсор в зависимости от текущего состояния
        hover_menu = status == true ? 1 : 2;
      }

      lcd.setCursor(0,3);
      lcd.print(( hover_menu == 1 ? ">>" : "  "));
      //Показываем "Заряжен"
      lcd.write(0xc7); lcd.write(0xe0); lcd.write(0xf0); lcd.write(0xff); lcd.write(0xe6); lcd.write(0xe5); lcd.write(0xed);

      lcd.setCursor(0,4);
      lcd.print(( hover_menu == 2 ? ">>" : "  "));
      //Показываем "НE заряжен"
      lcd.write(0xcd); lcd.write(0x45); lcd.write(0x20); lcd.write(0xe7); lcd.write(0xe0); lcd.write(0xf0); lcd.write(0xff); lcd.write(0xe6); lcd.write(0xe5); lcd.write(0xed);
      
    break;

    case 2: //установка будильника

      lcd.setCursor(0, 0);
      //Пишем "Будильник:"
      lcd.write(0xc1); lcd.write(0xf3); lcd.write(0xe4); lcd.write(0xe8); lcd.write(0xeb); lcd.write(0xfc); lcd.write(0xed); lcd.write(0xe8); lcd.write(0xea); lcd.write(0x3a);

      lcd.setCursor(0, 2);
      for(byte i = 0; i <= 3; i++) {

        /* 0 - 9  = 48 - 57 */
        byte dec = byte(currentTime[i]) - 48;

        Serial.println(dec);
        Serial.println(inverseInt[dec], HEX);
        
        if(curretTimeCursor == i) {
          lcd.write(inverseInt[dec]);
        } else {
          //lcd.write(inverseInt[dec]);
          lcd.print(currentTime[i]);
        }

        if(i == 1) {
          lcd.print(':');
        }
      }
        
      lcd.setCursor(0, 4);
      lcd.print("* - ");
      //Пишем "сохранить"
      lcd.write(0xf1); lcd.write(0xee); lcd.write(0xf5); lcd.write(0xf0); lcd.write(0xe0); lcd.write(0xed); lcd.write(0xe8); lcd.write(0xf2); lcd.write(0xfc);

    break;

    case 3: //установка времени

    break;

    case 4: //настройка дисплея

    break;
    
    default: //главное меню, либо окно общего состояния
      
      if(hover_menu == 0) { //окно состояния. Отображение времени, будильника, будет ли включение.

          unsigned long sec = ltime / ( 1000 / global_delay );
          
          lcd.setCursor(0,1); //Будем показывать время
          //Пишем слово "Время "
          lcd.write(0xc2); lcd.write(0xf0); lcd.write(0xe5); lcd.write(0xec); lcd.write(0xff); lcd.write(0x20);
          showTime( sec ); //Отображаем форматировано время

          lcd.setCursor(0, 3); //Будем отображать будильник
          showTime( alarm, false); //Отображаем форматировано время
          lcd.print("  (");
          showTime(alarm - sec + (sec > alarm ? 86400 : 0), false);
          lcd.print(")");

          //Показываем состояние
          lcd.setCursor(0, 5);

          if(status) { //Показываем "Заряжен"
            lcd.write(0xc7); lcd.write(0xe0); lcd.write(0xf0); lcd.write(0xff); lcd.write(0xe6); lcd.write(0xe5); lcd.write(0xed);
          } else { //Показываем "НE заряжен"
            lcd.write(0xcd); lcd.write(0x45); lcd.write(0x20); lcd.write(0xe7); lcd.write(0xe0); lcd.write(0xf0); lcd.write(0xff); lcd.write(0xe6); lcd.write(0xe5); lcd.write(0xed);
          }
          //lcd.setInverse(false);

        
      } else { //Выбор пунктов в главном меню

        //lcd.write(0xBB);

        //Зарядить
        lcd.setCursor(0,0);
        lcd.print(( hover_menu == 1 ? ">>" : "  "));
        lcd.write(0xc7); lcd.write(0xe0); lcd.write(0xf0); lcd.write(0xff); lcd.write(0xe4); lcd.write(0xe8); lcd.write(0xf2); lcd.write(0xfc);

        //Будильник
        lcd.setCursor(0,1);
        lcd.print(( hover_menu == 2 ? ">>" : "  "));
        lcd.write(0xc1); lcd.write(0xf3); lcd.write(0xe4); lcd.write(0xe8); lcd.write(0xeb); lcd.write(0xfc); lcd.write(0xed); lcd.write(0xe8); lcd.write(0xea);

        //Время
        lcd.setCursor(0,2);
        lcd.print(( hover_menu == 3 ? ">>" : "  "));
        lcd.write(0xc2); lcd.write(0xf0); lcd.write(0xe5); lcd.write(0xec); lcd.write(0xff);

        //Дислей
        lcd.setCursor(0,3);
        lcd.print(( hover_menu == 4 ? ">>" : "  "));
        lcd.write(0xc4); lcd.write(0xe8); lcd.write(0xf1); lcd.write(0xeb); lcd.write(0xe5); lcd.write(0xe9);

        //Выход
        lcd.setCursor(0,4);
        lcd.print(( hover_menu == 5 ? ">>" : "  "));
        lcd.write(0xc2); lcd.write(0xfb); lcd.write(0xf5); lcd.write(0xee); lcd.write(0xe4);
      }
      
    
    break;
  }
}

void showTime(unsigned long sec) {
  showTime(sec, true);
}

void showTime(unsigned long sec, bool show_sec) {
  unsigned int hours = sec / 3600;
  unsigned int minutes = (sec / 60) % 60;
  unsigned int seconds = sec % 60;
  
  if(hours < 10) {
    lcd.print('0');
  }
  lcd.print(hours);
  lcd.print(':');
  
  if(minutes < 10) {
    lcd.print('0');
  }
  lcd.print(minutes);

  if(show_sec) {
    lcd.print(':');
  
    if(seconds < 10) {
      lcd.print('0');
    }
    lcd.print(seconds);
  }
}




