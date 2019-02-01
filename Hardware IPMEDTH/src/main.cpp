
//#                        ###### LIBRARIES ########
#include <Arduino.h>
#include <ESP8266WiFi.h>

// Libraries nodig voor gyroscope
  // https://github.com/tockn/MPU6050_tockn 
#include <MPU6050_tockn.h> // Library is aangepast zodat het met een ESP8266 kan werken (Moet de samlping in calcGyroOffsets in MPU6050_tockn.cpp naar 1200 van 2000 )
#include <Wire.h>

// library voor data verstuuren via webSockets
  //https://github.com/washo4evr/Socket.io-v1.x-Library
#include <SocketIOClient.h> 
#include <ArduinoJson.h> 

// Librariers voor data verstuuren via webserver. //https://circuits4you.com/2016/12/16/esp8266-web-server-html/
#include <WiFiClient.h>
#include <ESP8266WebServer.h>
ESP8266WebServer server(80);


//#                        ###### VARIABLES########
// MPU calibratie en printen van variables
MPU6050 mpu6050(Wire);
String x,y,z; //Strings for the values so it can be sent throught socket.io
float xi,yi,zi; // MPU position values as floats
float ZeroX, ZeroY, ZeroZ; // Zeroing offset values


// Wifi info 
const char* ssid = "wifi2"; 
const char* password = "lumc12345"; 
IPAddress ap_local_IP(192,168,1,1);
IPAddress ap_gateway(192,168,1,254);
IPAddress ap_subnet(255,255,255,0);


//Socket.io variables and initialization
SocketIOClient client;

  // server ip and port
  char host[] = "83.81.251.42";
  int port = 8080;

  // Json buffer voor het stuuren van berichten
  StaticJsonBuffer<200> jsonBuffer;
  String JSON;
  JsonObject& root = jsonBuffer.createObject();


  // gebruikt voort lezen van inkomende data bij socket
  String incoming;
  extern String RID;
  extern String Rname;
  extern String Rcontent;

  // Mode van data verzenden  true = socket - false = webserver
  int socketMode = false;  



//#                        ###### WIFI SETUP FUNCTIE ########
void setupWifi(){
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  
  WiFi.begin(ssid, password);
  
  // repeat until wifi is connected
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println("");
  Serial.println("WiFi connected");  
  Serial.println("IP address: ");
  Serial.println(WiFi.localIP());
  
}

// Setup ESP8266 als access point in plaats van wifi 
void setupAccessPoint(){
  WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
  Serial.print("Setting soft-AP ... ");
  WiFi.softAP(ssid, password);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
}


//#                        ###### MPU FUNCTIES ########

// Initialiseren van MPU en MPU Pins
void setupMPU(){
  Wire.begin(4,5); //SDA = 4, SCL = 5
  mpu6050.begin();
}

// Zeroing values pakken
void setZeroing(){
  mpu6050.calcGyroOffsets(true); // calculate gyroscope offsets

  int samples = 50;

  //mpu6050.update(); // get current position

  float allx;
  float ally;
  float allz;

  for( int a = 0; a < samples; a++ ) {
      mpu6050.update();
      allx += mpu6050.getAngleX();
      ally += mpu6050.getAngleY();
      allz += mpu6050.getAngleZ();
  }

  allx = allx / samples;
  ally = ally / samples;
  allz = allz / samples;

  float checkX = mpu6050.getAngleX() - allx;
  float checkY = mpu6050.getAngleY() - ally;
  float checkZ = mpu6050.getAngleZ() - allz;

  if(checkX > 0){
    allx += checkX;
  } 

  if(checkY > 0){
    ally += checkY;
  } 

  if(checkZ > 0){
    allz += checkZ;
  } 


  // replace with mpu6050.getAngleX(); enz 
  ZeroX = allx; 
  ZeroY = ally;
  ZeroZ = allz;

} 

//Zet het meest recent MPU posities in variabelen. Deze waarden transformeren naar String zodat het verstuurbaar is. 
void updateMPU(){

  mpu6050.update(); // get current position

  float xz = mpu6050.getAngleZ() - ZeroZ;

  // get new values - offset values
  xi = mpu6050.getAngleX() - ZeroX; 
  yi = mpu6050.getAngleY() - ZeroY;

  if (xz > 360){
     zi = (mpu6050.getAngleZ() - ZeroZ) - 360;
  }
  else{
    zi = mpu6050.getAngleZ() - ZeroZ;
  }
 



  //Uncomment om filters to gebruiken voor de waarden. Deze filters zijn gebaseerd op de graden dat een menselijke voet kan bewegen.. https://www.verywellhealth.com/what-is-normal-range-of-motion-in-a-joint-3120361
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

  // floats naar strings converteren zodat het verstuurd kan worden 
  x = (String)xi;
  y = (String)yi;
  z = (String)zi;

}

// MPU values printen
void printMPU(){
  updateMPU();
  Serial.println(x + ","+ y +","+ z);
}




//#                        ###### DATA VERZENDEN VIA SERVER. FUNCTIES########


// setup van de server
void setupMyServer(){
  server.begin();// begint de server
  Serial.printf("Web server started, open %s in a web browser\n", WiFi.localIP().toString().c_str()); // print ip via serial
}


// Data die gestuurd wordt als er een request binnen komt(WORDT GEREGELD DOOR SERVER.ON() IN SETUP) 
void postServerData(){
  updateMPU();
  String data = y + "," + z + "," + x;
  server.send(200, "text/plain", data);
  delay(30);
}



//#                        ###### DATA VERZENDEN VIA SOCKET. FUNCTIES ########

// Initialiseer en verbinding maken met socket.io server
void setupSocket(){

  // verbinding met host maken. Host and port zijn variabelen boven
  client.connect(host, port);

  // loop tot dat het verbinding maakt.
  while(!client.connected()){
     Serial.println("Connection Failed, retrying");
     client.connect(host, port);
     delay(500);
    }

  if (client.connected())
  {
    Serial.println("Connected");
  }
  
}

// data verstuuren met socket.io
void postValues(){
  updateMPU(); // nieuw MPU waarden krijgen

  if(WL_CONNECTED){
    // kijk als er een verbinding is met client
    if(client.connected()){ 
      client.send("data","waarden", y + "," + z + "," + x ); // data verstuuren naar host
      delay(30); // !!!! belangrijke delay, anders crasht het
    }
    else{
      Serial.println("Client down");
    }
  }
  else{
    Serial.println("Wifi down");
  }

}



//#                        ###### SETUP AND LOOP########

// Setup, runs only once
void setup() {
  Serial.begin(115200); // Serial baudrate zetten

  // MPU setup 
  setupMPU();
  setZeroing();

  // Recalibratie interrupt setup 
  pinMode(12, HIGH);
  attachInterrupt(digitalPinToInterrupt(12), setZeroing, LOW);

  // wifi setup 
  setupWifi();
  //setupAccessPoint();

  // Socket of Webserver setup
  if(socketMode ==  true){
    setupSocket();
  }
  else{
    server.on("/", postServerData); // BELANGRIJK, DEZE REGELT DE RESPONSE BIJ DATA REQUEST 
    setupMyServer();

  }


}

//Keeps looping
void loop() {
  if(socketMode == true){
    postValues(); // socket posting
  } 
  else{
    server.handleClient(); // webserver posting
  }

  
}

