#include <Arduino.h>
#include <SoftwareSerial.h>
#include "DHT.h"
#define DHTPIN 4


SoftwareSerial wifiSerial(2, 3);      // RX, TX for wifiSerial8266
DHT dht(DHTPIN, DHT22);
bool DEBUG = true;   //show more logs
int rwifiSerialonseTime = 3000; //communication timeout
String server = "quppeq.pythonanywhere.com";

void sendData(String  );
boolean find(String, String);
String  readSerialMessage();
String  readWifiSerialMessage();
String sendToWifi(String, int, boolean);
String sendToUno(String, int, boolean);
void httpPOST(String, int, boolean);




void setup() {
  pinMode(13,OUTPUT);  //set build in led as output
  // Open serial communications and wait for port to open wifiSerial8266:
  Serial.begin(9600);
  dht.begin();
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  wifiSerial.begin(115200);
  while (!wifiSerial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }
  sendToWifi("ATE0",rwifiSerialonseTime,1);
  sendToWifi("AT+CWMODE=1",rwifiSerialonseTime,1); // configure as access point
  // put your setup code here, to run once:
}

void loop() {
  if(Serial.available()>0){
     String message = readSerialMessage();
    if(find(message,"debugwifiSerial8266:")){
      String result = sendToWifi(message.substring(13,message.length()),rwifiSerialonseTime,DEBUG);
      if(find(result,"OK"))
        sendData("\nOK");
      else
        sendData("\nEr");
    }
  }
  if(wifiSerial.available()>0){

    String message = readWifiSerialMessage();

    if(find(message,"wifiSerial8266:")){
       String result = sendToWifi(message.substring(8,message.length()),rwifiSerialonseTime,DEBUG);
      if(find(result,"OK"))
        sendData("\n"+result);
      else
        sendData("\nErrRead");               //At command ERROR CODE for Failed Executing statement
    }else
    if(find(message,"HELLO")){  //receives HELLO from wifi
        sendData("\nHI!");    //arduino says HI
    }else if(find(message,"LEDON")){
      //turn on built in LED:
      digitalWrite(13,HIGH);
    }else if(find(message,"LEDOFF")){
      //turn off built in LED:
      digitalWrite(13,LOW);
    }
    else{
      sendData("\nErrRead");                 //Command ERROR CODE for UNABLE TO READ
    }
  }
  float h = dht.readHumidity();

  float t = dht.readTemperature();

  String postT = String("{\"name\":\"temperature\",\"data\":\"")+t+String("\"}");

  httpPOST(postT,1000,1);
  String postH = String("{\"name\":\"humidity\",\"data\":\"")+h+String("\"}");
  httpPOST(postH,1000,1);
  delay(rwifiSerialonseTime);

  // put your main code here, to run repeatedly:
}

void sendData(String str){
  String len="";
  len+=str.length();
  sendToWifi("AT+CIPSEND=0,"+len,rwifiSerialonseTime,DEBUG);
  delay(100);
  sendToWifi(str,rwifiSerialonseTime,DEBUG);
  delay(100);
//  sendToWifi("AT+CIPCLOSE=5",rwifiSerialonseTime,DEBUG);
}

/*
* Name: find
* Description: Function used to match two string
* Params:
* Returns: true if match else false
*/
boolean find(String string, String value){
  return string.indexOf(value)>=0;
}

/*
* Name: readSerialMessage
* Description: Function used to read data from Arduino Serial.
* Params:
* Returns: The rwifiSerialonse from the Arduino (if there is a reponse)
*/
String  readSerialMessage(){
  char value[100];
  int index_count =0;
  while(Serial.available()>0){
    value[index_count]=Serial.read();
    index_count++;
    value[index_count] = '\0'; // Null terminate the string
  }
  String str(value);
  str.trim();
  return str;
}



/*
* Name: readWifiSerialMessage
* Description: Function used to read data from wifiSerial8266 Serial.
* Params:
* Returns: The rwifiSerialonse from the wifiSerial8266 (if there is a reponse)
*/
String  readWifiSerialMessage(){
  char value[100];
  int index_count =0;
  while(wifiSerial.available()>0){
    value[index_count]=wifiSerial.read();
    index_count++;
    value[index_count] = '\0'; // Null terminate the string
  }
  String str(value);
  str.trim();
  return str;
}



/*
* Name: sendToWifi
* Description: Function used to send data to wifiSerial8266.
* Params: command - the data/command to send; timeout - the time to wait for a rwifiSerialonse; debug - print to Serial window?(true = yes, false = no)
* Returns: The rwifiSerialonse from the wifiSerial8266 (if there is a reponse)
*/
String sendToWifi(String command, int timeout, boolean debug){
  String rwifiSerialonse = "";
  wifiSerial.println(command); // send the read character to the wifiSerial8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(wifiSerial.available())
    {
    // The wifiSerial has data so display its output to the serial window
    char c = wifiSerial.read(); // read the next character.
    rwifiSerialonse+=c;
    }
  }
  if(debug)
  {
    Serial.println(rwifiSerialonse);
  }
  return rwifiSerialonse;
}

/*
* Name: sendToUno
* Description: Function used to send data to Arduino.
* Params: command - the data/command to send; timeout - the time to wait for a rwifiSerialonse; debug - print to Serial window?(true = yes, false = no)
* Returns: The rwifiSerialonse from the wifiSerial8266 (if there is a reponse)
*/
String sendToUno(String command, const int timeout, boolean debug){
  String rwifiSerialonse = "";
  Serial.println(command); // send the read character to the wifiSerial8266
  long int time = millis();
  while( (time+timeout) > millis())
  {
    while(Serial.available())
    {
      // The wifiSerial has data so display its output to the serial window
      char c = Serial.read(); // read the next character.
      rwifiSerialonse+=c;
    }
  }
  if(debug)
  {
    Serial.println(rwifiSerialonse);
  }
  return rwifiSerialonse;
}
void httpPOST(String data, int timeout, boolean debug){
  sendToWifi("AT+CIPSTART=\"TCP\",\"" + server + "\",80",4*rwifiSerialonseTime,DEBUG);
  delay(2000);
  if( wifiSerial.find("OK"))
  {
    Serial.println("TCP connection ready");
  }
  delay(2000);
  String postRequest =  String("POST ") + String("/arduino/postData") + String(" HTTP/1.1\r\n") +
    String("Host:quppeq.pythonanywhere.com\r\n") +
    String("Content-Type:application/json\r\n") +
    String("Content-Length:40\r\n\r\n") +
    data + String("\r\n\r\n\r\n\r\n");
  String sendCmd = "AT+CIPSEND=";//determine the number of caracters to be sent/.
  wifiSerial.print(sendCmd);
  delay(100);

  wifiSerial.println(postRequest.length() );

  delay(800);

  if(wifiSerial.find(">")) {
    Serial.println("Sending..");
    Serial.println(postRequest);
    sendToWifi(postRequest,4000, debug);
    delay(1000);
  }
  delay(1500);
}
