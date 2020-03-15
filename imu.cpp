#include <Wire.h>
#include <Arduino.h>
#include "Kalman.h"
#include "imu.h"

#define START_ANGLE 180

Kalman kalmanX;
Kalman kalmanY;

uint8_t IMUAddress = 0x68;

/* IMU Data */
int16_t accX;
int16_t accY;
int16_t accZ;
int16_t tempRaw;
int16_t gyroX;
int16_t gyroY;
int16_t gyroZ;

double accXangle; // Angle calculate using the accelerometer
double accYangle;
double temp;
double gyroXangle = START_ANGLE; // Angle calculate using the gyro
double gyroYangle = START_ANGLE;
double compAngleX = START_ANGLE; // Calculate the angle using a Kalman filter
double compAngleY = START_ANGLE;
double kalAngleX; // Calculate the angle using a Kalman filter
double kalAngleY;

uint32_t timer;

void i2cWrite(uint8_t registerAddress, uint8_t data){
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.write(data);
  Wire.endTransmission(); // Send stop
}
uint8_t* i2cRead(uint8_t registerAddress, uint8_t nbytes) {
  uint8_t data[nbytes];
  Wire.beginTransmission(IMUAddress);
  Wire.write(registerAddress);
  Wire.endTransmission(false); // Don't release the bus
  Wire.requestFrom(IMUAddress, nbytes); // Send a repeated start and then release the bus after reading
  for(uint8_t i = 0; i < nbytes; i++)
    data [i]= Wire.read();
  return data;
}

void imuStart(){
  
  Wire.begin();

  i2cWrite(0x6B,0x00); // Disable sleep mode      
  kalmanX.setAngle(START_ANGLE); // Set starting angle
  kalmanY.setAngle(START_ANGLE);
  timer = micros();
}

void imuUpdate(){
  uint8_t* data = i2cRead(0x3B,14);
  accX = ((data[0] << 8) | data[1]);
  accY = ((data[2] << 8) | data[3]);
  accZ = ((data[4] << 8) | data[5]);
  tempRaw = ((data[6] << 8) | data[7]);
  gyroX = ((data[8] << 8) | data[9]);
  gyroY = ((data[10] << 8) | data[11]);
  gyroZ = ((data[12] << 8) | data[13]);

  /* Calculate the angls based on the different sensors and algorithm */
  accYangle = (atan2(accX,accZ)+PI)*RAD_TO_DEG;
  accXangle = (atan2(accY,accZ)+PI)*RAD_TO_DEG;  

  double gyroXrate = (double)gyroX/131.0;
  double gyroYrate = -((double)gyroY/131.0);
  gyroXangle += kalmanX.getRate()*((double)(micros()-timer)/1000000); // Calculate gyro angle using the unbiased rate
  gyroYangle += kalmanY.getRate()*((double)(micros()-timer)/1000000);

  kalAngleX = kalmanX.getAngle(accXangle, gyroXrate, (double)(micros()-timer)/1000000); // Calculate the angle using a Kalman filter
  kalAngleY = kalmanY.getAngle(accYangle, gyroYrate, (double)(micros()-timer)/1000000);
  timer = micros();
}

double imuGetY() {
  return kalAngleY;
}

double imuGetX(){
  return kalAngleX;
}