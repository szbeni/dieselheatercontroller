
long prev_interrupt_time = 0;
int prev_pin_state = 0;
long prev_duration = 0;

uint8_t bits[64];
int bits_cntr = 0;

//#define BIT_TYPE_ZERO 0
//#define BIT_TYPE_ONE 1
//#define BIT_TYPE_START 2
//#define BIT_TYPE_STOP 3

#define LEN_START_BIT 30 //ms

//Check if we are within tolerance
bool check_dur(long val, long duration)
{
  if (val < 0) return false;
  if (val < duration - 1)
    return false;
  if (val > duration + 1)
    return false;
  return true;
}


ICACHE_RAM_ATTR void comms_sniffer_interrupt() {
  long now = millis();
  int state = digitalRead(PIN_COMMS);
  
  long duration = now - prev_interrupt_time;

  //50 ms end of transaction
  if(duration > 50)
    bits_cntr = 0;
    
  if (bits_cntr > 63)
    bits_cntr = 0;
    
  //start bit
  if (state == 1 &&  check_dur(duration, 30))
  {
      //bits_cntr = 0;
  }
  //bit 1
  else if (state == 1 && check_dur(duration, 4) && check_dur(prev_duration, 8))
  {
      bits[bits_cntr] = 1;
      bits_cntr++;
  }
  //bit 0
  else if (state == 1 && check_dur(duration, 8) && check_dur(prev_duration, 4))
  {
      bits[bits_cntr] = 0;
      bits_cntr++;
  }
  //stop bit
  else if (state == 0 && duration < 2)
  {
    
  }
  
//  Serial.print("int ");
//  Serial.print(duration);
//  Serial.print("ms: ");
//  if (state == 1)
//    Serial.println("rising");
//  else
//    Serial.println("falling");

  
  prev_interrupt_time = now;
  prev_pin_state = state;
  prev_duration = duration;
}


void comms_sniffer_init()
{
    pinMode(PIN_COMMS, INPUT);
    attachInterrupt(digitalPinToInterrupt(PIN_COMMS), comms_sniffer_interrupt,  CHANGE);  
}


uint8_t bits_to_bytes(uint8_t *bits)
{
  uint8_t sum = 0;
  for(int i = 0; i < 8; i++)
  {
    sum |= (bits[7-i] << i);
  }
  return sum;
}

void comms_sniffer_loop()
{
  static long last_run = 0;
  char tmp[128];
  char data[8];

  long now = millis();
  if (now - last_run > 10)
  {
    
    //Transaction finished
    if (now - prev_interrupt_time > 40)
    {
      if(bits_cntr == 0)
      {
        // Nothing received
      }
      else
      {
//        Serial.print("Received: ");
//        for(int i=0;i<bits_cntr; i++)
//        {
//          if(i % 8 == 0)
//            Serial.print(" ");
//          if(bits[i])
//            Serial.print("1");
//          else
//            Serial.print("0");
//
//        }
//        Serial.println("");

        if (bits_cntr % 8 == 0 && bits_cntr <= 64)
        {
          
          Serial.print("Received: ");
          int len = bits_cntr / 8;
          for(int i=0; i<len ; i++)
          {
            data[i] = bits_to_bytes(&bits[i*8]);
            sprintf(tmp, "0x%X ", data[i]);
            Serial.print(tmp);
          }
          Serial.println("");


          if (len == 3 && data[2] & 0x40)
          {
            intensity = (data[2] & 0x07) + 1;
          }          
        }
        else
        {
          Serial.print("Error number of bits received: ");
          Serial.println(bits_cntr);
        }
        bits_cntr = 0;
      }

    }
    
    last_run = now;
  }
}
