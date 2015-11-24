#ifndef DHT_h
#define DHT_h

#if defined(ARDUINO) && (ARDUINO >= 100)
#include <Arduino.h>
#else
#include <WProgram.h>
#endif

#define	DHT_OK				 0	//	Данные получены и обработаны без ошибок
#define	DHT_ERROR_CHECKSUM	-1	//	Ошибка контрольной суммы (данные прочтены не корректно, мешает другое устройство на той же шине или имеются помехи)
#define	DHT_ERROR_DATA		-2	//	Данные не соответствуют заявленным в datasheet на сенсоры
#define	DHT_ERROR_NO_REPLY	-3	//	Данных нет, датчик не реагирует или отсутствует

class DHT{
	public:
		int		read(int pinDHT);	//	функция "read", принимает значение int в переменную "pinDHT", и возвращает № ошибки типа int если она есть
		float 	hum;				//	значение влажности   в %
		float	tem;				//	значение температуры в °C
	private:
		int		model[];			//	внутренний массив с № типа сенсора 11 или 22 на конкретном выводе
		uint8_t	reply[6];			//	внутренний массив из 5ти байт, (тип данных: u - беззнаковое, int - целое, 8 - количество бит, _t - тип а не функция или что еще там...)
};

#endif