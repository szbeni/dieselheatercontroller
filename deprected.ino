//
//
//void sensor_loop() {
//  long now = millis();
//  // Sensor readings
//  if (now - lastSensorMsgTime > MQTT_PUBLISH_SENSOR_DELAY) {
//    lastSensorMsgTime = now;
//
//    // Reading BME280 sensor data
////    bme.takeForcedMeasurement(); // has no effect in normal mode
////    humidity = bme.readHumidity();
////    temperature = bme.readTemperature();
////    pressure = bme.readPressure() / 100.0F;
////    altitude = bme.readAltitude(SEALEVELPRESSURE_HPA);
//    
////    if (isnan(humidity) || isnan(temperature)|| isnan(pressure)) {
////      Serial.println("BME280 reading issues");
////      return;
////    }
//
//    humidity = 0;
//    temperature = 0;
//    pressure = 0;
//    altitude = 0;
//
//    // Publishing sensor data
//    mqttPublish(MQTT_TOPIC_TEMPERATURE, temperature);
//    mqttPublish(MQTT_TOPIC_HUMIDITY, humidity);
//    mqttPublish(MQTT_TOPIC_PRESSURE, pressure);
//    mqttPublish(MQTT_TOPIC_ALTITUDE, altitude);
//    Serial.println("Temp:");
//    Serial.println(temperature);
//  }
//}

//void heatingControlLoop() {
//  long now = millis();
//  
//  if (now - lastInputMsgTime > MQTT_PUBLISH_INPUT_DELAY) {
//    publishInputs();
//  }
//  
//  if (now - lastInputCheckTime > INPUT_CHECK_DELAY) {
//    lastInputCheckTime = now;
//  }
//  else {
//    return;
//  }
//
////  input[0] = digitalRead(INPUT_GAS_AUTO_FAN);
////  input[1] = digitalRead(INPUT_GAS_SLOW_FAN);
////  input[2] = digitalRead(INPUT_FAN);
////  input[3] = digitalRead(INPUT_ELEC_AUTO_FAN);
////  input[4] = digitalRead(INPUT_ELEC_SLOW_FAN);
////  
////  int changed = 0;
////  for(int i=0;i<INPUT_NUM;i++){
////    if (input_prev[i] != input[i]) 
////    {
////      input_prev[i] = input[i];
////      changed = 1;
////    }
////  }
////  if (changed == 1)
////  {
////    publishInputs();
////  }
//}
