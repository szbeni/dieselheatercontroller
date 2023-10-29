
void turn_onoff(int onoff)
{
  if(switch_on_request != 0 || switch_off_request != 0 || knob_request != 0)
  {
    return;
  }
  
  if(onoff == 1)
  {
    switch_on_request = 1;
  }
  else if(onoff == 0)
  {
    switch_off_request = 1;
  }
}

void set_intensity(int set_intensity)
{
  if(switch_on_request != 0 || switch_off_request != 0 || knob_request != 0)
  {
    return;
  }
  if (get_diesel_heater_state() != DIESEL_HEATER_STATE_ON)
    return;
  
  if (set_intensity <= 6 && set_intensity > 0)
  {
    intensity_request = set_intensity;
  }
}


int get_diesel_heater_state()
{  

  if (!green_led_state && !red_led_state)
    return DIESEL_HEATER_STATE_OFF;
  else if (green_led_state && red_led_state == 0) 
    return DIESEL_HEATER_STATE_ON;
  else if (green_led_state  == 0 && red_led_state == 1)
    return DIESEL_HEATER_STATE_COOLING;
  else
    return DIESEL_HEATER_STATE_ON;  
}


void servo_loop()
{
  long now = millis();
  static long last_run = 0;
  static long last_quick_run = 0;
  static long heater_on_counter = 0;
  static float on_percent = 0;
  static float last_servo_angle = 0;

  if (now - last_quick_run  > 100)
  {
    //servo.write(mqtt_servo_angle); 
      if (vent_state == 1)
      {
        if(on_percent < 100)
        {
          on_percent += 0.5;
        }
      }
      else
      {
        if (on_percent > 0)
        {
          on_percent -= 0.5;
        }
      }

    float servo_angle = VENT_OFF_ANGLE + (float)(VENT_ON_ANGLE - VENT_OFF_ANGLE) * on_percent / 100.0;
    float diff = fabsf(last_servo_angle - servo_angle);
    if (diff > 0.4)
    {
      last_servo_angle = servo_angle;
      servo.write(servo_angle); 
      Serial.print("Servo angle: ");
      Serial.println(servo_angle);
    }
  
    last_quick_run = now;
  }
  
  if (now - last_run > 1000)
  {
    int state = get_diesel_heater_state();
    
    if (state == DIESEL_HEATER_STATE_ON)
    {
      if(heater_on_counter < vent_on_time_sec)
      {
        vent_state = 0;
        heater_on_counter++;
      }
      else
      {
        heater_on_counter = vent_on_time_sec;
        vent_state = 1;
      }
    }
    else
    {
      if (heater_on_counter > 0)
        heater_on_counter--;
    
      vent_state = 0;
    }

    last_run = now;
    
  }
//  static long last_servo_change = 0;
//  static float duty = 90;
//  if (Serial.available())
//  {
//    char c = Serial.read();
//    if (c == 'q')
//      duty -= 1;
//    else if(c == 'w')
//      duty += 1;
//    if (duty < 35) duty = 35;
//    if (duty > 170) duty = 170;
//    Serial.print("duty: ");
//    Serial.println(duty);
//    servo.write(duty);
//  }

}


void turn_onoff_request_handler()
{
  static int btn_on_pressed = 0;
  static long btn_on_pressed_time = 0;
  static int switch_on_retry_cntr = 0;
  
  static int btn_off_pressed = 0;
  static long btn_off_pressed_time = 0;
  static int switch_off_retry_cntr = 0;

  static int knob_state = 0;
  static long knob_state_time = 0;
  
  static int rot_a_state = 0;
  static int rot_b_state = 0;

  static long last_check_intensity_time = 0;
  long now = millis();

 
  if (intensity_request != 0) 
  {
    // Check if intensity matches desired
    if(knob_state == 0)
    {
      if(now - knob_state_time > 3000)
      {
        if (get_diesel_heater_state() != DIESEL_HEATER_STATE_ON)
        {
          knob_request = 0;
          intensity_request = 0;
        }
        else
        {
          knob_request = intensity_request - intensity;
          if (knob_request == 0)
          {
            intensity_request = 0;
          }
          else
          {
            knob_state = 1;
            knob_state_time = now;
          }
        }
      }
    }
    else if(knob_state == 1)
    {
      rot_a_state = digitalRead(PIN_ROTARY_A);
      rot_b_state = digitalRead(PIN_ROTARY_B);
      Serial.print("Inital state: ");
      Serial.print(rot_a_state);
      Serial.println(rot_b_state);

      if (rot_a_state == 1)
      {
        if (knob_request < 0)
        {
          digitalWrite(PIN_ROTARY_A, rot_a_state);
          digitalWrite(PIN_ROTARY_B, !rot_b_state);
        }
        else
        {
          digitalWrite(PIN_ROTARY_A, !rot_a_state);
          digitalWrite(PIN_ROTARY_B, rot_b_state);
        }      
      }
      else
      {
        if (knob_request < 0)
        {
          digitalWrite(PIN_ROTARY_A, !rot_a_state);
          digitalWrite(PIN_ROTARY_B, rot_b_state);
        }
        else
        {
          digitalWrite(PIN_ROTARY_A, rot_a_state);
          digitalWrite(PIN_ROTARY_B, !rot_b_state);
        }      
      }
      
      pinMode(PIN_ROTARY_A, OUTPUT);
      pinMode(PIN_ROTARY_B, OUTPUT);
      
      knob_state = 2;
      knob_state_time = now;
    }
    else if(knob_state == 2)
    {
      if(now - knob_state_time > 50)
      {
        digitalWrite(PIN_ROTARY_A, !rot_a_state);
        digitalWrite(PIN_ROTARY_B, !rot_b_state);
        knob_state = 3;
        knob_state_time = now;
        
      }
    }
    else if(knob_state == 3)
    {
      if(now - knob_state_time > 50)
      {
        if (rot_a_state == 1)
        {
          if (knob_request < 0)
          {
            digitalWrite(PIN_ROTARY_A, rot_a_state);
            digitalWrite(PIN_ROTARY_B, !rot_b_state);
          }
          else
          {
            digitalWrite(PIN_ROTARY_A, !rot_a_state);
            digitalWrite(PIN_ROTARY_B, rot_b_state);
          }      
        }
        else
        {
          if (knob_request < 0)
          {
            digitalWrite(PIN_ROTARY_A, !rot_a_state);
            digitalWrite(PIN_ROTARY_B, rot_b_state);
          }
          else
          {
            digitalWrite(PIN_ROTARY_A, rot_a_state);
            digitalWrite(PIN_ROTARY_B, !rot_b_state);
          }      

        }
        knob_state = 4;
        knob_state_time = now;
      }
    }
    else if(knob_state == 4)
    {
      if(now - knob_state_time > 50)
      {
        digitalWrite(PIN_ROTARY_A, rot_a_state);
        digitalWrite(PIN_ROTARY_B, rot_b_state);

        pinMode(PIN_ROTARY_A, INPUT);
        pinMode(PIN_ROTARY_B, INPUT);

        knob_state = 5;
        knob_state_time = now;
      }
    } 
    else if(knob_state == 5)
    {
      if(now - knob_state_time > 500)
      {
        
        if(knob_request < 0)
          knob_request++;
        else if(knob_request > 0)
          knob_request--; 

        if (knob_request != 0)
        {
          //Need to do more turns
          knob_state = 1;
          knob_state_time = now;      
        }
        else
        {
          // no more turns, check intensity reached
          knob_state = 0;
          knob_state_time = now;      
        }
      }
    }
  }
  
  if(switch_on_request)
  {     
    if (btn_on_pressed == 0)
    {
      if(green_led_state == 1)
      {
        Serial.println("Heater is already on.");
        switch_on_request = 0;
      }
      else
      {
        btn_on_pressed = 1;
        digitalWrite(PIN_BTN_ON, LOW);
        Serial.println("ON btn pressed");
        btn_on_pressed_time = millis();        
      }
    }
    else
    {
      if (millis() - btn_on_pressed_time > 400)
      {
        //Release button
        digitalWrite(PIN_BTN_ON, HIGH);
      }
      if (millis() - btn_on_pressed_time > 1500)
      {
        Serial.println("ON btn released");  
        Serial.print("greeb led state: ");   
        Serial.println(green_led_state);
        btn_on_pressed = 0;
        switch_on_request = 0;
        if (green_led_state == 0)
        {
          if(switch_on_retry_cntr > 3)
          {
            Serial.print("Cannot switch on heater after ");
            Serial.print(switch_on_retry_cntr);
            Serial.println(" tries. Giving up.");
            switch_on_retry_cntr = 0;
            switch_on_request = 0; 
          }
          else
          {
              switch_on_retry_cntr++;
              Serial.println("Heater not on.. try pressing agin");
              switch_on_request = 1;
          }
        }
        else
        {
          switch_on_retry_cntr = 0;
        }
      }
      
    }
  }

  if(switch_off_request)
  {    
    if (btn_off_pressed == 0)
    {
      if(green_led_state == 0)
      {
        Serial.println("Heater is already off.");
        switch_off_request = 0;
      }
      else
      {
        btn_off_pressed = 1;
        digitalWrite(PIN_BTN_OFF, LOW);
        Serial.println("OFF btn pressed");
        btn_off_pressed_time = millis();      
      }
    }
    else
    {
      if (millis() - btn_off_pressed_time > 400)
      {
        //Release button
        digitalWrite(PIN_BTN_OFF, HIGH);
      }
      if (millis() - btn_off_pressed_time > 1500)
      {
        Serial.println("OFF btn released");  
        Serial.print("green led state: ");   
        Serial.println(green_led_state);
        btn_off_pressed = 0;
        switch_off_request = 0;
        if (green_led_state == 1)
        {
          if(switch_off_retry_cntr > 3)
          {
            Serial.println("Cannot switch off heater after ");
            Serial.print(switch_off_retry_cntr);
            Serial.println(" tries. Giving up.");
            switch_off_retry_cntr = 0;
            switch_off_request = 0; 
          }
          else
          {
              switch_off_retry_cntr++;
              Serial.println("Heater is still on.. try pressing agin");
              switch_off_request = 1;
          }
        }
        else
        {
          switch_off_retry_cntr = 0;
        }
      }
      
    }
  }
}


void led_state_checker_loop()
{
  long now = millis();
  static int green_led = 0;
  static int green_led_prev = 0;
  static long last_change_green = 0;
  static int green_state = 0;
  static int green_state_prev = 0;

  static int red_led = 0;
  static int red_led_prev = 0;
  static long last_change_red = 0;
  static int red_state = 0;
  static int red_state_prev = 0;

  green_led = !digitalRead(PIN_LED_GREEN);
  red_led = !digitalRead(PIN_LED_RED);
  
  if(green_led != green_led_prev)
  {
    green_led_prev = green_led;
    last_change_green = now;
  }

  if(red_led != red_led_prev)
  {
    red_led_prev = red_led;
    last_change_red = now;
  }

  if (green_led == 0 && now - last_change_green > 300)
  {
    green_state = 0; 
  }
  if(green_led == 1 && now - last_change_green < 300)
  {
    green_state = 1;
  }
  if (green_state != green_state_prev)
  {
    green_state_prev = green_state;
    green_led_state = green_state;
    Serial.print("green: ");
    Serial.println(green_state);
  }
  
  if (red_led == 0 && now - last_change_red > 300)
  {
    red_state = 0;
  }
  if(red_led == 1 && now - last_change_red < 300)
  {
    red_state = 1;
  }
  if (red_state != red_state_prev)
  {
    red_state_prev = red_state;
    red_led_state = red_state;
    Serial.print("red: ");
    Serial.println(red_state);
  }
}


void knob_test_loop()
{
  static int rot_a = 0;
  static int rot_a_prev = 0;
  static int rot_b = 0;
  static int rot_b_prev = 0;
  static int rot_a_last_action = 0;
  static int rot_b_last_action = 0;
  rot_a = digitalRead(PIN_ROTARY_A);
  rot_b = digitalRead(PIN_ROTARY_B);
  if (rot_a != rot_a_prev) 
  {
    Serial.print("rota: ");
    Serial.print(rot_a);
    Serial.println(rot_b);
    rot_a_prev = rot_a;
    
    if (rot_a != rot_b) 
    {
      //Serial.println("up rota");
      rot_a_last_action = 1;
    }
    else
    {
      
      Serial.println("down rota");
      //switch_off_request = 1;
      rot_a_last_action = 0;
    }
  }
  if (rot_b != rot_b_prev) 
  {
    rot_b_prev = rot_b;
    Serial.print("rotb: ");
    Serial.print(rot_a);
    Serial.println(rot_b);

    
    if (rot_a != rot_b) 
    {
      //Serial.println("down rotb");
      rot_b_last_action = 0;
    }
    else
    {
      Serial.println("up rotb");
      //switch_on_request = 1;
      rot_b_last_action = 1;
    }
  }
}
