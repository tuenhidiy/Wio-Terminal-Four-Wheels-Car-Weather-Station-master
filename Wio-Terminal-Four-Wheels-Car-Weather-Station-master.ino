/*!
* Wio Terminal - The Weather Station On The Four-Wheels Bluetooth Controlled Car
* By TUENHIDIY 
*/

#define BLYNK_PRINT Serial
#define BLYNK_USE_DIRECT_CONNECT
 
#include <BlynkSimpleWioTerminal_BLE.h>
#include <BLEDevice.h>
#include <BLEServer.h>

BlynkTimer timer;                                   // Define a Blynk timer for weather information

// Wio Terminal LCD properties
#include <TFT_eSPI.h>
#include "Free_Fonts.h"
TFT_eSPI tft = TFT_eSPI();
TFT_eSprite Car_Image = TFT_eSprite(&tft);          // Define a Sprite shown on the LCD screen

#define WIDTH   320                                // Difine Sprite width  
#define HEIGHT  48                                 // Difine Sprite height

#include <Wire.h>
#define WIRE Wire

// Temperature and humidity DHT11 sensor
#include <DHT.h>
#define DHTPIN  D4                                  // Connect to pin D4/A4 Wio Terminal
#define DHTTYPE DHT11                               // DHT 11 
DHT dht(DHTPIN, DHTTYPE);
float temperature = 0;                              // Actual temperature
float humidity = 0;                                 // Actual humidity
String temperatureStr, humidityStr;                 // Temperature & humidity in String format

// Pressure sensor
#include <Seeed_BMP280.h>
BMP280 bmp280;                                      
float pressure;                                     // Actual pressure
String pressureStr;                                 // Pressure in String format

// Light sensor
int light;                                          // Actual light value
String lightStr;                                    // Light in String format

// You should get your own Auth Token in the Blynk App.
char auth[] = "Your Own Auth Token";

// Pin connection from Wio Terminal to 
const int SPD1_PIN = 0;  // Motor 1 Speed Pin D0 (BCM27).
const int SPD2_PIN = 2;  // Motor 2 Speed Pin D2 (BCM23).
const int SPD3_PIN = 6;  // Motor 3 Speed Pin D6 (BCM13).
const int SPD4_PIN = 8;  // Motor 4 Speed Pin D8 (BCM26).

const int DIR1_PIN = 1;  // Motor 1 Direction Pin D1 (BCM22).
const int DIR2_PIN = 3;  // Motor 2 Direction Pin D3 (BCM24).
const int DIR3_PIN = 5;  // Motor 3 Direction Pin D5 (BCM12).
const int DIR4_PIN = 7;  // Motor 4 Direction Pin D7 (BCM16).

//int SET_SPEED;
int INIT_SPEED = 0;

// Motors properties
typedef struct
{
  int SPD_PIN;    // Speed Pin
  int DIR_PIN;    // Direction Pin
  int SET_SPEED;  // Speed to be set
} Motor;

Motor MotorFrontLeft  {SPD1_PIN, DIR1_PIN, INIT_SPEED}; // Motor 1 - M1 Terminal on the Quad Motor Driver Shield
Motor MotorFrontRight {SPD2_PIN, DIR2_PIN, INIT_SPEED}; // Motor 2 - M2 Terminal on the Quad Motor Driver Shield
Motor MotorBackLeft   {SPD3_PIN, DIR3_PIN, INIT_SPEED}; // Motor 3 - M3 Terminal on the Quad Motor Driver Shield
Motor MotorBackRight  {SPD4_PIN, DIR4_PIN, INIT_SPEED}; // Motor 4 - M4 Terminal on the Quad Motor Driver Shield

// Blynk command
int Forwardcmd_V1;
int Rightcmd_V2;
int Backwardcmd_V3;
int Leftcmd_V4;
int Speed_V5;

// Speed ratio
#define RatioM1     1.0
#define RatioM2     1.0
#define RatioM3     0.9
#define RatioM4     0.9

void setup()
{
  // Debug console
  Serial.begin(9600);
  Serial.println("Waiting for connections...");
  
  Blynk.setDeviceName("Blynk");
  Blynk.begin(auth);
 
  tft.begin();
  tft.setRotation(3);
  tft.fillScreen(TFT_BLACK);
  
  // LCD start-up information 
  // Header "Four Wheels RC Car" 
  Car_Image.createSprite(WIDTH, HEIGHT/2);
  Car_Image.fillSprite(TFT_BLACK);    //TFT_GREEN
  Car_Image.setTextColor(TFT_WHITE);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("Four Wheels RC Car", 30, 2);  
  Car_Image.pushSprite(0, 0); 
  Car_Image.deleteSprite();
  /*
  // Show car control command from Blynk "STOP"
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("STOP", 90, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
  */
  // Show car SPEED = 0 
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("SPEED: 0%", 10, 6);  
  Car_Image.pushSprite(0, (3*HEIGHT/2) + 1);
  Car_Image.deleteSprite();
  
  // Show header "Weather Information"
  Car_Image.createSprite(WIDTH, HEIGHT/2);
  Car_Image.fillSprite(TFT_BLACK);  //TFT_GREEN
  Car_Image.setTextColor(TFT_WHITE);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("Weather Information", 30, 2);  
  Car_Image.pushSprite(0, (5*HEIGHT/2) + 1);
  Car_Image.deleteSprite();

  // Show Temperature "T:....*C"
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("T:....*C", 2, 12);  
  Car_Image.pushSprite(0, (6*HEIGHT/2) + 2);
  Car_Image.deleteSprite(); 
  
  // Show Humidity "H:...%"
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("H:...%", 12, 12);  
  Car_Image.pushSprite(WIDTH/2, (6*HEIGHT/2) + 2);
  Car_Image.deleteSprite();
  
  // Show Pressure "P:......kPA"
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("P:......kPA", 2, 12);  
  Car_Image.pushSprite(0, (8*HEIGHT/2) + 1);
  Car_Image.deleteSprite();
  
  // Show Light "L:....lux"
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString("L:....lux", 15, 12);  
  Car_Image.pushSprite(WIDTH/2, (8*HEIGHT/2) + 1);
  Car_Image.deleteSprite();
 
  pinMode(SPD1_PIN,OUTPUT);
  pinMode(SPD2_PIN,OUTPUT);
  pinMode(SPD3_PIN,OUTPUT);
  pinMode(SPD4_PIN,OUTPUT);
  
  pinMode(DIR1_PIN,OUTPUT);
  pinMode(DIR2_PIN,OUTPUT);
  pinMode(DIR3_PIN,OUTPUT);
  pinMode(DIR4_PIN,OUTPUT);

  pinMode(WIO_LIGHT, INPUT);
  dht.begin();
  bmp280.init();
  
  // Set-up 2 second intervals between BMP280 readings
  timer.setInterval(2000L, myTimerBMP280);
  // Set-up 3 second intervals between DHT11 readings
  timer.setInterval(3000L, myTimerDHT11);
  // Set-up 4 second intervals between Light sensor readings
  timer.setInterval(4000L, myTimerLight);

  // Enable E-Stop mode
  digitalWrite(MotorFrontLeft.DIR_PIN, 0);
  digitalWrite(MotorFrontRight.DIR_PIN, 0);
  digitalWrite(MotorBackLeft.DIR_PIN, 0);
  digitalWrite(MotorBackRight.DIR_PIN, 0);
  
  analogWrite(MotorFrontLeft.SPD_PIN, 0);
  analogWrite(MotorFrontRight.SPD_PIN, 0);
  analogWrite(MotorBackLeft.SPD_PIN, 0);
  analogWrite(MotorBackRight.SPD_PIN, 0);
  
  // Show "E-STOP" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("E-STOP", 80, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void RC_Car_Forward()
{
  // Set car Direction
  digitalWrite(MotorFrontLeft.DIR_PIN, 0);
  digitalWrite(MotorBackLeft.DIR_PIN, 0);
  digitalWrite(MotorFrontRight.DIR_PIN, 0);  
  digitalWrite(MotorBackRight.DIR_PIN, 0);
  
  // Set car Speed
  analogWrite(MotorFrontLeft.SPD_PIN, MotorFrontLeft.SET_SPEED);
  analogWrite(MotorFrontRight.SPD_PIN, MotorFrontRight.SET_SPEED);
  analogWrite(MotorBackLeft.SPD_PIN, MotorBackLeft.SET_SPEED);
  analogWrite(MotorBackRight.SPD_PIN, MotorBackRight.SET_SPEED); 
  
  // Show "FORWARD" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_MAROON);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("FORWARD", 70, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1); 
  Car_Image.deleteSprite();
}

void RC_Car_Backward()
{
  analogWrite(MotorFrontLeft.SPD_PIN, MotorFrontLeft.SET_SPEED);
  analogWrite(MotorFrontRight.SPD_PIN, MotorFrontRight.SET_SPEED);
  analogWrite(MotorBackLeft.SPD_PIN, MotorBackLeft.SET_SPEED);
  analogWrite(MotorBackRight.SPD_PIN, MotorBackRight.SET_SPEED);
  
  digitalWrite(MotorFrontLeft.DIR_PIN, 1);
  digitalWrite(MotorFrontRight.DIR_PIN, 1);
  digitalWrite(MotorBackLeft.DIR_PIN, 1);
  digitalWrite(MotorBackRight.DIR_PIN, 1);

  // Show "BACKWARD" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_OLIVE);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("BACKWARD", 70, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void RC_Car_Right()
{
  analogWrite(MotorFrontLeft.SPD_PIN, MotorFrontLeft.SET_SPEED);
  analogWrite(MotorFrontRight.SPD_PIN, MotorFrontRight.SET_SPEED);
  analogWrite(MotorBackLeft.SPD_PIN, MotorBackLeft.SET_SPEED);
  analogWrite(MotorBackRight.SPD_PIN, MotorBackRight.SET_SPEED);
  
  digitalWrite(MotorFrontLeft.DIR_PIN, 0);
  digitalWrite(MotorBackLeft.DIR_PIN, 0);
  digitalWrite(MotorFrontRight.DIR_PIN, 1);  
  digitalWrite(MotorBackRight.DIR_PIN, 1);
  
  // Show "TURN RIGHT" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_MAGENTA);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("TURN RIGHT", 30, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void RC_Car_Left()
{
  analogWrite(MotorFrontLeft.SPD_PIN, MotorFrontLeft.SET_SPEED);
  analogWrite(MotorFrontRight.SPD_PIN, MotorFrontRight.SET_SPEED);
  analogWrite(MotorBackLeft.SPD_PIN, MotorBackLeft.SET_SPEED);
  analogWrite(MotorBackRight.SPD_PIN, MotorBackRight.SET_SPEED);
  
  digitalWrite(MotorFrontLeft.DIR_PIN, 1);
  digitalWrite(MotorBackLeft.DIR_PIN, 1);
  digitalWrite(MotorFrontRight.DIR_PIN, 0);  
  digitalWrite(MotorBackRight.DIR_PIN, 0);

  // Show "TURN LEFT" command on LCD screen 
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_PURPLE);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("TURN LEFT", 30, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void RC_Car_Stop()
{
  analogWrite(MotorFrontLeft.SPD_PIN, 0);
  analogWrite(MotorFrontRight.SPD_PIN, 0);
  analogWrite(MotorBackLeft.SPD_PIN, 0);
  analogWrite(MotorBackRight.SPD_PIN, 0);
  
  digitalWrite(MotorFrontLeft.DIR_PIN, 0);
  digitalWrite(MotorFrontRight.DIR_PIN, 0);
  digitalWrite(MotorBackLeft.DIR_PIN, 0);
  digitalWrite(MotorBackRight.DIR_PIN, 0);
  
  // Show "STOP" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("STOP", 90, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void RC_Car_EStop()
{
  analogWrite(MotorFrontLeft.SPD_PIN, 0);
  analogWrite(MotorFrontRight.SPD_PIN, 0);
  analogWrite(MotorBackLeft.SPD_PIN, 0);
  analogWrite(MotorBackRight.SPD_PIN, 0);
  
  digitalWrite(MotorFrontLeft.DIR_PIN, 0);
  digitalWrite(MotorFrontRight.DIR_PIN, 0);
  digitalWrite(MotorBackLeft.DIR_PIN, 0);
  digitalWrite(MotorBackRight.DIR_PIN, 0);
  
  // Show "STOP" command on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString("E-STOP", 80, 6);  
  Car_Image.pushSprite(0, (HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

BLYNK_WRITE(V1)
{   
  Forwardcmd_V1 = param.asInt(); // Get value as integer
  if(Forwardcmd_V1)
  {
    RC_Car_Forward();
  }
  else
  {
    RC_Car_Stop();
  }
}

BLYNK_WRITE(V2)
{   
  Rightcmd_V2 = param.asInt(); // Get value as integer
  if(Rightcmd_V2)
  {
    RC_Car_Right();
  }
  else
  {
    RC_Car_Stop();
  }
}

BLYNK_WRITE(V3)
{   
  Backwardcmd_V3 = param.asInt(); // Get value as integer
  if(Backwardcmd_V3)
  {
    RC_Car_Backward();
  }
  else
  {
    RC_Car_Stop();
  }  
}

BLYNK_WRITE(V4)
{   
  Leftcmd_V4 = param.asInt(); // Get value as integer
  if(Leftcmd_V4)
  {
    RC_Car_Left();
  }
  else
  {
    RC_Car_Stop();
  }  
}

BLYNK_WRITE(V10)
{   
  Leftcmd_V4 = param.asInt(); // Get value as integer
  if(Leftcmd_V4)
  {
    RC_Car_EStop();
  }  
}

BLYNK_WRITE(V5)
{   
  Speed_V5 = param.asInt();              // Get value as integer
  
  // Set car speed from Blynk slider V5 to DC motors   
  MotorFrontLeft.SET_SPEED  = RatioM1 * Speed_V5;
  MotorFrontRight.SET_SPEED = RatioM2 * Speed_V5;
  MotorBackLeft.SET_SPEED   = RatioM3 * Speed_V5;
  MotorBackRight.SET_SPEED  = RatioM4 * Speed_V5;
  
  // Convert to String for showing on the LCD screen
  String value_V5_Str = String(100*Speed_V5/255);
  String SpeedStr = "SPEED: " + value_V5_Str + "%";
  
  // Show actual car speed on LCD screen
  Car_Image.createSprite(WIDTH, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold24pt7b);
  Car_Image.drawString(SpeedStr, 10, 6);  
  Car_Image.pushSprite(0, (3*HEIGHT/2) + 1);
  Car_Image.deleteSprite();

}

void myTimerBMP280()
{  
  readPressure();
  // Send to Blynk
  Blynk.virtualWrite(V6, pressure/1000);
  
  // Show actual pressure on LCD screen
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString(pressureStr, 2, 12);  
  Car_Image.pushSprite(0, (8*HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void myTimerDHT11()
{  
  readHum_Tem();
  // Send to Blynk
  Blynk.virtualWrite(V7, temperature);
  Blynk.virtualWrite(V8, humidity);
  
  // Show actual temperature on LCD screen
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString(temperatureStr, 2, 12);  
  Car_Image.pushSprite(0, (6*HEIGHT/2) + 2);
  Car_Image.deleteSprite(); 
  
  // Show actual humidity on LCD screen
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString(humidityStr, 12, 12);  
  Car_Image.pushSprite(WIDTH/2, (6*HEIGHT/2) + 2);
  Car_Image.deleteSprite();  
}

void myTimerLight()
{
  readLight();
  // Send to Blynk
  Blynk.virtualWrite(V9, light);
  
  // Show actual pressure on LCD screen
  Car_Image.createSprite(WIDTH/2, HEIGHT);
  Car_Image.fillSprite(TFT_BLACK);
  Car_Image.setTextColor(TFT_RED);
  Car_Image.setFreeFont(&FreeMonoBold12pt7b);
  Car_Image.drawString(lightStr, 15, 12);  
  Car_Image.pushSprite(WIDTH/2, (8*HEIGHT/2) + 1);
  Car_Image.deleteSprite();
}

void readHum_Tem()
{
  // Read temperature and humidity from DHT11
  temperature = dht.readTemperature();
  humidity = dht.readHumidity();
  // Convert to String for showing on the LCD screen
  temperatureStr = "T: " + String(temperature) + "*C";
  humidityStr = "H: " + String(humidity) + "%";
}

void readPressure()
{
  // Read the pressure from BMP280
  pressure = bmp280.getPressure();
  // Convert to String for showing on the LCD screen
  pressureStr = "P:" + String(pressure/1000) + "kPa";
}

void readLight()
{
  light = analogRead(WIO_LIGHT);
  lightStr = "L: " + String(light) + "lux";
}

void loop()
{ 
  Blynk.run();    // Running Blynk
  timer.run();    // Running Blynk timer
}
  
  
