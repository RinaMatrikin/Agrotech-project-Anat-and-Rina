
// Wifi and Thingspeak setup:
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_ADS1X15.h>

Adafruit_ADS1115 ads;  /* Use this for the 16-bit version */



#include "ThingSpeak.h"
unsigned long myChannelNumber = ;//add your channel number on thingspeak
const char * myWriteAPIKey = "";//ass API Key



const char* ssid = ""; // your wifi SSID name
const char* password = "" ;// wifi pasword
 
const char* server = "api.thingspeak.com";

WiFiClient client;




#include "HX711.h"
// HX711 circuit wiring
const int LOADCELL_DOUT_PIN = 17;
const int LOADCELL_SCK_PIN = 18;
float zero;
HX711 scale;
 
void WeightLoop();
void PreasureLoop();

void setup() {
  Serial.begin(9600);

  WiFi.disconnect();
  delay(10);
  WiFi.begin(ssid, password);
  Serial.println();
  Serial.println();
  Serial.print("Connecting to ");
  Serial.println(ssid);
  ThingSpeak.begin(client);
  WiFi.begin(ssid, password);
 
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println("");
  Serial.print("NodeMcu connected to wifi...");
  Serial.println(ssid);
  Serial.println();


  //Begin Scale
  scale.begin(LOADCELL_DOUT_PIN, LOADCELL_SCK_PIN);
  delay(1000);
  if (scale.is_ready()) {
    long reading = scale.read();
    //Serial.print("HX711 reading: ");
    zero = reading*0.0189-157017;
  } else {
    Serial.println("HX711 not found.");
  }
  //End Scale
  //Preasure
 

  Serial.println("Getting single-ended readings from AIN0..3");
  Serial.println("ADC Range: +/- 6.144V (1 bit = 3mV/ADS1015, 0.1875mV/ADS1115)");

  ads.setGain(GAIN_ONE);        // 1x gain   +/- 4.096V  1 bit = 2mV      0.125mV


  if (!ads.begin()) {
    Serial.println("Failed to initialize ADS.");
    while (1);
  }
  //End Preasure
}

void PreasureLoop(){
  float normalizationValue, kpa_to_cm;
  float pressure0,pressure1;
  float sensor_signal0,sensor_signal1;
  int16_t adc0, adc1;
  float volts0, volts1;
  float volts0Correct,volts1Correct;
  
  adc0 = ads.readADC_SingleEnded(0);   //Reads from preassure sensor 0
  adc1 = ads.readADC_SingleEnded(1);  //Reads from preassure sensor 1
  volts0 = ads.computeVolts(adc0);   //Voltage of preassure sensor 0
  volts1 = ads.computeVolts(adc1);  //Voltage of preassure sensor 1


  volts0Correct =  2* volts0*100 / 4.096;    //Voltage correction to +/- 4.096V  1 bit = 2mV
  volts1Correct = 2* volts1*100 / 4.096;    //Voltage correction to +/- 4.096V  1 bit = 2mV
  
  sensor_signal0=adc0*0.125*0.001 * volts0Correct;   //Sensor 0 value calculation with voltage
  sensor_signal1=adc1*0.125*0.001* volts1Correct;   //Sensor 1 value calculation with voltage
  
  pressure0 = (sensor_signal0-0.2)/0.045; //Sensor 0 Preassure final value 
  pressure1 = (sensor_signal1-0.2)/0.045;  //Sensor 1 Preassure final value 

  normalizationValue = 1.13 / 4.34;   //Normalization to default atmospheric value

  pressure0 = pressure0 * normalizationValue; //Sensor 0 Preassure final value Normalized
  pressure1 = pressure1 * normalizationValue;  //Sensor 1 Preassure final value Normalized 

  kpa_to_cm = 10.2;


  pressure0 = pressure0 * kpa_to_cm; //Sensor 0 Preassure final value Normalized in cmH2O
  pressure1 = pressure1 * kpa_to_cm;  //Sensor 1 Preassure final value Normalized in cmH2O
  
  Serial.print("ADC0: ");
  Serial.print(adc0);
  Serial.print(" VOLTS0: ");
  Serial.print(volts0);
  Serial.println();
  Serial.print("PREASSURE: ");
  Serial.print(pressure0);
  Serial.println();
  
  Serial.print("ADC1: ");
  Serial.print(adc1);
  Serial.print(" VOLTS1: ");
  Serial.print(volts1);
  Serial.println();
  Serial.print("PREASSURE: ");
  Serial.print(pressure1);
  Serial.println();
    
  Serial.println("-----------------------------------------------------------");
  ThingSpeak.setField(2,pressure0);
  ThingSpeak.setField(3,pressure1);

  ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  delay(1000);
 
}

void WeightLoop(){
    if (scale.is_ready()) {
    long reading = scale.read();
    //Serial.print("HX711 reading: ");
    long reading_formule = abs(reading*0.0189-157017-zero)*1.05;
    Serial.println(reading_formule);

    //Thingspeak setup:
    ThingSpeak.setField(1,reading_formule);
  
    ThingSpeak.writeFields(myChannelNumber, myWriteAPIKey);
  
    Serial.println("uploaded to Thingspeak server....");
  

    
  } else {
    Serial.println("HX711 not found.");
  }
  delay(100);

}
 
void loop() {

  WeightLoop();
  PreasureLoop();

  client.stop();

  
}
