#include <SoftwareSerial.h>
#include <LiquidCrystal.h>
#include <Servo.h>

LiquidCrystal lcd(12, 11, 8, 7, 4, 5); // Arduino pins connected to LCD. Another option is using I2C
#define RX 2 // Arduino Pin 2 connected to TX Pin of ESP8266
#define TX 3 // Arduino Pin 3 connected to RX Pin of ESP8266
int servoPin = 13; // Arduino Pin servo_motor is connected to
Servo myServo;
int window_door = 0;
int pos = 0;
int piezoPin = 9; //Arduino Pin connected to Buzzer
int fanPin = 10; // Arduino Pin connected to mini-fan
String AP = "";       // Internet Username to connect to ESP
String wifi_password = ""; // Internet password
String write_api = "";   // Thingspeak Write API KEY
String host_api = "api.thingspeak.com"; //Thingspeak api 
String PORT = "80"; // Connection to port 80
int trueCount;
int timeCount; 
boolean found = false; 
int valSensor = 1;
int const gas_pin = A0;
  
SoftwareSerial esp8266(RX,TX); 
  
void setup() {
  lcd.begin(16, 2);
  lcd.print(" HELLO ");
  myServo.attach(servoPin);
  myServo.write(pos);
  pinMode(piezoPin, OUTPUT);  
  pinMode(fanPin, OUTPUT);
  Serial.begin(9600);
  esp8266.begin(115200);
  sendingCommands("AT",5,"OK");
  sendingCommands("AT+CWMODE=1",5,"OK");
  sendingCommands("AT+CWJAP=\""+ AP +"\",\""+ wifi_password +"\"",20,"OK");
}

void loop() {
  if (getGasValue()<100){
    //low level of gas concentration in the air
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" NORMAL ");
    lcd.setCursor(0,1);
    lcd.print("Gas Value : ");
    lcd.print(getGasValue());
    digitalWrite(piezoPin, LOW);
    digitalWrite(fanPin, LOW);
    
    if (window_door == 1){
      for (pos = 180; pos >= 0; pos -= 1){
        myServo.write(pos);
        delay(15);
        }
        window_door = 0;
    }  
    String getData = "GET /update?api_key="+ write_api +"&field1="+getGasValue();
    sendingCommands("AT+CIPMUX=1",5,"OK");
    sendingCommands("AT+CIPSTART=0,\"TCP\",\""+ host_api +"\","+ PORT,15,"OK");
    sendingCommands("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(150);trueCount++;
    sendingCommands("AT+CIPCLOSE=0",5,"OK");    
  }
  //Moderate gas value will be updated to thingspeak
  else if(getGasValue()>101 && getGasValue()<300) {
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print(" MEDIUM ");
    lcd.setCursor(0,1);
    lcd.print("Gas Value : ");
    lcd.print(getGasValue());
    digitalWrite(piezoPin, HIGH);
    delay(10);
    digitalWrite(piezoPin, LOW);
    delay(10);
    digitalWrite(fanPin, HIGH);
    String getData = "GET /update?api_key="+ write_api +"&field1="+getGasValue();
    sendingCommands("AT+CIPMUX=1",5,"OK");
    sendingCommands("AT+CIPSTART=0,\"TCP\",\""+ host_api +"\","+ PORT,15,"OK");
    sendingCommands("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(150);trueCount++;
    sendingCommands("AT+CIPCLOSE=0",5,"OK");    
  }
  else {
    // Extreme level of gas concentration in the air
    // Data obtained will be updated to Thingspeak
    lcd.clear();
    lcd.setCursor(0,0);
    lcd.print("!!** EXTREME **!!");
    lcd.setCursor(0,1);
    lcd.print("Gas Value : ");
    lcd.print(getGasValue());
    tone(piezoPin, 4000, 5000);  
    digitalWrite(piezoPin, HIGH);   
    digitalWrite(fanPin, HIGH); 
    if(window_door==0){
      for(pos = 0;pos <= 180; pos += 1){
        myServo.write(pos);
        delay(15);
      }   
      window_door=1;
    }
    String getData = "GET /update?api_key="+ write_api +"&field1="+getGasValue();
    sendingCommands("AT+CIPMUX=1",5,"OK");
    sendingCommands("AT+CIPSTART=0,\"TCP\",\""+ host_api +"\","+ PORT,15,"OK");
    sendingCommands("AT+CIPSEND=0," +String(getData.length()+4),4,">");
    esp8266.println(getData);delay(150);trueCount++;
    sendingCommands("AT+CIPCLOSE=0",5,"OK");    
  }
}


int getGasValue(){

   float g = analogRead(gas_pin);
   Serial.print(" Gas Value is %= ");
   Serial.println(g);
   delay(10);
   return int(g); 
  
}

void sendingCommands(String command, int maxTime, char readReplay[]) {
  Serial.print(trueCount);
  Serial.print(". at command -> ");
  Serial.print(command);
  Serial.print(" ");
  while(timeCount < (maxTime*1))
  {
    esp8266.println(command);//at+cipsend
    if(esp8266.find(readReplay))//ok
    {
      command = true;
      break;
    }
  
    timeCount++;
  }
  
  if(found == true)
  {
    Serial.println("OK");
    trueCount++;
    timeCount = 0;
  }
  
  if(found == false)
  {
    Serial.println("FAIL UPLOAD");
    trueCount = 0;
    timeCount = 0;
  }
  
  found = false;
 }
