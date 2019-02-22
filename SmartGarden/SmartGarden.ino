/***************************************************
  Adafruit MQTT Library ESP8266 Example

  Must use ESP8266 Arduino from:
    https://github.com/esp8266/Arduino

  Works great with Adafruit's Huzzah ESP board & Feather
  ----> https://www.adafruit.com/product/2471
  ----> https://www.adafruit.com/products/2821

  Adafruit invests time and resources providing this open source code,
  please support Adafruit and open-source hardware by purchasing
  products from Adafruit!

  Written by Tony DiCola for Adafruit Industries.
  MIT license, all text above must be included in any redistribution
 ****************************************************/
#include <ESP8266WiFi.h>
#include "Adafruit_MQTT.h"
#include "Adafruit_MQTT_Client.h"
#include <DHT.h>

/************************* WiFi Access Point *********************************/

#define WLAN_SSID       "Seger"
#define WLAN_PASS       "JusAlpukat223"

/************************* Adafruit.io Setup *********************************/

#define AIO_SERVER      "io.adafruit.com"
#define AIO_SERVERPORT  1883                   // use 8883 for SSL
#define AIO_USERNAME    "hidrops"
#define AIO_KEY         "4f080277071f4d5a87160603e0485f18" // You can see this code in adafruit.io in your feeds and account

/************************* pinMode definition ********************************/

#define LED_PIN D6
#define DHT11_PIN D3
DHT dht(D3, DHT11); //Pin, Jenis DHT

/************ Global State (you don't need to change this!) ******************/

// Create an ESP8266 WiFiClient class to connect to the MQTT server.
WiFiClient client;
// or... use WiFiFlientSecure for SSL
//WiFiClientSecure client;

// Setup the MQTT client class by passing in the WiFi client and MQTT server and login details.
Adafruit_MQTT_Client mqtt(&client, AIO_SERVER, AIO_SERVERPORT, AIO_USERNAME, AIO_KEY);

/****************************** Feeds ***************************************/

// Setup a feed called 'Temperature_Statistics' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish  Temperature_Statistics   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ Temperature_Statistics  ");

// Setup a feed called ' Temperature_Statistics  ' for subscribing to changes.
Adafruit_MQTT_Subscribe  Temperature_Statisticsbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ Temperature_Statistics  ");

// Setup a feed called 'Temperature_Statistics' for publishing.
// Notice MQTT paths for AIO follow the form: <username>/feeds/<feedname>
Adafruit_MQTT_Publish  Humidity_Statistics   = Adafruit_MQTT_Publish(&mqtt, AIO_USERNAME "/feeds/ Humidity_Statistics  ");

// Setup a feed called ' Temperature_Statistics  ' for subscribing to changes.
Adafruit_MQTT_Subscribe  Humidity_Statisticsbutton = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/ Humidity_Statistics  ");

// set up the 'dimmer' feed
Adafruit_MQTT_Subscribe dimmer = Adafruit_MQTT_Subscribe(&mqtt, AIO_USERNAME "/feeds/dimmer"); //


/*************************** Sketch Code ************************************/

// Bug workaround for Arduino 1.6.6, it seems to need a function declaration
// for some reason (only affects ESP8266, likely an arduino-builder bug).
void MQTT_connect();

void setup() {
  // start the serial connection
  Serial.begin(115200);
  delay(10);

  // Define the LED pin
  pinMode(LED_PIN, OUTPUT);

  Serial.println("Adafruit MQTT demo");

  // Connect to WiFi access point.
  Serial.println(); Serial.println();
  Serial.print("Connecting to ");
  Serial.println(WLAN_SSID);

  WiFi.begin(WLAN_SSID, WLAN_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }
  Serial.println();

  Serial.println("WiFi connected");
  Serial.println("IP address: "); 
  Serial.println(WiFi.localIP());

  // Setup MQTT subscription for  Temperature_Statistics  and Humidity_Statistics feeds.
  mqtt.subscribe(& Temperature_Statisticsbutton);
  mqtt.subscribe(& Humidity_Statisticsbutton);
  //Subscribe to the dimmer feed
  mqtt.subscribe(&dimmer);
}

// To get temperature data

// Initialize the data
float temp;
float hum;
 
void loop() {
  
  // Ensure the connection to the MQTT server is alive (this will make the first
  // connection and automatically reconnect when disconnected).  See the MQTT_connect
  // function definition further below.
  MQTT_connect();

  // this is our 'wait for incoming subscription packets' busy subloop
  // try to spend your time here

// To get the data from the sensor
  temp = dht.readTemperature();
  hum = dht.readHumidity();
  

  Adafruit_MQTT_Subscribe *subscriptionTemp;
  while ((subscriptionTemp = mqtt.readSubscription(5000))) {
    if (subscriptionTemp == & Temperature_Statisticsbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *) Temperature_Statisticsbutton.lastread);
    }
  }
  Adafruit_MQTT_Subscribe *subscriptionHum;
  while ((subscriptionHum = mqtt.readSubscription(5000))) {
    if (subscriptionHum == & Humidity_Statisticsbutton) {
      Serial.print(F("Got: "));
      Serial.println((char *) Humidity_Statisticsbutton.lastread);
    }
  }

  Adafruit_MQTT_Subscribe *subscriptionDimmer;
  Serial.println((char*) dimmer.lastread);
  int value = atoi((const char *)dimmer.lastread);  //converting the character received from adafruit to integer type
  Serial.println(0+(value*1023)/100);
  analogWrite(LED_PIN, (0+(value*1023)/100));  //analog write for PWM active low logic for the  built it LED
      
  // Dimmer code Start Here
  while ((subscriptionDimmer = mqtt.readSubscription(5000))) {
  if (subscriptionDimmer == &dimmer){
      //Print the new value to the serial monitor
      Serial.print("value: ");
      Serial.println((char*) dimmer.lastread);
      int value = atoi((const char *)dimmer.lastread);  //converting the character received from adafruit to integer type
      
      analogWrite(LED_PIN, (0+(value*1023)/100));  //analog write for PWM active low logic for the  built it LED
      /*when using an external led or any PWM actuator use the lower line 
  analogWrite(LED_PIN, (value/10)*102);*/
      /*highest value for pwm can be 1024 in our case, to divide it into 10 steps we use (1023-(value/10)*102)
  highest value 1023 is not reached in our code as we are not dividing by 102.4 precisely to avoid floating point operations*/
    }
  }
  // Dimmer code End Here
  

  // Now we can publish stuff!
  Serial.print(F("\nSending  Temperature_Statistics   val "));
  Serial.print(temp);
  Serial.print("...");
  if (!  Temperature_Statistics  .publish(temp)) {
    Serial.println(F("Failed"));
  } else {
    Serial.println(F("OK!"));
  }
  Humidity_Statistics  .publish(hum);

  // ping the server to keep the mqtt connection alive
  // NOT required if you are publishing once every KEEPALIVE seconds
  /*
  if(! mqtt.ping()) {
    mqtt.disconnect();
  }
  */
}

// Function to connect and reconnect as necessary to the MQTT server.
// Should be called in the loop function and it will take care if connecting.
void MQTT_connect() {
  // To print DHT temperature to serial monitor
  Serial.print("Temp is: " ); Serial.print(temp, 1); Serial.println(' C');
  // To print DHT humidity to serial monitor
  Serial.print("Humidity is: " ); Serial.print(hum, 1); Serial.println(' Hum');

  
  int8_t ret;

  // Stop if already connected.
  if (mqtt.connected()) {
    return;
  }

  Serial.print("Connecting to MQTT... ");

  uint8_t retries = 3;
  while ((ret = mqtt.connect()) != 0) { // connect will return 0 for connected
       Serial.println(mqtt.connectErrorString(ret));
       Serial.println("Retrying MQTT connection in 5 seconds...");
       mqtt.disconnect();
       delay(5000);  // wait 5 seconds
       retries--;
       if (retries == 0) {
         // basically die and wait for WDT to reset me
         while (1);
       }
  }
  Serial.println("MQTT Connected!");
}
