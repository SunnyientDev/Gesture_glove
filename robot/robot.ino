#include "Messenger.h"
#include "Servo.h"

#define L_DIR_PIN 5
#define R_DIR_PIN 11
#define L_SPEED_PIN 6
#define R_SPEED_PIN 3
#define GRIP_PIN 8

#define FORWARD 0
#define BACKWARD 1
#define MAXGRIP 45
#define MINGRIP 15

#define DEBUG 0

int line_speed = 0;
int side_speed = 0;
int grip = MINGRIP;

int enabled = 1;
int r_speed = 0;
int l_speed = 0;


Messenger message = Messenger();
Servo gripservo; 

void calculateStates() {
	if(enabled){
		//вычисляем скорости левых и правых моторов из скоростей езды и поворота
		if(line_speed >= 0) {
			r_speed = (line_speed * 2.55) + (side_speed * 2);
			l_speed = (line_speed * 2.55) - (side_speed * 2);	
		}
		else {
			r_speed = (line_speed * 2.55) - (side_speed * 2);
			l_speed = (line_speed * 2.55) + (side_speed * 2);	
		}
		//если скорость правой стороны больше 255 - пропорционально тормозим левую сторону 
		if (r_speed > 255) {
			l_speed = l_speed - (r_speed - 255);
			r_speed = 255;
		} else if(r_speed < -255) {
			l_speed = l_speed + (r_speed + 255);
			r_speed = -255;
		}
		//если скорость левой стороны больше 255 - пропорционально тормозим правую сторону 
		if (r_speed > 255) {
			l_speed = l_speed - (r_speed - 255);
			r_speed = 255;
		} else if(r_speed < -255) {
			l_speed = l_speed + (r_speed + 255);
			r_speed = -255;
		}
		// обрезаем раскрыв клешни под максимальный
		if (grip > MAXGRIP)
			grip = MAXGRIP;
		if (grip > 0 && grip < MAXGRIP)
			grip = MINGRIP;
	} 
	else {
		// если выключеаемся - останавливаемся и выключаем серву
		line_speed = 0;
		side_speed = 0;
		r_speed = 0;
		l_speed = 0;
		grip = 0;
	}	
}

void updateStates(){
	// записываем скорость и направления вращения в порты
	if(r_speed >= 0) {
		digitalWrite(R_DIR_PIN, FORWARD);
	}
	else {
		digitalWrite(R_DIR_PIN, BACKWARD);
	}
	
	if(l_speed >= 0) {
		digitalWrite(L_DIR_PIN, FORWARD);
	}
	else {
		digitalWrite(L_DIR_PIN, BACKWARD);
	}

	analogWrite(R_SPEED_PIN, r_speed );
	analogWrite(L_SPEED_PIN, l_speed);
	// записываем степень раскрыва клешни
	gripservo.write(grip);
	// в режиме отладки выводим полученные значения в порт
	if(DEBUG){
		Serial.print("line_speed:");
		Serial.print(line_speed);
		Serial.print(" side_speed:");
		Serial.print(side_speed);
		Serial.print(" l_speed:");
		Serial.print(l_speed);
		Serial.print(" r_speed:");
		Serial.print(r_speed);
		Serial.print(" grip:");
		Serial.println(grip);
	}
	
}

void messageCompleted() {
	// если приходит строка вида "с * * *" - значит пришли скорости и степень раскрыва клешни
  	if (message.checkString("c")) {
	  	line_speed = message.readInt();
		side_speed = message.readInt();
		grip = message.readInt();
  	}
  	// если приходит строка вида "e *" - значит пришла команда включения/выключения
	else if (message.checkString("e")) {
  		enabled = message.readInt();
  	}
  	// вычисляем значения скоростей
  	calculateStates();
  	// записываем полученные значения в порты
	updateStates();
}

void setup() {
	// настраиваем пины
	pinMode(L_DIR_PIN, OUTPUT);
	pinMode(R_DIR_PIN, OUTPUT);
	// настраиваем послеловательный порт
	Serial.begin(9600);
	// подключаем серву
	gripservo.attach(GRIP_PIN);
	// подключаем обработчик команд
	message.attach(messageCompleted);
	// записываем стартовые значения в порты
	updateStates();
}

void loop() {
	// если пришла команда - обрабатываем её
	while ( Serial.available( ) ) 
		message.process(Serial.read( ) );

}
