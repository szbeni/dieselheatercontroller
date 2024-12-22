
#include <Wire.h>
#include <Adafruit_Sensor.h>
#include <Adafruit_BME280.h>
#include <ESP8266WiFi.h>
#include <PubSubClient.h>
#include <ArduinoOTA.h>
#include <Servo.h>

//#define MQTT_TOPIC_HUMIDITY "livingroom/dieselheater/sensor/humidity"
//#define MQTT_TOPIC_TEMPERATURE "livingroom/dieselheater/sensor/temperature"
//#define MQTT_TOPIC_PRESSURE "livingroom/dieselheater/sensor/pressure"
//#define MQTT_TOPIC_ALTITUDE "livingroom/dieselheater/sensor/altitude"
#define MQTT_TOPIC_HEATER_STATE  "livingroom/dieselheater/heaterstate"
#define MQTT_TOPIC_HEATER_INTENSITY  "livingroom/dieselheater/heaterintensity"
#define MQTT_TOPIC_VENT_STATE  "livingroom/dieselheater/ventstate"
#define MQTT_TOPIC_CONNECTION_STATE "livingroom/dieselheater/status"
#define MQTT_TOPIC_COMMAND "livingroom/dieselheater/cmd/#"
#define MQTT_CLIENT_ID "dieselheater"

//#define MQTT_PUBLISH_SENSOR_DELAY 60000
#define MQTT_PUBLISH_HEATER_STATE_DELAY 10000
#define MQTT_PUBLISH_VENT_STATE_DELAY 10000



#define SEALEVELPRESSURE_HPA (1013.25)
//#define BME280_ADDRESS 0x76

#define BUTTON_PRESS_DELAY  50

#define DIESEL_HEATER_STATE_OFF          0   //  Both led off
#define DIESEL_HEATER_STATE_ON           1   // Green led on
#define DIESEL_HEATER_STATE_COOLING      2   // Red led on
#define DIESEL_HEATER_STATE_ERROR        3   // Green led flashing 


#define PIN_COMMS        D7
#define PIN_LED_GREEN    D0
#define PIN_LED_RED      D1

#define PIN_BTN_ON         D2
#define PIN_BTN_OFF        D3

#define PIN_MOTOR_PWM      D4
#define PIN_MOTOR_PWM_NUM  2   // D4 is pin 2 in Servo.h
#define VENT_ON_ANGLE  170     // Servo angle for vent on
#define VENT_OFF_ANGLE 35      // Servo angle for vent off

#define PIN_ROTARY_A     D5
#define PIN_ROTARY_B     D6

#define INPUT_CHECK_DELAY 500
#define INPUT_NUM           4

const char *WIFI_SSID = "*****";
const char *WIFI_PASSWORD = "*****";

const char *MQTT_SERVER = "10.1.1.1";
const char *MQTT_USER = "*****";    // NULL for no authentication
const char *MQTT_PASSWORD = "*****"; // NULL for no authentication

float humidity;
float temperature;
float pressure;
float altitude;

int prev_diesel_heater_state = 0;
long lastDieselHeaterStateReportTime = 0;
long lastVentStateReportTime = 0;

// Turn on the vent afer 4 minutes of heater running time

int vent_on_time_sec = 240;
int vent_state = 0;
int switch_on_request = 0;
int switch_off_request = 1;
int intensity_request = 0;
int knob_request = 0;
int mqtt_servo_angle = 0;
int prev_mqtt_servo_angle = 0;

int intensity = 0;
int prev_intensity = 0;


int green_led_state = 0;
int red_led_state = 0;

int input[INPUT_NUM];
int input_prev[INPUT_NUM];

Servo servo;
Adafruit_BME280 bme;
WiFiClient espClient;
PubSubClient mqttClient(espClient);


void set_intensity(int intensity);
void turn_onoff(int onoff);

void setupWifi() {
  Serial.print("Connecting to ");
  Serial.println(WIFI_SSID);

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("WiFi connected");
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());
}

void setup() {   

  
  // Vent control
  pinMode(PIN_MOTOR_PWM,OUTPUT);
  servo.attach(PIN_MOTOR_PWM_NUM); //D4
  servo.write(VENT_OFF_ANGLE);

  //Diesel heater controller 
  pinMode(PIN_LED_GREEN, INPUT_PULLUP);
  pinMode(PIN_LED_RED, INPUT);

  pinMode(PIN_BTN_ON, OUTPUT);
  digitalWrite(PIN_BTN_ON, HIGH);
  
  pinMode(PIN_BTN_OFF, OUTPUT);
  digitalWrite(PIN_BTN_OFF, HIGH);

//  pinMode(PIN_BTN_ON, INPUT);
//  pinMode(PIN_BTN_OFF, INPUT);
  pinMode(PIN_ROTARY_A, INPUT);
  pinMode(PIN_ROTARY_B, INPUT);

  comms_sniffer_init();
  
  
  
  Serial.begin(115200);
  while (! Serial);
//
//  if (!bme.begin(BME280_ADDRESS)) {
//    Serial.println("Could not find a valid BME280 sensor, check wiring or BME-280 address!");
//    while (1);
//  }

  // Use force mode so that the sensor returns to sleep mode when the measurement is finished
//  bme.setSampling(Adafruit_BME280::MODE_FORCED,
//                  Adafruit_BME280::SAMPLING_X1, // temperature
//                  Adafruit_BME280::SAMPLING_X1, // pressure
//                  Adafruit_BME280::SAMPLING_X1, // humidity
//                  Adafruit_BME280::FILTER_OFF,  // filter off
//                  Adafruit_BME280::STANDBY_MS_1000); // only in normal mode
    

  setupWifi();
  mqttClient.setServer(MQTT_SERVER, 1883);
  mqttClient.setCallback(mqttOnMessage);
  ArduinoOTA.begin();
  
}


void loop() {
  // Check WiFi connection periodically
  while ( WiFi.status() != WL_CONNECTED) {
    Serial.print("Attempting to connect to WEP network, SSID: ");
    Serial.println(WIFI_SSID);
    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);

    // wait 10 seconds for connection:
    delay(10000);
  }
  
  if (!mqttClient.connected()) {
    mqttReconnect();
  }
  mqttClient.loop();
  turn_onoff_request_handler();
  servo_loop();
  //knob_test_loop();
  led_state_checker_loop();
  comms_sniffer_loop();

  
  int diesel_heater_state = get_diesel_heater_state();
  if(diesel_heater_state != prev_diesel_heater_state)
  {
    prev_diesel_heater_state = diesel_heater_state;
    publishDieselHeaterState();
  }

  if(diesel_heater_state != DIESEL_HEATER_STATE_ON)
  {
    intensity = 0;
  }
  if (intensity != prev_intensity)
  {
     prev_intensity = intensity;
     publishDieselHeaterState();
  }
  
  if (millis() - lastDieselHeaterStateReportTime > MQTT_PUBLISH_HEATER_STATE_DELAY)
  {
    publishDieselHeaterState();
  }
  if (millis() - lastVentStateReportTime > MQTT_PUBLISH_HEATER_STATE_DELAY)
  {
      publishVentState();
  }

  ArduinoOTA.handle();
}
