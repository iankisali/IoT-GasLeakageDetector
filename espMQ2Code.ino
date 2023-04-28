#include <SoftwareSerial.h>
#define RX 2
#define TX 3
#define dht_apin 11 // Analog Pin sensor is connected to
String AP = "";       // AP NAME
String PASS = ""; // AP PASSWORD
String API = "UV1XF";   // Thingspeak Write API KEY
String HOST = "api.thingspeak.com";
String PORT = "80";
int countTrueCommand;
int countTimeCommand; 
boolean found = false; 
int valSensor = 1;
int const gas_pin = A0;
  
SoftwareSerial esp8266(RX,TX); 
  
void setup() {
  Serial.begin(9600);
  esp8266.begin(115200);
  sendCommand("AT",5,"OK");
  sendCommand("AT+CWMODE=1",5,"OK");
  sendCommand("AT+CWJAP=\""+ AP +"\",\""+ PASS +"\"",20,"OK");
}

void loop() {
  if (getGasValue()<100){
    //low level of gas concentration in the air
    String getData = "GET /update?api_key="+ API +"&field1="+getGasValue();
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");    
  }
  //Moderate gas value will be updated to thingspeak
  else if(getGasValue()>101 && getGasValue()<200) {
    String getData = "GET /update?api_key="+ API +"&field1="+getGasValue();
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");    
  }
  else {
    // Extreme level of gas concentration in the air
    // Data obtained will be updated to Thingspeak
    String getData = "GET /update?api_key="+ API +"&field1="+getGasValue();
    sendCommand("AT+CIPMUX=1",5,"OK");
    sendCommand("AT+CIPSTART=0,\"TCP\",\""+ HOST +"\","+ PORT,15,"OK");
    sendCommand("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(1500);countTrueCommand++;
    sendCommand("AT+CIPCLOSE=0",5,"OK");    
  }
}


int getGasValue(){

   float g = analogRead(gas_pin);
   Serial.print(" Gas Value is %= ");
   Serial.println(g);
   delay(50);
   return int(g); 
  
}

void sendCommand(String command, int maxTime, char readReplay[]) {
  Serial.print(countTrueCommand);
  Serial.print(". at command => ");
  Serial.print(command);
  Serial.print(" ");
  while(countTimeCommand < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      found = true;
      break;
    }
  
    countTimeCommand++;
  }
  
  if(found == true)
  {
    Serial.println("OYI");
    countTrueCommand++;
    countTimeCommand = 0;
  }
  
  if(found == false)
  {
    Serial.println("Fail");
    countTrueCommand = 0;
    countTimeCommand = 0;
  }
  
  found = false;
 }
