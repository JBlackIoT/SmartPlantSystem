/*
 * Project 11_SmartPlant_SystemCode3
 * Description:smartplantsytem
 * Author:JBlack
 * Date:03-17-23
 */


#include <Adafruit_MQTT.h> //Include the Adafruit MQTT library

#include "Adafruit_MQTT/Adafruit_MQTT.h"  
#include "Adafruit_MQTT/Adafruit_MQTT_SPARK.h" 
#include "Adafruit_MQTT/Adafruit_MQTT.h" 

#include "credentials.h" //Include credentials for my Adafruit IO account

#include <Adafruit_BME280.h> //Include the Adafruit BME280 library for temperature, pressure, and humidity sensors
#include <Adafruit_SSD1306.h> //Include the Adafruit SSD1306 library for the OLED display

#include "Air_Quality_Sensor.h" //Include Air quality sensor Library

const int OLED_RESET = D0;  //Reset pin for the OLED display

Adafruit_SSD1306 display(OLED_RESET); //Create an instance of the OLED display

Adafruit_BME280 bme; //Create an instance of the BME280 sensor

AirQualitySensor sensor(A4); //Create an instance of the Air Quality Sensor

TCPClient TheClient; //Create a TCP client instance for the MQTT connection
 
Adafruit_MQTT_SPARK mqtt(&TheClient,AIO_SERVER,AIO_SERVERPORT,AIO_USERNAME,AIO_KEY); //Create an instance of the MQTT client and connect to Adafruit IO

Adafruit_MQTT_Subscribe buttonState = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/buttonOnOff");  //Subscribe to the buttonOnOff feed
Adafruit_MQTT_Publish soilRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/moisturelevel"); //Publish to the moisturelevel feed
Adafruit_MQTT_Publish tempRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/temperature"); //Publish to the Temperature Reading feed
Adafruit_MQTT_Publish pressureRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/pressure"); //Publish to the Pressure Reading feed
Adafruit_MQTT_Publish humidityRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/humidity"); //Publish to the Humidity Reading feed
Adafruit_MQTT_Publish airRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/air-quality-sensor");  //Publish to the AirQuality Reading feed
Adafruit_MQTT_Publish dustRead = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/dustsensor");  ////Publish to the Dust Reading feed


float temp; //Variable to store temperature
float pressure; //Variable to store pressure
float humidity; //Variable to store humidity
unsigned long last; //Variable to store humidity
unsigned long lastTime; //last MQTT ping time
unsigned long pumpLastTime; //Variable to store last sensor read time
unsigned long dustStartTime; //Variable to store dust sensor start time
unsigned long duration; //Variable to store pulse duration from the dust sensor

int value; //stores value subcribed from adafruit.io
int airQuality; //stores value from Air Quality Sensor
int capRead; //stores capacitive soil sensor reading
int slopeQuality; //Variable to store slope value from Air Quality Sensor
char dangerReading; //Variable to store danger reading from Air Quality Sensor
int dust; //Variable to store dust reading
float lowPulseOccupancy; 

float concentration = 0;
float realConcentration;
float ratio;

const int ledPin = D7; //pin to turn on D7 lil mini led just so I know im getting a digital signal
const int soilPin = A0; // analog input pin for the soil moisture sensor
const int dustPin = A3; // analog input pin for dust sensor
const int airPin = A4;  // analog input pin for air quality sensor
const int pumpPin = D11;  // digital output pin to turn on the water pump

void setup() {
  Serial.begin(9600);
  display.begin(SSD1306_SWITCHCAPVCC, 0x3C); // initialize the OLED display
  display.display(); // turn on the OLED display
  delay(2000); // wait for 2 seconds
  Wire.begin();
  bme.begin(0x76);  // initialize the BME280 sensor


  if (sensor.init()) {
      Serial.printf("Sensor ready."); // print message to the serial monitor
  } else {
      Serial.printf("Sensor ERROR!");
  }  

  mqtt.subscribe(&buttonState); // subscribe to the buttonOnOff feed on adafruit.io


  pinMode(soilPin, INPUT); 
  pinMode(pumpPin, OUTPUT);
  pinMode(ledPin, OUTPUT);
  pinMode(airPin, INPUT);
  pinMode(dustPin, INPUT);

  dustStartTime = millis();
}

void loop() {

  duration = pulseIn(dustPin, LOW);
  lowPulseOccupancy = lowPulseOccupancy + duration;
  if ((millis()-dustStartTime) >= 30000){
    ratio = lowPulseOccupancy/(30000*10.0);
    concentration = 1.1*pow(ratio,3)-3.8*pow(ratio,2)+520*ratio+0.62;
    if(concentration > 1){
      realConcentration = concentration;
    }
    Serial.printf("Concentration = %f pcs/0.01cf",realConcentration);
    lowPulseOccupancy = 0;
    dustStartTime = millis();
  }

  int slopeQuality = sensor.slope();
  if (slopeQuality == AirQualitySensor::FORCE_SIGNAL) {
  Serial.printf("High pollution! Force signal active.");
  }
  else if (slopeQuality == AirQualitySensor::HIGH_POLLUTION) {
    Serial.printf("High pollution!");
  }
  else if (slopeQuality == AirQualitySensor::LOW_POLLUTION) {
    Serial.printf("Low pollution!");
  }
  else if (slopeQuality == AirQualitySensor::FRESH_AIR) {
    Serial.printf("Fresh air.");
  }

  display.clearDisplay();
  display.setRotation(2);
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0,0);
  temp = ((bme.readTemperature()*9/5)+32);
  pressure = ((bme.readPressure()/100)*0.03);
  humidity = bme.readHumidity();
  airQuality = sensor.getValue();

  display.printf("Soil: %i\nTemp: %.2f%c\nPressure: %.2f\nHumidity: %.2f%c\nAir Quality: %i\nDust: %.2f\n",capRead,temp,(char)247,pressure,humidity,(char)37,airQuality,realConcentration);
  display.display();

  MQTT_connect();

  if ((millis()-last)>5000) {
      Serial.printf("Pinging MQTT \n");
      if(! mqtt.ping()) {
        Serial.printf("Disconnecting \n");
        mqtt.disconnect();
      }
      last = millis();
  }
 
  Adafruit_MQTT_Subscribe *subscription;
  while ((subscription = mqtt.readSubscription(10000))) {   // info for dashboard button
    if (subscription == &buttonState) {
      value = atof((char *)buttonState.lastread);
      if(value == 1){
      digitalWrite(ledPin, HIGH); // so I know pump is getting digital signal
      digitalWrite(pumpPin, HIGH);
      }
      else{
      digitalWrite(ledPin, LOW);
      digitalWrite(pumpPin, LOW);
      }
    }
  }

  if((millis()-lastTime > 30000)) {
    if(mqtt.Update()) {
      analogWrite(pumpPin, HIGH);
      capRead = analogRead(soilPin);
      soilRead.publish(capRead);
      tempRead.publish(temp);
      pressureRead.publish(pressure);
      humidityRead.publish(humidity);
      airRead.publish(airQuality);
      dustRead.publish(concentration);
      Serial.printf("Soil Read: %i Temp: %f Pressure: %f Humidity %f Air Quality: %i Dust: %f\n",capRead,temp,pressure,humidity, airQuality, concentration); 
      } 
    lastTime = millis();
  }

  if((millis()-pumpLastTime > 60000)) { //now we are watering plant automatic
    if(capRead > 1600){
    digitalWrite(pumpPin, HIGH);
    digitalWrite(ledPin, HIGH);
    Serial.printf("Watering plant\n");
    delay(1000);
    digitalWrite(pumpPin, LOW); 
    digitalWrite(ledPin, LOW);
    Serial.printf("Stop Watering plant\n");
    }
    pumpLastTime = millis();
  }

}

void MQTT_connect() {
  int8_t ret;
 
  if (mqtt.connected()) {
    return;
  }
 
  Serial.print("Connecting to MQTT... ");
 
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.printf(mqtt.connectErrorString(ret));
       Serial.printf("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
  }
  Serial.printf("MQTT Connected!");
}