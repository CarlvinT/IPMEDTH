#include <EEPROM.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h> 
#include <ESP8266WebServer.h>
//Defining Web Server
ESP8266WebServer server(80);
/* Configuration Variables for the AP name and IP. */
const char *ssid = "Test";
const char *password = "Password";
IPAddress ap_local_IP(192,168,1,1);
IPAddress ap_gateway(192,168,1,254);
IPAddress ap_subnet(255,255,255,0);



//Creating the input form
const char INDEX_HTML[] =
"<!DOCTYPE HTML>"
"<html>"
"<head>"
"<meta content=\"text/html; charset=ISO-8859-1\""
" http-equiv=\"content-type\">"
"<meta name = \"viewport\" content = \"width = device-width, initial-scale = 1.0, maximum-scale = 1.0, user-scalable=0\">"
"<title>ESP8266 Web Form Demo</title>"
"<style>"
"\"body { background-color: #808080; font-family: Arial, Helvetica, Sans-Serif; Color: #000000; }\""
"</style>"
"</head>"
"<body>"
"<h1>ESP8266 Web Form Demo</h1>"
"<FORM action=\"/\" method=\"post\">"
"<P>"
"<label>ssid:&nbsp;</label>"
"<input maxlength=\"30\" name=\"ssid\"><br>"
"<label>Password:&nbsp;</label><input maxlength=\"30\" name=\"Password\"><br>"
"<label>IP:&nbsp;</label><input maxlength=\"15\" name=\"IP\"><br>"
"<label>Gateway:&nbsp;</label><input maxlength=\"3\" name=\"GW\"><br>"
"<INPUT type=\"submit\" value=\"Send\"> <INPUT type=\"reset\">"
"</P>"
"</FORM>"
"</body>"
"</html>";





void setup() {
	delay(1000);
	Serial.begin(115200);//Starting serial comunication
  EEPROM.begin(512);//Starting and setting size of the EEPROM
	Serial.println();
	Serial.print("Configuring access point...");
	Serial.print("Setting soft-AP configuration ... ");
  WiFi.softAPConfig(ap_local_IP, ap_gateway, ap_subnet);
  Serial.print("Setting soft-AP ... ");
  WiFi.softAP(ssid, password);
  Serial.print("Soft-AP IP address = ");
  Serial.println(WiFi.softAPIP());
	

	/**IPAddress myIP = WiFi.softAPIP();
	Serial.print("AP IP address: ");
	Serial.println(myIP);**/
  //Configuring the web server
	server.on("/", handleRoot);
  server.onNotFound(handleNotFound);
	server.begin();
	Serial.println("HTTP server started");
}



void loop() {
	server.handleClient();//Checks for web server activity
  
}
//Dealing with the call to root
void handleRoot() {
   if (server.hasArg("ssid")&& server.hasArg("Password")&& server.hasArg("IP")&&server.hasArg("GW") ) {//If all form fields contain data call handelSubmit()
    handleSubmit();
  }
  else {//Redisplay the form
    server.send(200, "text/html", INDEX_HTML);
  }
}
void handleSubmit(){//dispaly values and write to memmory
  String response="<p>The ssid is ";
 response += server.arg("ssid");
 response +="<br>";
 response +="And the password is ";
 response +=server.arg("Password");
 response +="<br>";
 response +="And the IP Address is ";
 response +=server.arg("IP");
 response +="</P><BR>";
 response +="<H2><a href=\"/\">go home</a></H2><br>";

 server.send(200, "text/html", response);
 //calling function that writes data to memory 
 write_to_Memory(String(server.arg("ssid")),String(server.arg("Password")),String(server.arg("IP")),String(server.arg("GW")));
}
//Write data to memory
/**
 * We prepping the data strings by adding the end of line symbol I decided to use ";". 
 * Then we pass it off to the write_EEPROM function to actually write it to memmory
 */
void write_to_Memory(String s,String p,String i, String g){
 s+=";";
 write_EEPROM(s,0);
 p+=";";
 write_EEPROM(p,100);
 i+=";";
 write_EEPROM(i,200); 
 g+=";";
 write_EEPROM(g,220); 
 EEPROM.commit();
}
//write to memory
void write_EEPROM(String x,int pos){
  for(int n=pos;n<x.length()+pos;n++){
     EEPROM.write(n,x[n-pos]);
  }
}
//Shows when we get a misformt or wrong request for the web server
void handleNotFound()
{
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += server.uri();
  message += "\nMethod: ";
  message += (server.method() == HTTP_GET)?"GET":"POST";
  message += "\nArguments: ";
  message += server.args();
  message += "\n";
  for (uint8_t i=0; i<server.args(); i++){
    message += " " + server.argName(i) + ": " + server.arg(i) + "\n";
  }
  message +="<H2><a href=\"/\">go home</a></H2><br>";
  server.send(404, "text/plain", message);
}


