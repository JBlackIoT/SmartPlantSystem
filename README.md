# SmartPlantSystem
Self Watering system with air quality detection 
The Deep Dive Calming Ecosystem is a self watering plant system that delivers real time room conditions. Many IoT projects are great at alarming users when things are wrong, or streamlining manufacturing/production to be more lean and efficent. This project encourages the end user to feel re-assured that their plant is watered , the room is at an optimal , safe and comfortable level, and they can slow down and take a moment to be calm. With this system IoT is deployed in an effort to encourage us to take small moments that the tech provides to engage more with our own humanity. 
The plant itself has a capacitive soil moisture sensor embedded into the soil that reads the level of moisture and notifies the system if the soil is dry. When the sensor detects a drop in moisture level the system triggers a pump that pumps water into the plant for 1.5 seconds. 
Two SEEED sensors, grove dust monitor and the air quality sensor tell us the level of dust and particulate matter in the air, and the pollution levels.
An BME 280 sensor delivers to us real time temperature, humidity, and pressure levels. 
All information is displayed on a OLED screen for quick look at room data.
Room conditions and soil moisture level are also integrated into a user friendly Adafruit.IO dashboard that gives a dynamic visual representation of separate data feeds.
