#include "DHT.h"

int	DHT::read(int PinDHT){

//	читаем шину SDA

	reply[0] = reply[1] = reply[2] = reply[3] = reply[4] = 0; //обнуляем массив

	pinMode(PinDHT,OUTPUT); digitalWrite(PinDHT,LOW); delay(25); //	прижимаем шину к земле на 25мс

	pinMode(PinDHT,INPUT); //	отпускаем шину

	if(pulseIn(PinDHT,HIGH,600)==0){return -3;} //	проверяем реакцию датчика на сигнал "старт"

	for(int i=0,j=0; i<40; i++) { //	читаем ответ датчика (40бит данных)
	    j = pulseIn(PinDHT,HIGH,600);
	    if(j) {
	        reply[i/8]<<=1;
	        reply[i/8]+=j>45 ? 1 : 0;
	    } else {
	        return -2;
	    }
	}


	if( pulseIn(PinDHT,HIGH,600)>0 ) { //	проверяем не отправляет ли датчик в шину больше 40 бит данных
	    return -2;
	}

	if(((reply[0]+reply[1]+reply[2]+reply[3])&0xff)!=reply[4]) { //	проверяем контрольную сумму
	    return -1;
	}

	if(reply[1] || reply[3] || reply[2]&0x80) { //	определяем модель сенсора
	    model[PinDHT]=22;
	} else if(reply[0]>3 || reply[2]>4 ) {
	    model[PinDHT]=11;
	}


//	записываем полученные данные в переменные (с учётом модели и знака температуры)
	hum = float(reply[1]+(model[PinDHT]==11?reply[0]: reply[0]      <<8))*(model[PinDHT]==11?1:                    0.1 );
	tem = float(reply[3]+(model[PinDHT]==11?reply[2]:(reply[2]&0x7F)<<8))*(model[PinDHT]==11?1:(reply[2]&0x80?-0.1:0.1));
	return 0;
}
