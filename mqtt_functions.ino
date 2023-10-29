void mqttReconnect() {
  while (!mqttClient.connected()) {
    Serial.print("Attempting MQTT connection...");

    // Attempt to connect
    if (mqttClient.connect(MQTT_CLIENT_ID, MQTT_USER, MQTT_PASSWORD, MQTT_TOPIC_CONNECTION_STATE, 1, true, "disconnected", false)) {
      Serial.println("connected");

      // Once connected, publish an announcement...
      mqttClient.publish(MQTT_TOPIC_CONNECTION_STATE, "connected", true);
      mqttClient.subscribe(MQTT_TOPIC_COMMAND);
    } else {
      Serial.print("failed, rc=");
      Serial.print(mqttClient.state());
      Serial.println(" try again in 5 seconds");
      delay(5000);
    }
  }
}


void mqttPublish(char *topic, float payload) {
  Serial.print(topic);
  Serial.print(": ");
  Serial.println(payload);

  mqttClient.publish(topic, String(payload).c_str(), true);
}


void mqttOnMessage(char* topic, byte* payload, unsigned int length) {
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.print("] ");
  for (int i = 0; i < length; i++) {
    Serial.print((char)payload[i]);
  }
  Serial.println();


  if (strstr(topic, "servoangle"))
  {
    int val = atoi((const char*)payload);
    Serial.print("New servoa:");
    Serial.println(val);
    if (val > 0 and val < 600)
      mqtt_servo_angle = val;
  }
  if (strstr(topic, "ventontime"))
  {
    int val = atoi((const char*)payload);
    Serial.print("New vent on time:");
    Serial.println(val);
    if (val > 0 and val < 600)
      vent_on_time_sec = val;
  }
  if (strstr(topic, "turnonoff"))
  {
    static int first_message = 1;
    if (first_message)
    {
      first_message = 0;
      Serial.print("Ignore first onoff message, todo: why this happens?\n");
      return;
    }
    if ((char)payload[0] == '0') {  
      turn_onoff(0);
    }
    else if ((char)payload[0] == '1') {  
      turn_onoff(1);
    }
  }

  if (strstr(topic, "intensity"))
  {
    if ((char)payload[0] == '1') {  
      set_intensity(1);
    }
    else if ((char)payload[0] == '2') {  
      set_intensity(2);
    }
    else if ((char)payload[0] == '3') {  
      set_intensity(3);
    }
    else if ((char)payload[0] == '4') {  
      set_intensity(4);
    }
    else if ((char)payload[0] == '5') {  
      set_intensity(5);
    }
    else if ((char)payload[0] == '6') {  
      set_intensity(6);
    }

  }
}


void publishDieselHeaterState()
{
    lastDieselHeaterStateReportTime = millis();
    int state = get_diesel_heater_state();
    String msg = String(state);
    mqttClient.publish(MQTT_TOPIC_HEATER_STATE, msg.c_str(), true);
    Serial.print("Heater state: ");
    Serial.print(msg.c_str());
    
    msg = String(intensity);
    mqttClient.publish(MQTT_TOPIC_HEATER_INTENSITY, msg.c_str(), true);
    Serial.print(", intensity: ");
    Serial.println(msg.c_str());
}



void publishVentState()
{
    lastVentStateReportTime = millis();
    String msg = String(vent_state);
    Serial.print("Vent state: ");
    Serial.println(msg.c_str());
    mqttClient.publish(MQTT_TOPIC_VENT_STATE, msg.c_str(), true);
}
