#include <BH1750.h>
#include <Wire.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <Ticker.h>
#include <Adafruit_Sensor.h>
#include <DHT.h>
#include <DHT_U.h>

#define red_led_pin 13
#define green_led_pin 12
#define mqtt_led_pin 22

// Set up the DHT11 sensor
#define dht11_pin 15
#define dht11_type DHT11
#define SDA_PIN 33
#define SCL_PIN 25
#define ADDR 26
// Set up the BH1750 sensor
BH1750 lightMeter;
DHT dht(dht11_pin, dht11_type);
// Set up the MQTT client
WiFiClient espClient;
PubSubClient mqtt_client(espClient);

const char* ssid = "S22nyawahyu";
const char* password = "wahyubinus";
const char* mqtt_server = "broker.emqx.io";
const char* mqtt_topic = "espd";
const char* mqtt_subscribe_topic = "cobad";
const char* mqtt_publisher_topic = "sensor_data";
float get_temperature_and_humidity() {
  float humidity, temperature;
  humidity = temperature = NAN;
  humidity = dht.readHumidity();
  temperature = dht.readTemperature();
  return (isnan(humidity) || isnan(temperature)) ? NAN : humidity;
}
// define the functions for the LEDs and MQTT
void turn_on_red_led() {
  digitalWrite(red_led_pin, HIGH);
  digitalWrite(green_led_pin, LOW);
}
void turn_on_green_led() {
  digitalWrite(red_led_pin, LOW);
  digitalWrite(green_led_pin, HIGH);
}
void turn_on_mqtt_led() {
  digitalWrite(mqtt_led_pin, HIGH);
}
void turn_off_mqtt_led() {
  digitalWrite(mqtt_led_pin, LOW);
}
void connect_wifi() {
  WiFi.begin(ssid, password);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
  }
}
void connect_mqtt() {
  mqtt_client.setServer(mqtt_server, 1883);
  while (!mqtt_client.connected()) {
    String client_id = "esp32_" + String(random(0xffff), HEX);
    if (mqtt_client.connect(client_id.c_str())) {
      mqtt_client.subscribe(mqtt_topic);
    }
    delay(1000);
  }
}
void on_message(char* topic, byte* payload, unsigned int length) {
  Serial.println((char*)payload);
  turn_off_mqtt_led();
}
// main loop
void setup() {
  Serial.begin(9600);
  // set up the pins for the LEDs
  pinMode(red_led_pin, OUTPUT);
  pinMode(green_led_pin, OUTPUT);
  pinMode(mqtt_led_pin, OUTPUT);
  // connect to WiFi
  connect_wifi();
  // set up the DHT11 sensor
  dht.begin();
  // set up the MQTT client
  mqtt_client.setCallback(on_message);
  connect_mqtt();
}
void loop() {
  // get the sensor data
  float humidity = get_temperature_and_humidity();
  // check the light level and turn on the appropriate LED
  if (humidity < 400) {
    turn_on_red_led();
  } else {
    turn_on_green_led();
  }
  // publish the sensor data to the MQTT broker
  String payload = "Humidity: " + String(dht.readHumidity()) + "%";
  mqtt_client.publish(mqtt_publisher_topic, payload.c_str());
  turn_on_mqtt_led();
  // check if the MQTT client is connected
  if (!mqtt_client.connected()) {
    connect_mqtt();
  }
  // check if the WiFi is connected
  if (WiFi.status() != WL_CONNECTED) {
    connect_wifi();
  }
  // check if there are any messages from the MQTT broker
  mqtt_client.loop();
  delay(1000);
}