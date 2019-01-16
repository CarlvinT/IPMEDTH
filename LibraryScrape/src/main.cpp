
#include <MPU6050_tockn.h>
#include <ArduinoJson.h>
#include <Arduino.h>
#include <Wire.h>
#include <ESP8266WiFi.h>
#include <SocketIOClient.h>


// REMOVE LATER
#include <WiFiUdp.h>
/*#include <string>
#include<iostream>
#include<sstream>*/
WiFiUDP Udp;
unsigned int localUdpPort = 4210;  // local port to listen on
char incomingPacket[255];  // buffer for incoming packets
char replyPacket[255];  
//char  replyPacket[] = "hello there";        




// MPU calibration and printing variables
MPU6050 mpu6050(Wire);
String x,y,z;
float xi,yi,zi;
float ZeroX, ZeroY, ZeroZ;


// Wifi info 
const char* ssid = "wifi2"; // Carlvin’s iPhone
const char* password = "lumc12345"; // 12345678


// Usefull socket.io variables 
SocketIOClient client;

  // server ip and port
char host[] = "83.81.251.42";
int port = 8080;

  // Json buffer for sending messages
StaticJsonBuffer<200> jsonBuffer;
String JSON;
JsonObject& root = jsonBuffer.createObject();



 int n; // not sure als het nodig is maar ben bang om het weg te halen.

  // used for reading incoming values
String incoming;
extern String RID;
extern String Rname;
extern String Rcontent;


// Initialize MPU and MPU Pins
void setupMPU(){
  Wire.begin(4,5);
  mpu6050.begin();
  // gyro offset was here
}

// Get values for Zeroing to create stable output
void setZeroing(){
  mpu6050.calcGyroOffsets(true);
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

// Get new values from MPU and Set filters to possible home foot movements
void updateMPU(){
  mpu6050.update();
  xi = mpu6050.getAngleX() - ZeroX;
  yi = mpu6050.getAngleY() - ZeroY;
  zi = mpu6050.getAngleZ() - ZeroZ;

  //https://www.verywellhealth.com/what-is-normal-range-of-motion-in-a-joint-3120361
  // Y = 20 to -50 
  // X = 35 to -25

  /*if(xi > 35.00){
    xi = 35.00;
  }
  else if (xi < -25.00){
    xi = -25.00;
  }*/

  /*if (yi > 20.00){
    yi = 20;
  }
  else if (yi < -50.00){
    yi = -50;
  }*/

  x = (String)xi;
  y = (String)yi;
  z = (String)zi;

}

// Prints MPU Values
void printMPU(){
  updateMPU();
  Serial.println(x + ","+ y +","+ z);
}

//Initialize and Connect to wifi 
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

// Initialize and Connect to Socket.io server
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
    //client.send("data","waarden","Connected");
  }
  
}
// REMOVE UDP LATER
void setupUDP(){
  Udp.begin(localUdpPort);
  Serial.printf("Now listening at IP %s, UDP port %d\n", WiFi.localIP().toString().c_str(), localUdpPort);
}

void postUDP(){
    /*std::ostringstream convert;
    float ji = 100.00;
    convert << ji;*/

    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    Udp.write(10);
    Udp.endPacket();
    delay(20);
}

// Updates Mpu position and post to socket.io
void postValues(){
  updateMPU();
  //client.monitor();
  /*client.send("data","x",x);
  client.send("data","y",y);
  client.send("data","z",z);*/
  client.send("data","waarden", y + "," + z + "," + x ); // x y unity  -- y z x
  delay(30);
}

// NOT WORKING. Suppos to read data
void readValues(){
  if(client.monitor()){
  incoming = Rcontent;
  Serial.print("Data: ");
  Serial.println(incoming);
  }
}

void setup() {
  Serial.begin(115200);
  setupMPU();
  setZeroing();
  setupWifi();
  //setupSocket();
  setupUDP();
  //client.send("register_sensor","ESP", "SENSOR1" );
}

void loop() {

  if(WL_CONNECTED){
    postUDP();
  }
  else{
    Serial.println("Wifi down");
  }

  /*if(WL_CONNECTED){
    if(client.connected()){
          postValues();
    }
    else{
      Serial.println("Client down");
    }
  }
  else{
    Serial.println("Wifi down");
  }*/
  
}
