
#include <MPU6050_tockn.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>

MPU6050 mpu6050(Wire);


StaticJsonBuffer<200> jsonBuffer;


int n;

SocketIOClient client;
const char* ssid = "Carlvin’s iPhone";
const char* password = "12345678";

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
    client.send("data","Waarde","Working");
  }
  
}

void setup() {
  Serial.begin(115200);
  setupMPU();
  setupWifi();
  setupSocket();
}

void loop() {
  mpu6050.update();
  String values = (String)mpu6050.getAngleX() +","+ (String)mpu6050.getAngleY() +","+ (String)mpu6050.getAngleZ();
  client.monitor();
  client.send("data","Waarde",values);

}
