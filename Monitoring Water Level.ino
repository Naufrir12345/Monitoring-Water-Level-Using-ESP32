#ifdef ESP32
#include <WiFi.h>
#else
#include <ESP8266WiFi.h>
#endif
#include <WiFiClientSecure.h>
#include <UniversalTelegramBot.h>   
#include <ArduinoJson.h>

//wifi
const char* ssid = "AndroidAP5387"; //isi sendiri bro
const char* password = "pyuh6248"; //isi sendiri bro

// Initialize Telegram BOT
#define BOTtoken "5752726455:AAEODC5VlCuh4l3iDjOZTLi87HHAWk5hk2c" // your Bot Token (Get from Botfather)

// ID Telegram
#define CHAT_ID "1693413651" // ID Tele mu

#ifdef ESP8266
  X509List cert(TELEGRAM_CERTIFICATE_ROOT);
#endif

WiFiClientSecure client;
UniversalTelegramBot bot(BOTtoken, client);

// cek pesan tiap 1 detik
int botRequestDelay = 1000;
unsigned long lastTimeBotRan;
//------------------------------------------------------------------------------------
//SOLENOID VALVE
int solenoidPin = 32;

//FLOW METER SENSOR
int sensorPin = 27;
volatile long pulse;
unsigned long lastTime;
float volume;
float flowMilliLitres = 0;
float totalMilliLitres = 0;
float input = 0;
float batastotal = 0;

//WATER LEVEL SENSOR
#define SIGNAL_PIN1 35 // ESP32 pin GIOP36 (ADC0) connected to sensor's signal pin
#define SIGNAL_PIN2 34 // ESP32 pin GIOP36 (ADC0) connected to sensor's signal pin
int value1 = 0; // variable to store the sensor value
int value2 = 0; // variable to store the sensor value

//FLOW SENSOR FUNCTION
void increase() {
  pulse++;
}
//------------------------------------------------------------------------------------
//Message Telebot
void handleNewMessages(int numNewMessages) {
  Serial.println("handleNewMessages");
  Serial.println(String(numNewMessages));

  for (int i=0; i<numNewMessages; i++) {
    // Chat id requester
    String chat_id = String(bot.messages[i].chat_id);
    if (chat_id != CHAT_ID){
      bot.sendMessage(chat_id, "Unauthorized user", ""); 
            continue;
    }
    
    // Print the received message
    String text = bot.messages[i].text;
    Serial.println(text);
    String from_name = bot.messages[i].from_name;

    if (text == "/Hai") {
      String welcome = "Welcome, " + from_name + ".\n\n";
      welcome += "Gunakan perintah dibawah ini untuk mengontrol Solenoid Valve.\n";
      welcome += "/Open Selenoid \n";
      welcome += "/Close Selenoid  \n\n";
      welcome += "Gunakan perintah dibawah ini untuk mengecek banyak air.\n";
      welcome += "/Check_WLS Kondisi banyak air\n\n";
      welcome += "Gunakan perintah dibawah ini untuk input banyak air yang dikeluarkan.\n";
      welcome += "/1_Liter untuk mengeluarkan air 1 liter \n";
      welcome += "/2_Liter untuk mengeluarkan air 2 liter \n";
      welcome += "/3_Liter untuk mengeluarkan air 3 liter \n\n";
      welcome += "Gunakan perintah dibawah ini untuk mereset.\n";
      welcome += "/Reset\n";
      bot.sendMessage(chat_id, welcome, "");
    }

    //solenoid manual on
    if (text == "/Open") {
        bot.sendMessage(chat_id, "Selenoid OPEN", "");
        digitalWrite(solenoidPin, HIGH);
    }
    //solenoid manual off
    if (text == "/Close") {
        bot.sendMessage(chat_id, "Selenoid CLOSE", "");
        digitalWrite(solenoidPin, LOW);
    }
    //cek kondisi banyak air
    if (text == "/Check_WLS") {
        bot.sendMessage(chat_id, "Kondisi  banyak air saat ini", "");
        
        if(value1 < 500){ //value 1 dari water level sensor 1
          //digitalWrite(solenoidPin, HIGH); //Switch Solenoid ON
          bot.sendMessage(chat_id, "Kondisi air : Hampir Kosong");
        }

        else if (value1 > 2000 && value2 < 500){
          bot.sendMessage(chat_id, "Kondisi air : Sisa Setengah");
        }
        
        else if (value2 > 2000){ //value 2 dari water level sensor 2
          //digitalWrite(solenoidPin, LOW); //Switch Solenoid OFF
          bot.sendMessage(chat_id, "Kondisi air : Hampir Penuh");
        }
    }

    //input air
    if (text == "/1_Liter") {
      batastotal = totalMilliLitres + 1000;
      digitalWrite(solenoidPin, HIGH);
      bot.sendMessage(chat_id, "Air akan keluar sebanyak 1 Liter");
      if (totalMilliLitres == batastotal){
        digitalWrite(solenoidPin, LOW);
      }
    }
    
    else if (text == "/2_Liter") {
      batastotal = totalMilliLitres + 2000;
      digitalWrite(solenoidPin, HIGH);
      bot.sendMessage(chat_id, "Air akan keluar sebanyak 2 Liter");
      if (totalMilliLitres == batastotal){
        digitalWrite(solenoidPin, LOW);
      }
    }
    
    else if (text == "/3_Liter") {
      batastotal = totalMilliLitres + 3000;
      digitalWrite(solenoidPin, HIGH);
      bot.sendMessage(chat_id, "Air akan keluar sebanyak 3 Liter");
      if (totalMilliLitres == batastotal){
        digitalWrite(solenoidPin, LOW);
      }
    }

    //input air
    if (text == "/Reset") {
      bot.sendMessage(chat_id, "Data banyak air yang keluar akan di reset");
      totalMilliLitres = 0;
    }
  }
}
//------------------------------------------------------------------------------------
void setup() {
  //SOLENOID VALVE
  Serial.begin(115200);
  pinMode(solenoidPin, OUTPUT); //Sets the pin as an output
  
  //FLOW METER SENSOR
  pinMode(sensorPin, INPUT);
    
  attachInterrupt(digitalPinToInterrupt(sensorPin), increase, RISING);

  //WATER LEVEL SENSOR
  // Connect to Wi-Fi
  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  #ifdef ESP32
    client.setCACert(TELEGRAM_CERTIFICATE_ROOT); // Add root certificate for api.telegram.org
  #endif
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi..");
  }
  // Print ESP32 Local IP Address
  Serial.println(WiFi.localIP());
}

void loop() {
  //WATER LEVEL SENSOR
  value1 = analogRead(SIGNAL_PIN1); // read the analog value from sensor
  value2 = analogRead(SIGNAL_PIN2); // read the analog value from sensor
  delay(10);                      // wait 1s
  
  //SOLENOID VALVE
  //If water level sensor condition to solenoid
  if(value1 < 500){ //value 1 dari water level sensor 1
      //Serial.print("HABIS\n");
      digitalWrite(solenoidPin, HIGH); //Switch Solenoid ON
      delay(1000); //Wait 1 Second
      //bot.sendMessage(chat_id, "SEDANG MENGISI");
  }
  
  else if (value2 > 2000){ //value 2 dari water level sensor 2
      //Serial.print("PENUH\n");
      digitalWrite(solenoidPin, LOW); //Switch Solenoid OFF
      delay(1000); //Wait 1 Second
      //bot.sendMessage(chat_id, "SUDAH PENUH");
  }

  //FLOW METER SENSOR
  volume = 2.663 * pulse / 1000 * 30;
  if (millis() - lastTime > 2000) {
    pulse = 0;
    lastTime = millis();
  }
  flowMilliLitres = (volume / 60) * 1000;
  
  // Add the millilitres passed in this second to the cumulative total
  totalMilliLitres += flowMilliLitres;
    
  // Print the flow rate for this second in litres / minute
  Serial.print("Flow rate: ");
  Serial.print(int(volume));  // Print the integer part of the variable
  Serial.print("L/min");
  Serial.print("\t");       // Print tab space

  // Print the cumulative total of litres flowed since starting
  Serial.print("Output Liquid Quantity: ");
  Serial.print(totalMilliLitres);
  Serial.print("mL / ");
  Serial.print(totalMilliLitres / 1000);
  Serial.println("L");


  
  if (millis() > lastTimeBotRan + botRequestDelay) {
    int numNewMessages = bot.getUpdates(bot.last_message_received + 1); 
    while(numNewMessages) {
      Serial.println("got response"); handleNewMessages(numNewMessages);
      numNewMessages = bot.getUpdates(bot.last_message_received + 1);
    }
    lastTimeBotRan = millis();
  }

/*  
    //FLOW METER SENSOR
    if (text == "/Check_FMS") {
      volume = 2.663 * pulse / 1000 * 30;
      if (millis() - lastTime > 2000) {
        pulse = 0;
        lastTime = millis();
      }
      Serial.print(volume);
      Serial.println(" L/m");
    }

    if (text == "/Input_FMS") {
      welcome += "/2000\n";
        if (text == "/2000"){
          int a = 2000;
          if (Total == a){
              digitalWrite(solenoidPin, LOW);
          }
        }
      welcome += "/4000\n";
      welcome += "/6000\n";
      welcome += "/8000\n";
      welcome += "/10000\n";
    }
  
    if (text == "/Check_WLS") {
      if(value1 < 500){
          Serial.print("HABIS\n"); 
      }
      else if (value1 > 2000){
          Serial.print("HAMPIR HABIS\n"); 
      }
      else if (value2 < 500){
          Serial.print("HAMPIR PENUH\n"); 
      }
      else if (value2 > 2000){
          Serial.print("PENUH\n"); 
      }
*/ 
  delay(1000);
}
