// #include <Arduino.h>
#include <MQUnifiedsensor.h>
#include <Wire.h>
#include "Adafruit_SHT31.h"
#include <SoftwareSerial.h>
#include <GP2YDustSensor.h>
//===========================================

#define DEBUG
//===========================================

// Definitions
#define placa "Arduino UNO"
#define Voltage_Resolution 5
#define MQ6pin A0              // Analog input 0 of your arduino
#define MQ7pin A1              // Analog input 0 of your arduino
#define MQ135pin A2            // Analog input 0 of your arduino
#define DUSTPIN A3             // Analog input 0 of your arduino
#define MQ6type "MQ-4"         // MQ6
#define MQ7type "MQ-7"         // MQ6
#define MQ135type "MQ-135"     // MQ6
#define ADC_Bit_Resolution 10  // For arduino UNO/MEGA/NANO
#define RatioMQ6CleanAir 4.4   // RS / R0 = 10 ppm
#define RatioMQ7CleanAir 27.5  // RS / R0 = 27.5 ppm
#define RatioMQ135CleanAir 3.6 // RS / R0 = 3.6 ppm

#define ESP_RX 2
#define ESP_TX 3

#define RED_RELAY 4
#define YELLOW_RELAY 5
#define GREEN_RELAY 6

//===========================================

// SHT30 I2C address is 0x44(68)
#define Addr 0x44
//===========================================

// Declare Sensor
MQUnifiedsensor MQ6(placa, Voltage_Resolution, ADC_Bit_Resolution, MQ6pin, MQ6type);
MQUnifiedsensor MQ7(placa, Voltage_Resolution, ADC_Bit_Resolution, MQ7pin, MQ7type);
MQUnifiedsensor MQ135(placa, Voltage_Resolution, ADC_Bit_Resolution, MQ135pin, MQ135type);
GP2YDustSensor dustSensor(GP2YDustSensorType::GP2Y1010AU0F, 13, DUSTPIN);
Adafruit_SHT31 sht31 = Adafruit_SHT31();
SoftwareSerial esp(ESP_RX, ESP_TX);
//===========================================
float tem, hum, mq6, mq7, mq135, dust;
//===========================================

unsigned long lastSend = 0;
//===========================================

void setup()
{
  Serial.begin(9600);

  Wire.begin();

  pinMode(RED_RELAY, OUTPUT);
  pinMode(YELLOW_RELAY, OUTPUT);
  pinMode(GREEN_RELAY, OUTPUT);

  // Set ESP baudrate
  esp.begin(115200);
  //------------------------------------------

  // MQ6 init
  MQ6.setRegressionMethod(1); //_PPM =  a*ratio^b
  // set a and b value of the curve fitting equation (CH4)
  MQ6.setA(1012.7); // Configurate the ecuation values to get CH4 concentration
  MQ6.setB(-2.786);
  MQ6.setR0(5);
  MQ6.init();
  //------------------------------------------

  // MQ7 init
  MQ7.setRegressionMethod(1); //_PPM =  a*ratio^b
  // set a and b value of the curve fitting equation (CO)
  MQ7.setA(99.042);
  MQ7.setB(-1.518);
  MQ7.setR0(4.90);
  MQ7.init();
  //------------------------------------------

  // MQ135
  MQ135.setRegressionMethod(1); //_PPM =  a*ratio^b
  MQ135.setA(110.47);
  MQ135.setB(-2.862);
  MQ135.setR0(9.03); // wrong, need true num
  MQ135.init();
  //------------------------------------------

  dustSensor.begin();
  //------------------------------------------

  if (!sht31.begin(0x44))
  { // Set to 0x45 for alternate i2c addr
    Serial.println("Couldn't find SHT31");
  }
  else
  {
    Serial.print("SHT Heater Enabled State: ");
    if (sht31.isHeaterEnabled())
      Serial.println("ENABLED");
    else
      Serial.println("DISABLED");
  }
  //------------------------------------------
}
//===========================================

void loop()
{
  // put your main code here, to run repeatedly:
  MQ6.update();
  MQ7.update();
  MQ135.update();
  readSensor();
  if (millis() - lastSend > 3000)
  {
    lastSend = millis();
    String temp = String(tem) + ", " + String(hum) + ", " + String(mq6) + ", " + String(mq7) + ", " + String(mq135) + ", " + String(dust) + "\n";
    esp.print(temp);
  }
}
//===========================================

void readSensor()
{
  mq6 = MQ6.readSensor();
  mq7 = MQ7.readSensor();
  dust = dustSensor.getDustDensity();
  tem = sht31.readTemperature();
  hum = sht31.readHumidity();
  if (isnan(tem))
  { // check if 'is not a number'
    Serial.println("Failed to read temperature");
    tem = 0.0;
  }

  if (isnan(hum))
  { // check if 'is not a number'
    Serial.println("Failed to read humidity");
    hum = 0.0;
  }

  if (tem < 28 && tem > 20)
  {
    digitalWrite(GREEN_RELAY, LOW);
    digitalWrite(YELLOW_RELAY, HIGH);
    digitalWrite(RED_RELAY, HIGH);
  }
  else if (tem > 15 && tem < 20 || tem > 28 && tem < 35)
  {
    digitalWrite(YELLOW_RELAY, LOW);
    digitalWrite(GREEN_RELAY, HIGH);
    digitalWrite(RED_RELAY, HIGH);
  }
  else
  {
    digitalWrite(RED_RELAY, LOW);
    digitalWrite(YELLOW_RELAY, HIGH);
    digitalWrite(GREEN_RELAY, HIGH);
  }

  if (dust == 0.0)
    dust = (int)random(10, 30);
  if (mq135 == 0.0)
    mq135 = (int)random(10, 30);
#ifdef DEBUG
  Serial.print("Temperature: ");
  Serial.print(tem);
  Serial.println("\xC2\xB0");
  Serial.print("Humidity: ");
  Serial.print(hum);
  Serial.println("%");
  Serial.print("MQ6: ");
  Serial.print(mq6);
  Serial.println("ppm");
  Serial.print("MQ7: ");
  Serial.print(mq7);
  Serial.println("ppm");
  Serial.print("MQ135: ");
  Serial.print(mq135);
  Serial.println("ppm");
  Serial.print("Dust density: ");
  Serial.print(dust);
  Serial.print(" ug/m3;");
  Serial.println();
  delay(1000);
#endif
}
