#include<Adafruit_Sensor.h>
#include <ESP8266WiFi.h>
#include<PubSubClient.h>
#include "DHT.h"

// Uncomment one of the lines bellow for whatever DHT sensor type you're using!
#define DHTTYPE DHT11

// Initializes the espClient. You should change the espClient name if you have multiple ESPs running in your home automation system
WiFiClient g_client;
PubSubClient client(g_client);

// Change the credentials below, so your ESP8266 connects to your router
const char *ssid="REPLACE_WITH_YOUR_SSID";
const char *password="REPLACE_WITH_YOUR_PASSWORD";

// Change the variable to your Raspberry Pi IP address, so it connects to your MQTT broker
const char* mqtt_server = "iot.iandwe.in";

//WiFiServer server(80);
int LED_Pin=14;         // Light - LED - GPIO 14 = D5 on ESP-12E NodeMCU board
int LED_Pin2=12;        // Fan - LED - GPIO 12 = D6 on ESP-12E NodeMCU board
const int DHTPin = 5 ;  // DHT Sensor - GPIO 5 = D1 on ESP-12E NodeMCU board

//Initialize DHT sensor.
DHT dht(DHTPin, DHTTYPE);

//Timers auxiliar variables
long now = millis();
long lastMeasure = 0;

// Don't change the function below. This functions connects your ESP8266 to your router
void setup_wifi() {
  Serial.begin(115200);
  WiFi.begin(ssid,password);

  pinMode(LED_Pin,OUTPUT);
  digitalWrite(LED_Pin, LOW);

  pinMode(LED_Pin2,OUTPUT);
  digitalWrite(LED_Pin2, LOW);

 // We start by connecting to a WiFi network
  Serial.print("Connecting . . . . . . .");
  while (WiFi.status() !=WL_CONNECTED){
    delay(1500);
    Serial.print("*");
  }
  Serial.print("\nConnected to ");
  Serial.println(ssid);
  Serial.print("Local IP Address: ");
  Serial.print("http://");
  
  Serial.print(WiFi.localIP());
  Serial.println("/");
  Serial.println();

}

// This functions is executed when some device publishes a message to a topic that your ESP8266 is subscribed to
// Change the function below to add logic to your program, so when a device publishes a message to a topic that 
// your ESP8266 is subscribed you can actually do something

void callback(String topic, byte* message, unsigned int length) {
  Serial.print("Message arrived on topic: ");
  Serial.print(topic);
  Serial.print(". Message: ");
  String messageTemp;
  String messageTempfan;
  String messageTemp_all;
  String message_autofan;
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp += (char)message[i];
  }
  Serial.println();
  
  for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTempfan += (char)message[i];
  }
    Serial.println();

    for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    messageTemp_all += (char)message[i];
  }
    Serial.println();

    for (int i = 0; i < length; i++) {
    Serial.print((char)message[i]);
    message_autofan += (char)message[i];
  }
    Serial.println();

if (topic=="gourab_room/lamp")
  {
  Serial.print("Changing Gourab's Room Lamp to ");
    if(messageTempfan == "on"){
      digitalWrite(LED_Pin, HIGH);
      Serial.print("ON.");
    }
    else if(messageTemp == "off"){
      digitalWrite(LED_Pin, LOW);
      Serial.print("OFF.");
    }
  }
      Serial.println();
     if(topic=="gourab_room/fan"){
       Serial.print("Changing Gourab's Room Fan to ");
      if(messageTempfan = "on"){
        digitalWrite(LED_Pin2, HIGH);
        Serial.print("On");
      }
      else if(messageTempfan=="off"){
        digitalWrite(LED_Pin2, LOW);
        Serial.print("OFF");
      }
     }
      Serial.println();
      if(topic=="gourab_room/turn_on_both"){
        if(messageTemp_all=="on"){
            digitalWrite(LED_Pin,HIGH);
            digitalWrite(LED_Pin2,HIGH);
            Serial.print("Turning ON Everything...");
          }
          else if(messageTemp_all=="off"){
            digitalWrite(LED_Pin,LOW);
            digitalWrite(LED_Pin2,LOW);
            Serial.print("Turning OFF Everything...");
          }
      }
      Serial.println();
      if(topic=="gourab_room/automate_fan"){
        if(message_autofan=="on"){
          int t = dht.readTemperature();  
          if(t>28){
            digitalWrite(LED_Pin2,HIGH);
            Serial.println("As temperature is high, I switched on the fan.");
          }
          else if(message_autofan=="off"){
            digitalWrite(LED_Pin2,LOW);
            Serial.println("As temperature is low, I switched off the fan.");
          }
      }
      }
      Serial.println();
 }

// This functions reconnects your ESP8266 to your MQTT broker
// Change the function below if you want to subscribe to more topics with your ESP8266

void reconnect(){
  //Loop until we are reconnected...
  while(!client.connected()){
    Serial.print("Attempting MQTT connection..."); //Attempting to connect 
  
  if(client.connect("g_client")){
    Serial.println("Connected.");
    
      // Subscribe or resubscribe to a topic
      // You can subscribe to more topics (to control more LEDs in this example)
    client.subscribe("gourab_room/lamp");
    client.subscribe("gourab_room/fan");
    client.subscribe("gourab_room/turn_on_both");
    client.subscribe("gourab_room/automate_fan");
  }
  else{
     Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      delay(5000);
  }
}
}

// The setup function sets your ESP GPIOs to Outputs, starts the serial communication at a baud rate of 115200
// Sets your mqtt broker and sets the callback function
// The callback function is what receives messages and actually controls the LEDs

void setup()
{
  pinMode(LED_Pin, OUTPUT);
  pinMode(LED_Pin2,OUTPUT);
  dht.begin();
  
  Serial.begin(115200);
  setup_wifi();
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}

// For this project, you don't need to change anything in the loop function. Basically it ensures that you ESP is connected to your broker
void loop() {
  if (!client.connected()) {
    reconnect();
  }
  if(!client.loop())
    client.connect("ESP8266Client");

  now = millis();
  // Publishes new temperature and humidity every 2 seconds
  if (now - lastMeasure > 2000) {
    lastMeasure = now;
    // Sensor readings may also be up to 2 seconds 'old' (its a very slow sensor)
    float h = dht.readHumidity();
    // Read temperature as Celsius (the default)
    float t = dht.readTemperature();
    // Read temperature as Fahrenheit (isFahrenheit = true)
    float f = dht.readTemperature(true);

    // Check if any reads failed and exit early (to try again).
    if (isnan(h) || isnan(t) || isnan(f)) {
      Serial.println("Failed to read from DHT sensor!");
      return;
    }

    // Computes temperature values in Celsius
    float hic = dht.computeHeatIndex(t, h, false);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    //Uncomment to compute temperature values in Fahrenheit 
    float hif = dht.computeHeatIndex(f, h);
    static char temperatureTemp[7];
    dtostrf(hic, 6, 2, temperatureTemp);
    
    static char humidityTemp[7];
    dtostrf(h, 6, 2, humidityTemp);

    //Publishes Temperature and Humidity values
    client.publish("gourab_room/temperature", temperatureTemp);
    client.publish("gourab_room/humidity", humidityTemp);
    
    Serial.print("Humidity: ");
    Serial.print(h);
    Serial.print(" %\t Temperature: ");
    Serial.print(t);
    Serial.print(" *C ");
    Serial.print(f);
    Serial.print(" *F\t Heat index: ");
    Serial.print(hic);
    Serial.println(" *C ");
    Serial.print(hif);
     Serial.println(" *F");
  }
}
