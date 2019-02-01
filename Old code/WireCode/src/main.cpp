#include <Wire.h>
#include <Arduino.h>
#include <ESP8266WiFi.h>
#include <FirebaseArduino.h>
#include <ArduinoJson.h>


// FIREBASE VARIABLES
#define FIREBASE_HOST "ipmedth.firebaseio.com"
#define FIREBASE_AUTH "DzVSfnl0MxRuDqTYOKNfdiTduBuJSf8yyo7i2Mls"
#define WIFI_SSID "Carlvin's iPhone"
#define WIFI_PASSWORD "12345678"

// MPU6050 Variables 
const int MPU_addr=0x68;  // I2C address of the MPU-6050


int16_t AcX,AcY,AcZ,Tmp,GyX,GyY,GyZ; // MPU VALUES 1
int16_t gyroXoffset, gyroYoffset, gyroZoffset;// MPU VALUES 2
int16_t X,Y,Z;



  // Functions
  void setupMPU(){
    Wire.begin(D2,D1);
    Wire.beginTransmission(MPU_addr);
    Wire.write(0x6B);  // PWR_MGMT_1 register
    Wire.write(0);     // set to zero (wakes up the MPU-6050)
    Wire.endTransmission(true);
    Serial.println("MPU SETUP DONE");
  }

  // Reads MPU values and set them in MPU VALUES 1
  void readMPU(int delayTime){

    Wire.beginTransmission(MPU_addr);
    Wire.write(0x3B);  // starting with register 0x3B (ACCEL_XOUT_H)
    Wire.endTransmission(false);
    Wire.requestFrom(MPU_addr,14,true);  // request a total of 14 registers

    AcX=Wire.read()<<8|Wire.read();  // 0x3B (ACCEL_XOUT_H) & 0x3C (ACCEL_XOUT_L)    
    AcY=Wire.read()<<8|Wire.read();  // 0x3D (ACCEL_YOUT_H) & 0x3E (ACCEL_YOUT_L)
    AcZ=Wire.read()<<8|Wire.read();  // 0x3F (ACCEL_ZOUT_H) & 0x40 (ACCEL_ZOUT_L)
    Tmp=Wire.read()<<8|Wire.read();  // 0x41 (TEMP_OUT_H) & 0x42 (TEMP_OUT_L)
    GyX=Wire.read()<<8|Wire.read();  // 0x43 (GYRO_XOUT_H) & 0x44 (GYRO_XOUT_L)
    GyY=Wire.read()<<8|Wire.read();  // 0x45 (GYRO_YOUT_H) & 0x46 (GYRO_YOUT_L)
    GyZ=Wire.read()<<8|Wire.read();  // 0x47 (GYRO_ZOUT_H) & 0x48 (GYRO_ZOUT_L)
    
    /*Serial.print(" | offsetX = "); Serial.print(gyroXoffset);
    Serial.print(" | offsetY = "); Serial.print(gyroYoffset);
    Serial.print(" | offsetZ = "); Serial.println(gyroZoffset);*/ 
    delay(delayTime);

  }

  // Calculates MPU offsets and sets them in MPU VALUES 2
  void offsetMPU(){
      int16_t CalcX, CalcY, CalcZ; // Calculation variabless (Might need to be set with other variables at the top )
      int times = 0;
      int samples = 200;

      Serial.println("Offsetting");

      while(times < samples) {
        readMPU(100);
        CalcX += AcX / 65;
        CalcY += AcY / 65;
        CalcZ += AcZ / 65;
        times += 1;

      }

      gyroXoffset = CalcX / samples;
      gyroYoffset = CalcY / samples;
      gyroZoffset = CalcZ / samples;

  }

  void setMpuValues(int readStat){
    readMPU(100);
    if(readStat == 1){
      X = ((AcX/65) - gyroXoffset) * 0.02f;
      Y = ((AcY/65) - gyroYoffset) * 0.02f;
      //Z = (AcZ/65) - gyroZoffset;
    }
    else if (readStat = 2){
      X = ((AcX/65) - gyroXoffset);
      Y = ((AcY/65) - gyroYoffset);
    }

  }

  void printMPU(int readStat){
    setMpuValues(readStat);
    Serial.print(" | GyX = "); Serial.print(X);
    Serial.print(" | GyY = "); Serial.println(Y);
  }

  void setupWifi(){
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    Serial.print("connecting");
    while (WiFi.status() != WL_CONNECTED) {
      Serial.print(".");
      delay(500);
    }
    Serial.println();
    Serial.print("connected: ");
    Serial.println(WiFi.localIP());
    
    Firebase.begin(FIREBASE_HOST, FIREBASE_AUTH);
  }

  void setFirebaseData(){

    // get new values
    setMpuValues(1);

    // post values
    Firebase.setFloat("X", X);
    Firebase.setFloat("Y", Y);
    // handle error
    if (Firebase.failed()) {
        Serial.print("setting /number failed:");
        Serial.println(Firebase.error());  
        return;
    }



  }


void setup(){
  Serial.begin(115200);
  Serial.println("Setting up MPU");
  setupMPU();
  Serial.println("Calcing MPU Offset");
  offsetMPU();
  Serial.println("Seetup wifi");
  setupWifi();

}
void loop(){
  printMPU(1);
  setFirebaseData();
}
