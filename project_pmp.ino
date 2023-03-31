#define DEBUG true
#include <LiquidCrystal.h>
LiquidCrystal lcd(7, 6, 5, 4, 3, 2);
float resolutionADC = .0049 ; // rezoluția implicită (pentru 
//referința 5V) = 0.049 [V] / unitate
float resolutionSensor = .01 ; // rezoluție senzor = 0.01V/°C
float temp;
float temp1;
float temp2=16.0;
void setup() {
 Serial.begin(115200);
 Serial1.begin(115200);
 pinMode(LED_BUILTIN, OUTPUT);
 digitalWrite(LED_BUILTIN, LOW);
 sendData("AT+RST\r\n", 2000, false); // resetare modul
 sendData("AT+CWMODE=2\r\n", 1000, false); // configurare ca 
 //access point
 sendData("AT+CIFSR\r\n", 1000, DEBUG); // citește adresa IP
 sendData("AT+CWSAP?\r\n", 2000, DEBUG); // citește informația 
 //SSID (nume rețea)
 sendData("AT+CIPMUX=1\r\n", 1000, false); // configurare 
 //conexiuni multiple
 sendData("AT+CIPSERVER=1,80\r\n", 1000, false); // pornire 
 //server pe port 80
}
void loop() {
 if (Serial1.available()) {
 if (Serial1.find("+IPD,"))
 {delay(500);
 int connectionId = Serial1.read() - 48; // functia 
 //read() returnează valori zecimale ASCII
 // si caracterul ‘0’ are codul ASCII 48
 String webpage="<h1>Termostat</h1>";
 webpage += "<h2>Setati temperatura dorita:</h2><a href=\"/l0\"><button>+</button></a>";
 webpage+=temp2;
 String cipSend = "AT+CIPSEND=";
 cipSend += connectionId;
 cipSend += ",";
 webpage += "<a href=\"/l1\"><button>-</button></a>";
 
 if (readSensor() > 0) {
 webpage += "<h2>Temperatura actuala:</h2>";
 webpage += readSensor();
 temp1=readSensor();
 }
lcd.begin(16, 2);
 if (temp2>temp1)
 {
  lcd.print("Centrala este ");
  lcd.setCursor(7, 1);
  lcd.print("pornita!");
 }
 else
 {lcd.print("Centrala este ");
 lcd.setCursor(7, 1);
  lcd.print("oprita!");
 }
 cipSend += webpage.length();
 cipSend += "\r\n";
 sendData(cipSend, 100, DEBUG);
 sendData(webpage, 150, DEBUG);
 
 String closeCommand = "AT+CIPCLOSE=";
 closeCommand += connectionId; //se adaugă
 //identificatorul conexiunii
 closeCommand += "\r\n";
 sendData(closeCommand, 300, DEBUG);
 }
 }
}
float readTempInCelsius(int count, int pin) { 
 // citește temperatura de count ori de pe pinul analogic pin
 float sumTemp = 0;
 for (int i =0; i < count; i++) { 
 int reading = analogRead(pin); 
 float voltage = reading * resolutionADC; 
 float tempCelsius = (voltage - 0.5) / resolutionSensor ; 
 // scade deplasament, convertește în grade C
 sumTemp = sumTemp + tempCelsius; // suma temperaturilor
 } 
 return sumTemp / (float)count; // media returnată
}

String sendData(String command, const int timeout, boolean debug) 
{
 String response = "";
 Serial1.print(command); // trimite comanda la esp8266
 long int time = millis();
 while ((time + timeout) > millis()) {
 while (Serial1.available()) {
 char c = Serial1.read(); // citește caracter următor
 response += c;
 }
 }

 if (response.indexOf("/l0") != -1) {
temp2=temp2+0.5;
 }
 if (response.indexOf("/l1") != -1) {
temp2=temp2-0.5;
 }
 if (debug) {
 Serial.print(response);
 }
 return response;
}
float readSensor() {
 temp = readTempInCelsius(10, 0);
 return temp;
}
