
#include <MPU6050_tockn.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>
//#include <cstdlib>

MPU6050 mpu6050(Wire);


StaticJsonBuffer<200> jsonBuffer;


int n;

SocketIOClient client;
const char* ssid = "wifi2"; // Carlvin’s iPhone
const char* password = "lumc12345"; // 12345678

char host[] = "83.81.251.42";
int port = 8080;
extern String RID;
extern String Rname;
extern String Rcontent;

unsigned long previousMillis = 0;
long interval = 5000;
unsigned long lastreply = 0;
unsigned long lastsend = 0;
String JSON;
JsonObject& root = jsonBuffer.createObject();


// mpu values
String x,y,z;
float ZeroX, ZeroY, ZeroZ;


void setupMPU(){
  Wire.begin(D2,D1);
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);
}

void setupWifi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

void setupSocket(){
  client.connect(host, port);

  while(!client.connected()){
     Serial.println("Connection Failed, retrying");
     client.connect(host, port);
     delay(500);
    }
  
   
 /* if (!client.connect(host, port)) {
    Serial.println("connection failed");
    return;
  }*/
  
  if (client.connected())
  {
    Serial.println("Connected");
    client.send("data","waarden","Connected");
  }
  
}

void updateMPU(){
  mpu6050.update();
  x = mpu6050.getAngleX() - ZeroX;
  y = mpu6050.getAngleY() - ZeroY;
  z = mpu6050.getAngleZ() - ZeroZ;
  
  x = (String)x;
  y = (String)y;
  z = (String)z;

}

void setZeroing(){
  mpu6050.update();
  /*int valuesX;
  int valuesY;
  int valuesZ;

  for(int i = 0; i < 50; i++){
    mpu6050.update();
    valuesX += mpu6050.getAngleX();
    valuesY += mpu6050.getAngleY();
    valuesZ += mpu6050.getAngleZ();
	}*/

  ZeroX = mpu6050.getAngleX();
  ZeroY = mpu6050.getAngleY();
  ZeroZ = mpu6050.getAngleZ();

}

void printMPU(){
  updateMPU();
  Serial.println(x + ","+ y +","+ z);
}

void postValues(){
  updateMPU();
  client.monitor();
  /*client.send("data","x",x);
  client.send("data","y",y);
  client.send("data","z",z);*/
  client.send("data","waarden", x + "," + y + "," + z );
  delay(20);
}

void setup() {
  Serial.begin(115200);
  setupMPU();
  setZeroing();
  setupWifi();
  setupSocket();
}

void loop() {
  postValues();
}
