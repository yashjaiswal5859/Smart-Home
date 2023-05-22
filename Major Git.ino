#include <Adafruit_Fingerprint.h>
#include<ESP8266WiFi.h>
#if (defined(__AVR__) || defined(ESP8266)) && !defined(__AVR_ATmega2560__)
SoftwareSerial mySerial(D7, D8);
#include <ESP8266HTTPClient.h> // http web access library
#include <NTPClient.h>
#include <WiFiUdp.h>
#else
#define mySerial Serial1
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   // Universal Telegram Bot Library written by Brian Lough: https://github.com/witnessmenow/Universal-Arduino-Telegram-Bot
#include <ArduinoJson.h>
#include <Arduino_JSON.h>
#include <WiFiClient.h>
#include"DHT.h"
DHT dht;
int temp;
String API_Key = "*********************";
String Location = "*****************";
unsigned long lastTime = 0;
// Timer set to 10 minutes (600000)
//unsigned long timerDelay = 600000;
// Set timer to 10 seconds (10000)
unsigned long timerDelay = 10000;
int fire=0;
WiFiClient wificlient;
String jsonBuffer;
Adafruit_Fingerprint finger = Adafruit_Fingerprint(&mySerial);
uint8_t id=0;
const char* ssid = "*******************"; // SSID i.e. Service Set Identifier is the name of your WIFI  
const char* password = "*************************"; 
WiFiServer server(80);
WiFiClient client; 
int pir=D0;
int bulb=D1;
int lock=D2;
int dhtsen=D3;
int vibration=A0;
int vib=0;
#define BOTtoken "***************************"  // your Bot Token (Get from Botfather)

// Use @myidbot to find out the chat ID of an individual or a group
// Also note that you need to click "start" on a bot before it can
// message you
#define CHAT_ID "********************"

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure cliente;
UniversalTelegramBot bot(BOTtoken, cliente);

// Checks for new messages every 1 second.
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;

const long utcOffsetInSeconds = 3600;

char daysOfTheWeek[7][12] = {"Sunday", "Monday", "Tuesday", "Wednesday", "Thursday", "Friday", "Saturday"};

// Define NTP Client to get time
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", utcOffsetInSeconds);

int motion=0;


// Handle what happens when you receive new messages
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id of the requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", "");
      continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.print("Text: ");
    Serial.println(text);
    bot.sendMessage(chat_id, "There has been some unwanted activity in front of your door", "");  
    String from_name = bot.messages[i].from_name;
    }
}

void setup() {
  Serial.begin(9600);
#ifdef ESP8266
    configTime(0, 0, "pool.ntp.org");      // get UTC time via NTP
    cliente.setTrustAnchors(&cert); // Add root certificate for api.telegram.org
  #endif  
#ifdef ESP32
    cliente.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif  
   timeClient.begin();
  while (!Serial);  // For Yun/Leo/Micro/Zero/...
  delay(100);
  pinMode(pir, INPUT);
  pinMode(vibration,INPUT);
pinMode(bulb, OUTPUT);
pinMode(lock, OUTPUT);
dht.setup(dhtsen);

  digitalWrite(bulb,LOW);


  digitalWrite(lock,LOW);
  
  
  Serial.println("\n\nAdafruit Fingerprint sensor enrollment");
  WiFi.begin(ssid, password);  
  while (WiFi.status() != WL_CONNECTED) {  
   delay(500);  
   Serial.println("Cannot connect to network.");  
  }  
Serial.println("Connected to network");  // set the data rate for the sensor serial port
server.begin();  
  Serial.println("Server started");  
  // Print the IP address  
  Serial.print("Use this URL to connect: ");  
  Serial.print("http://");  
  Serial.print(WiFi.localIP()); //Gets the WiFi shield's IP address and Print the IP address of serial monitor  
  Serial.println("/");
  finger.begin(57600);
  
  while(!finger.verifyPassword()) {
    Serial.println("Not Found fingerprint sensor!");
    
  } 
  Serial.println(F("Reading sensor parameters"));
  finger.getParameters();
  Serial.print(F("Status: 0x")); Serial.println(finger.status_reg, HEX);
  Serial.print(F("Sys ID: 0x")); Serial.println(finger.system_id, HEX);
  Serial.print(F("Capacity: ")); Serial.println(finger.capacity);
  Serial.print(F("Security level: ")); Serial.println(finger.security_level);
  Serial.print(F("Device address: ")); Serial.println(finger.device_addr, HEX);
  Serial.print(F("Packet len: ")); Serial.println(finger.packet_len);
  Serial.print(F("Baud rate: ")); Serial.println(finger.baud_rate);
  
}

void loop() {
if(vib==0 && analogRead(vibration)>100)
{
    vib=1;
    message("There has been some unwanted activity in front of your door"); 
}
if(vib==1 && analogRead(vibration)<100)
{
vib=0;
}
//bot.sendMessage(CHAT_ID, "There has been some unwanted activity in front of your door", "");  


  digitalWrite(lock,LOW);
  tempe(); 
  
  if(fire==0 && dht.getTemperature()-temp>30)
  {
    fire=1;
    message("Alert!!! Temperature is rising.\nHome Temperature: "+String(dht.getTemperature())+" degree celsius, City's temperature: "+String(temp)+".00 degree celsius");
return;    
  }
  if(fire==1 && dht.getTemperature()-temp<20)
  {
    fire=0;
    message("Temperature is now in control.\nHome Temperature: "+String(dht.getTemperature())+" degree celsius, City's temperature: "+String(temp)+".00 degree celsius");
    return;
  }
  PIRSensor();
  id=0;
  client=server.available();
  uint8_t tempid;
    uint8_t p;

  if (!client) {  
p=getFingerprintID(tempid)    ;
        if(p!=FINGERPRINT_OK)
    {
        return;
    }        
          digitalWrite(lock,HIGH);
          delay(4000);
   return;
  }  
  // Wait until the client sends some data 
  
  Serial.println("new client");  
  while(!client.available()){ 
    return; 
   delay(1);  
  }  
  
  String request = client.readStringUntil('\r'); 
if(request[5]=='z')
{
client.println("HTTP/1.1 200 OK");  
    client.println("Content-Type: text/html");  
    client.println("");  
    client.println("<!DOCTYPE HTML>");  
    client.println("<html>");
    client.print(dht.getTemperature());
    client.print("                   ");
    client.print(temp);
    client.println("</br>");        
    client.println("</html>");  
    client.flush();
    client.stop();  
    delay(1);  
return;
} 
  if(request[5]=='b')
{
Serial.println("off");
digitalWrite(bulb,LOW);
message("Turning off light!!!");
motion=0;
client.flush();
    client.stop();  
return;
} 
if(request[5]=='a')
{
Serial.println("on");
digitalWrite(bulb,HIGH);
message("Turning on light!!!");
client.flush();
    client.stop();  
return;
} 
  for(int i=5;i<request.length();i++)
  {
if(request[i]==' ')
{
  break;
}

id=id*10+(request[i]-'0');
      
  }
      
  Serial.println(id);    
  
    client.println("HTTP/1.1 200 OK");  
    client.println("Content-Type: text/html");  
    client.println("");  
    client.println("<!DOCTYPE HTML>");  
    client.println("<html>");
    client.println("<body>");
    client.println("<font color='green'>");
    client.println("Ready to enroll a fingerprint!<br>");
    client.println("Enrolling ID # ");
    client.println(id);
    client.println("</font>"); 
    client.println("</br>");        
  while ( getFingerprintEnroll() !=FINGERPRINT_OK)
  {
    client.println("<font color='red'>");
    client.println("Some error occured!!! Please do it again</br>");  
    client.println("</font>");  
  } 
     id=(id+1)%127;
client.println("</body>");      
    client.println("</html>");  
    client.flush();
    client.stop();  
    delay(1);  
  
  
}




uint8_t readnumber(void) {
  uint8_t num = 0;

  while (num == 0) {
    while (! Serial.available());
    num = Serial.parseInt();
  }
  return num;
}


uint8_t getFingerprintEnroll() {

  try
  {
  int p = -1;
  client.println("<font color='blue'>");  
  client.println("Waiting for valid finger to enroll as #"); 
  client.println(id);
  client.println("</font>");
client.println("</br>");  
  int a=-1;
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
    client.println("<font color='green'>");
      client.println("Image taken</br>");
client.println("</font>");      
      break;
    case FINGERPRINT_NOFINGER:
      if(a!=p)
            {
              client.println("<font color='blue'>");
              client.println("Processing.....</br>");
              client.println("</font>");
              a=p;
            }      
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Communication error</br>");
              client.println("</font>");
              a=p;
            }      
      break;
    case FINGERPRINT_IMAGEFAIL:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Imaging error</br>");
              client.println("</font>");                           
               a=p;
            }  
      break;
    default:
        if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Unknown error</br>");
              client.println("</font>");
               a=p;
            }      
      break;
    }
  }

  // OK success  
  p = finger.image2Tz(1);
  switch (p) {
    case FINGERPRINT_OK:
client.println("<font color='green'>");    
      client.println("Image converted</br>");
      client.println("</font>");
      break;
    case FINGERPRINT_IMAGEMESS:
      if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Image too messy</br>");
              client.println("</font>");
               a=p;
            }      
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Communication error</br>");
              client.println("</font>");
               a=p;
            }      
      return p;
    case FINGERPRINT_FEATUREFAIL:
    if(a!=p)
            {
              client.println("<font color='red'>");              
              client.println("Could not find fingerprint feature</br>");
              client.println("</font>");
               a=p;
            }      
      return p;
    case FINGERPRINT_INVALIDIMAGE:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Invalid Image</br>");
              client.println("</font>");
               a=p;
            }      
      return p;
    default:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Unknown error</br>");
              client.println("</font>");
               a=p;
            }      
      return p;
  }
  client.println("<font color='black'>");
  client.println("Remove finger</br>");
  client.println("</font>");
  delay(3000);
  p = 0;
  while (p != FINGERPRINT_NOFINGER) {
    p = finger.getImage();
  }
  client.println("ID "); client.println(id);
  client.println("</br>");
  p = -1;
  client.println("Place same finger again</br>");
  while (p != FINGERPRINT_OK) {
    p = finger.getImage();
    switch (p) {
    case FINGERPRINT_OK:
      client.println("<font color='green'>");    
      client.println("Image taken</br>");
      client.println("</font>");
      break;
    case FINGERPRINT_NOFINGER:
    if(a!=p)
            {
              client.println("<font color='blue'>");
              client.println("Processing...</br>");
              client.println("</font>");
              a=p;
            }      
      break;
    case FINGERPRINT_PACKETRECIEVEERR:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Communication error</br>");
              client.println("</font>");
              a=p;
            }      
      break;
    case FINGERPRINT_IMAGEFAIL:
        if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Imaging error</br>");
              client.println("</font>");
              a=p;
            }          
      break;
    default:
    if(a!=p)
            {
              client.println("<font color='red'>");
              client.println("Unknown error</br>");
              client.println("</font>");
              a=p;
            }         
      break;
    }
  }

  // OK success!

  p = finger.image2Tz(2);
  switch (p) {
    case FINGERPRINT_OK:
    client.println("<font color='green'>");
      client.println("Image converted</br>");
      client.println("</font>");
      break;
    case FINGERPRINT_IMAGEMESS:
    if(a!=p)
    {
        client.println("<font color='red'>");
        client.println("Image too messy</br>");
        client.println("</font>");    
        a=p;
    }
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
    if(a!=p)
    {
      client.println("<font color='red'>");
        client.println("Communication error</br>");
        client.println("</font>");    
        a=p;
    }
      return p;
    case FINGERPRINT_FEATUREFAIL:
      if(a!=p)
    {
      client.println("<font color='red'>");
        client.println("Could not find fingerprint features</br>");
        client.println("</font>");  
        a=p;
    }
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      if(a!=p)
    { client.println("<font color='red'>");
        client.println("Invalid Image</br>");
        client.println("</font>");
        a=p;
    }
      return p;
    default:
        if(a!=p)
    {
        client.println("<font color='red'>");
        client.println("Unknown error</br>");
        client.println("</font>");
        a=p;
    }      
      return p;
  }

  // OK converted!
  client.println("<font color='green'>");
  client.println("Creating model for #");  client.println(id);
  client.println("</br>");
  client.println("</font>");
  p = finger.createModel();
  if (p == FINGERPRINT_OK) {
    client.println("<font color='green'>");
    client.println("Prints matched!</br>");
    client.println("</font>");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    client.println("<font color='red'>");
    client.println("Communication error</br>");
    client.println("</font>");
    return p;
  } else if (p == FINGERPRINT_ENROLLMISMATCH) {
    client.println("<font color='red'>");
    client.println("Fingerprints did not match</br>");
    client.println("</font>");
    return p;
  } else {
    client.println("<font color='red'>");
    client.println("Unknown error</br>");
    client.println("</font>");
    return p;
  }

  client.println("ID "); client.println(id);
  client.println("</br>");
  p = finger.storeModel(id);
  if (p == FINGERPRINT_OK) {
    client.println("<font color='green'>");
    client.println("Stored!</br>");
    client.println("</font>");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    client.println("<font color='red'>");
    client.println("Communication error</br>");
    client.println("</font>");
    return p;
  } else if (p == FINGERPRINT_BADLOCATION) {
    client.println("<font color='red'>");
    client.println("Could not store in that location</br>");
    client.println("</font>");
    return p;
  } else if (p == FINGERPRINT_FLASHERR) {
    client.println("<font color='red'>");
    client.println("Error writing to flash</br>");
    client.println("</font>");
    return p;
  } else {
    client.println("<font color='red'>");
    client.println("Unknown error</br>");
    client.println("</font>");
    return p;
  }

  return p;
  }  
  catch(int exc)
  {
    client.println("<font color='red'>");
    client.println("Exception occured</br>");
    client.println("</font>");    
    return -1;  
  }
}


uint8_t getFingerprintID(uint8_t &temp) {
try  
{
  uint8_t p = finger.getImage();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image taken");
      break;
    case FINGERPRINT_NOFINGER:
      //Serial.println("No finger detected");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_IMAGEFAIL:
      Serial.println("Imaging error");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK success!

  p = finger.image2Tz();
  switch (p) {
    case FINGERPRINT_OK:
      Serial.println("Image converted");
      break;
    case FINGERPRINT_IMAGEMESS:
      Serial.println("Image too messy");
      return p;
    case FINGERPRINT_PACKETRECIEVEERR:
      Serial.println("Communication error");
      return p;
    case FINGERPRINT_FEATUREFAIL:
      Serial.println("Could not find fingerprint features");
      return p;
    case FINGERPRINT_INVALIDIMAGE:
      Serial.println("Invalid image");
      return p;
    default:
      Serial.println("Unknown error");
      return p;
  }

  // OK converted!
  p = finger.fingerSearch();
  if (p == FINGERPRINT_OK) {
    Serial.println("Found a print match!");
  } else if (p == FINGERPRINT_PACKETRECIEVEERR) {
    Serial.println("Communication error");
    return p;
  } else if (p == FINGERPRINT_NOTFOUND) {
    Serial.println("Did not find a match");
    return p;
  } else {
    Serial.println("Unknown error");
    return p;
  }

  // found a match!
  Serial.print("Found ID #"); Serial.print(finger.fingerID);
  Serial.print(" with confidence of "); Serial.println(finger.confidence);
  temp=finger.fingerID;
  return p;
  }  
  catch(int exc)
  {
    Serial.println("Exception");
    return -1;  
  }
}

void PIRSensor() 
{
   if(digitalRead(pir) == HIGH)
    {
        motion++;
        
        Serial.println("Motion detected.");
        
        bot.sendMessage(CHAT_ID,"Motion Number: "+String(motion),"");
        message("Someone entered in your room!!!!!!!!!!!");
         digitalWrite(bulb,HIGH);
         delay(50);
      }
      
}

void message(String msg)
{
  timeClient.update();
  int min=timeClient.getMinutes()+30;
  int hr=timeClient.getHours()+4;
  int d=timeClient.getDay();
  hr+=min/60;
  min=min%60;
  d+=hr/24;
  hr=hr%24;
  d=d%7;
  bot.sendMessage(CHAT_ID,String(daysOfTheWeek[d])+", Time in 24hr format: "+String(hr)+":"+String(min)+":"+String(timeClient.getSeconds()),"");
  bot.sendMessage(CHAT_ID, "\n"+msg, "");
}

String httpGETRequest(const char* serverName) {
  WiFiClient client;
  HTTPClient http;
    
  // Your IP address with path or Domain name with URL path 
  http.begin(client, serverName);
  
  // Send HTTP POST request
  int httpResponseCode = http.GET();
  
  String payload = "{}"; 
  
  if (httpResponseCode>0) {
    Serial.print("HTTP Response code: ");
    Serial.println(httpResponseCode);
    payload = http.getString();
  }
  else {
    Serial.print("Error code: ");
    Serial.println(httpResponseCode);
  }
  // Free resources
  http.end();

  return payload;
}
void tempe()
{
  if ((millis() - lastTime) > timerDelay) {
    // Check WiFi connection status
    if(WiFi.status()== WL_CONNECTED){
            String serverPath = "http://api.openweathermap.org/data/2.5/weather?q=" + Location + "&APPID=" + API_Key;
jsonBuffer = httpGETRequest(serverPath.c_str());
      JSONVar myObject = JSON.parse(jsonBuffer);
  
      // JSON.typeof(jsonVar) can be used to get the type of the var
      if (JSON.typeof(myObject) == "undefined") {
        Serial.println("Parsing input failed!");
        return;
      }
      temp=(int)myObject["main"]["temp"];
      temp-=273;
       

    }
    else {
      Serial.println("WiFi Disconnected");
    }
    lastTime = millis();

  }

}