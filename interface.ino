#include "imu.h"

#define LED 13

#define MAXGRIP 80
#define MINGRIP 30
#define SENSBUFFSIZE 5
#define SENSCOUNT 4

#define DEBUG 0

double imuAngleX;
double imuAngleY;

uint8_t unsens_threshold = 10;
uint8_t zero_position = 180;
int16_t line_speed = 0;
int16_t side_speed = 0;
int16_t grip = MINGRIP;
bool enabled = true;

uint16_t sensbuffer[SENSBUFFSIZE];
uint16_t sensarray[SENSCOUNT];


void setup() {
    pinMode(LED, OUTPUT);
    Serial.begin(9600);

    imuStart();
}

void loop() {

    // обновляем данные с гироскопа
    imuUpdate();

    imuAngleY = imuGetY();
    imuAngleX = imuGetX();

    // вычисляем скорость
    // если угол меньше 170 - едем вперёд
    if (imuAngleY < (zero_position - unsens_threshold)) { 
        line_speed = ((zero_position - unsens_threshold) - imuAngleY) * 5;
        if (line_speed > 100)
            line_speed = 100;
    } 
    else {
        //если угол больше 190 - едем назад
        if (imuAngleY > ( zero_position + unsens_threshold)) {
            line_speed = (imuAngleY - (zero_position + unsens_threshold)) * -2;
            if (line_speed < -100)
                line_speed = -100;
        // иначе - стоим на месте
        } else {
            line_speed = 0;
        }  
    }

    // если угол меньше 170 - едем влево
    if (imuAngleX < (zero_position - unsens_threshold)) { 
        side_speed = ((zero_position - unsens_threshold) - imuAngleX) * 4;
        if (side_speed > 100)
            side_speed = 100;
    } 
    else {
        //если угол больше 190 - едем вправо
        if (imuAngleX > ( zero_position + unsens_threshold)) {
            side_speed = (imuAngleX - (zero_position + unsens_threshold)) * -4;
            if (side_speed < -100)
                side_speed = -100;
        // иначе - стоим на месте
        } else {
            side_speed = 0;
        }  
    }

    // получаем данные с датчиков изгиба. делаем выборку для усреднения
    for(int i=0; i<SENSCOUNT; i++){
        for(int j=0; j<SENSBUFFSIZE; j++){
            sensbuffer[j] = analogRead(i);
        }
        for(int j=0; j<SENSBUFFSIZE; j++){
            sensarray[i] +=sensbuffer[j];
        }
        sensarray[i] /=SENSBUFFSIZE;
    }

    // формируем значения для клешни 
    if(sensarray[0] > 300)
        sensarray[0] = 300;
    if(sensarray[0] < 200)
        sensarray[0] = 200;
    grip = map(sensarray[0], 200, 300, MINGRIP, MAXGRIP);

    if(sensarray[0] == 300 && sensarray[1] < 150){
        enabled = !enabled;
        Serial.print("e ");
        Serial.println(enabled);
        digitalWrite(LED, enabled);
        delay(2000);
    }

    if(enabled){
        Serial.print("c ");
        Serial.print(line_speed);
        Serial.print(" ");
        Serial.print(side_speed);
        Serial.print(" ");
        Serial.println(grip);
    }

    if(DEBUG){
        for(int i=0; i<4; i++){
            Serial.print("s");
            Serial.print(i);
            Serial.print(":");
            Serial.print(sensarray[i]);
            Serial.print(" ");     
        }
        Serial.print("Y:");
        Serial.print(imuAngleY);
        Serial.print(" X:");
        Serial.println(imuAngleX);
    }
    
    delay (100);
}

