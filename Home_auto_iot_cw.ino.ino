// Blynk
// Template ID, Device Name and Auth Token are provided by the Blynk.Cloud
// See the Device Info tab, or Template settings
#define BLYNK_TEMPLATE_ID           "TMPLC4J51I1D"
#define BLYNK_DEVICE_NAME           "Quickstart Device"
#define BLYNK_AUTH_TOKEN            "h50Ky-zPCLRXRoHUb3qax1TFQBjs-oEV"

// Comment this out to disable prints and save space
#define BLYNK_PRINT Serial

// Libraries
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include "DHTesp.h"
#include <arduino-timer.h>

DHTesp dht; 
auto timer_ntdft = timer_create_default();

// PIR enable/disable key
bool pirKey = 0;

// Lamp operation auto mode
bool opMode = 0;

// Daylight status
bool statDaylight = 0;

// onStart lampOn key
bool onStartLampKey = 0;

// Gas detection system key
bool gasSystemKey = 0;

// Define Component pins
#define Buzzer D0
#define Temp D1
#define PIR D2
#define MQ2 D3
#define relay1 D4
#define relay2 D5
#define relay3 D6
#define relay4 D7
#define LDR D8
#define LED_R D10
#define LED_G D9
#define GasAnalog A0

char auth[] = BLYNK_AUTH_TOKEN;

// Your WiFi credentials.
// Set password to "" for open networks.
char ssid[] = "SLT-FIBER";
char pass[] = "HOME0773883350";

BlynkTimer timer;

// This function is called every time the Virtual Pin 0 state changes
BLYNK_WRITE(V0)
{
  // Set incoming value from pin V0 to a variable
  int value = param.asInt();
  pirKey = value;

  // Update state
  Blynk.virtualWrite(V0, value);
}

BLYNK_WRITE(V11)
{
  // Set incoming value from pin V11 to a variable
  int value = param.asInt();
  opMode = value;
  
  // Update state
  Blynk.virtualWrite(V11, value);
}

BLYNK_WRITE(V16)
{
  // Set incoming value from pin V16 to a variable
  int value = param.asInt();
  gasSystemKey = value;
  
  // Update state
  Blynk.virtualWrite(V16, value);
}

void ActionONMode(){
  //switch with opMode value
  if(onStartLampKey == 1) {
  
  // turn on switch relays
  digitalWrite(relay1,LOW);
  digitalWrite(relay2,LOW);
  digitalWrite(relay3,LOW);
  digitalWrite(relay4,LOW);
  Serial.print("[MODE] - Startup Lamps ON");
  timer_ntdft.in(60000, ActionOFFMode);
  onStartLampKey = 0;
  }  
  // if night in Auto Lamp Mode
  else if (opMode == 1 && statDaylight == 0){
    // power lamps on when PIR detects a movement
    if(digitalRead(PIR)){
      Serial.print("[MODE] - PIR night mode");
      digitalWrite(relay4,LOW);
      timer_ntdft.in(300000, [](void*) -> bool { 
        digitalWrite(relay4,HIGH);
        return true; 
        });
    }
  }
  else{
    ActionOFFMode;
  }
}

bool ActionOFFMode(void *){
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);
  return true;
}

// This function is called every time the device is connected to the Blynk.Cloud
BLYNK_CONNECTED()
{
  // Change Web Link Button message to "Congratulations!"
  Blynk.setProperty(V3, "offImageUrl", "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations.png");
  Blynk.setProperty(V3, "onImageUrl",  "https://static-image.nyc3.cdn.digitaloceanspaces.com/general/fte/congratulations_pressed.png");
  Blynk.setProperty(V3, "url", "https://docs.blynk.io/en/getting-started/what-do-i-need-to-blynk/how-quickstart-device-was-made");
}

// This function sends Arduino's uptime every second to Virtual Pin 2.
void myTimerEvent()
{
  // You can send any value at any time.
  // Please don't send more that 10 values per second.
  int millisecs = millis();
  int sec = millisecs/1000;
  int mins = sec/60;
  int hrs = mins/60;
  
  String hrMinSec = (String(hrs)+":"+String(mins-(hrs*60))+":"+String(sec-(mins*60)));
  
  Blynk.virtualWrite(V2, hrMinSec);
}

void setup()
{
  // Debug console
  Serial.begin(115200);

  // lamp key status true when first setup (only)
  onStartLampKey = 1;

  Blynk.begin(auth, ssid, pass);

  // Setup a function to be called every second
  timer.setInterval(100L, myTimerEvent);
  timer.setInterval(100L, DHT11sensor);
  timer.setInterval(100L, PIRsensor);
  timer.setInterval(100L, MQ2sensor);
  timer.setInterval(100L, LDRsensor);
  timer.setInterval(100L, ActionONMode);
  
  

  // Setup pins
  pinMode(Buzzer, OUTPUT);
  pinMode(PIR, INPUT);
  pinMode(MQ2, INPUT);
  pinMode(LDR, INPUT);
  pinMode(relay1, OUTPUT);
  pinMode(relay2, OUTPUT);
  pinMode(relay3, OUTPUT);
  pinMode(relay4, OUTPUT);
  pinMode(LED_R, OUTPUT);
  pinMode(LED_G, OUTPUT);

  // Set default status on relay outputs
  digitalWrite(relay1,HIGH);
  digitalWrite(relay2,HIGH);
  digitalWrite(relay3,HIGH);
  digitalWrite(relay4,HIGH);

  // Setup dht11 
  dht.setup(Temp,DHTesp::DHT11);
  
  // set security button to off
   Blynk.virtualWrite(V0, 0);
   Blynk.virtualWrite(V11, 0);
   Blynk.virtualWrite(V16, 0);
}

//Get the DHT11 sensor values
void DHT11sensor() {
  float h = dht.getHumidity();
  float t = dht.getTemperature();

  if (isnan(h) || isnan(t)) {
    Serial.println("Failed to read from DHT sensor!");
    return;
  }
  Blynk.virtualWrite(V4, t);
  Blynk.virtualWrite(V5, h);

}

//Get the PIR sensor values
void PIRsensor() {
  bool value = digitalRead(PIR);
  if (pirKey == 1) {
    if (value == 0) {
      digitalWrite(Buzzer, LOW);
      Blynk.virtualWrite(V12, 0);
    } else if (value == 1) {
      Blynk.notify("Warning! Please check your security system");
      Blynk.virtualWrite(V12, 1);
      digitalWrite(Buzzer, HIGH);
    }
  }
}

//Get the MQ2 sensor values
void MQ2sensor() {
  int value = digitalRead(MQ2);
  int val2 = analogRead(GasAnalog);

  if (isnan(value) || isnan(val2)) {
    Serial.println("Failed to read from MQ2 sensor!");
    return;
  }
  
  val2 = map(val2, 0, 1024, 0, 100);

  if(gasSystemKey == 1){
    if (val2<40) {
    Blynk.virtualWrite(V13, 0);
    Blynk.virtualWrite(V15, "Good");
    digitalWrite(Buzzer, LOW);
    digitalWrite(LED_G,HIGH);
    digitalWrite(LED_R,LOW);
  }
  else if (val2>40) {
    Blynk.notify("Warning! Gas leak detected");

    // control noise according to gas level
    if(val2<60){
       tone(Buzzer,100,1500);
       Blynk.virtualWrite(V15,"Moderate");
    }
    else if(val2>80){
       tone(Buzzer,2000,1500);
       Blynk.virtualWrite(V15,"Hazardous");
    }
    
    Blynk.virtualWrite(V13, 1);
    digitalWrite(LED_R,HIGH);
    digitalWrite(LED_G,LOW);
    }
  }
  else{
    Blynk.virtualWrite(V15, "Disabled");
    digitalWrite(LED_G,HIGH);
    timer_ntdft.in(2500, [](void *) -> bool {
      digitalWrite(LED_G,LOW);
      return true; 
      });
  }
  // show data real time in gas guage
  Blynk.virtualWrite(14, val2);
}

//Get LDR values
void LDRsensor(){
  int value = digitalRead(LDR);

  if (value == 1) {
    Blynk.virtualWrite(V10, 0);
    statDaylight = 0;
  } else if (value == 0) {
    Blynk.virtualWrite(V10, 1);
    statDaylight = 1;
  }
}

//Get buttons_relay1 values
BLYNK_WRITE(V6) {
 bool Relay1 = param.asInt();
  if (Relay1 == 1) {
    digitalWrite(relay1, LOW);
  } else {
    digitalWrite(relay1, HIGH);
  }
}

//Get buttons_relay2 values
BLYNK_WRITE(V7) {
 bool Relay2 = param.asInt();
  if (Relay2 == 1) {
    digitalWrite(relay2, LOW);
  } else {
    digitalWrite(relay2, HIGH);
  }
}

//Get buttons_relay3 values
BLYNK_WRITE(V8) {
 bool Relay3 = param.asInt();
  if (Relay3 == 1) {
    digitalWrite(relay3, LOW);
  } else {
    digitalWrite(relay3, HIGH);
  }
}

//Get buttons_relay4 values
BLYNK_WRITE(V9) {
 bool Relay4 = param.asInt();
  if (Relay4 == 1) {
    digitalWrite(relay4, LOW);
  } else {
    digitalWrite(relay4, HIGH);
  }
}
void loop()
{
  Blynk.run();
  timer.run();
  timer_ntdft.tick();
  // You can inject your own code or combine it with other sketches.
  // Check other examples on how to communicate with Blynk. Remember
  // to avoid delay() function!
}
