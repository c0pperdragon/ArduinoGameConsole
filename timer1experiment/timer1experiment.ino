// running on an atmega328

byte phi1;
byte phi2;

byte level;
unsigned int boosttable[256]; // range from 0 to 1023

void setup() 
{
  
  pinMode(9,OUTPUT);   // timer1 A pulling low when active
  digitalWrite(9, HIGH);
  pinMode(10,OUTPUT);  // timer1 B pulling high when active
  digitalWrite(10, LOW);
  pinMode(2,OUTPUT);   // debug line - see how much time the interrupt takes

  int x;
  for (x=0; x<256; x++)
  {
//    boosttable[x] = 262911 / (512-x);   // too slow
//    boosttable[x] = 199485 / (450-x);   // still too slow
    boosttable[x] = 148335 / (400-x);     // too fast
    boosttable[x] = 179025 / (430-x);     // just right ;-)
  }

  
  noInterrupts();
  
  // -- TIMER 1 setup
  // set to mode 7 for 10-bit counter (fast PWM)
  // will count 0 to 1023 for a 64 us period
  TCCR1A = 
    B00000011    // WGM1:0=3
  | B10000000    // COM1A : set on bottom, clear at match
  | B00110000 ;  // COM1B : clear on bottom, set at match
  TCCR1B = 
    B00001000    // WGM3=1
  | B00000001 ;  // clock source = full speed

  // set compare register A to 512 for 50% duty cycle
  OCR1AH = 2;    // prepare high byte 
  OCR1AL = 0;    // write to 16-bit register  

  // set compare register B to 1023 to disable pulse
  OCR1BH = 3;    // prepare high byte 
  OCR1BL = 255;   // write to 16-bit register 

  TIMSK1 = B00000001;  // enable timer 1 overflow interrupt

  // start values of signal generation
  phi1 = 0;
  phi2 = 5;
  level = 128;

  
  // disable other timer interrupts
  TIMSK0 = 0;
  TIMSK2 = 0;
  
  interrupts();
}


void loop() 
{
  /*
  digitalWrite(9,LOW);
  delay(1);

  digitalWrite(9,HIGH);
  digitalWrite(10,HIGH);
  delay(10);
  digitalWrite(10,LOW);
  delay(1);
  */
}


ISR(TIMER1_OVF_vect)
{
    byte newlevel1 = 128;
    byte newlevel2 = 128;
    byte newlevel;

    PORTD = B11111011;  // for time measurement
    
    // increase waveform phase
    phi1 = (phi1+1) & 31;
    phi2 = (phi2+1) & 63;

//    // triangle wave
//    if (phi1<16) { newlevel1 = phi1*16; }
//    else { newlevel1 = 255 - (phi1-16)*16; }
    
//    // triangle wave
//    if (phi2<32) { newlevel2 = phi2*8; }
//    else { newlevel2 = 255 - (phi2-32)*8; }
    
//    // square wave
//    if (phi2<32) { newlevel2 =0; }
//    else { newlevel2 = 255; }
    
    // triangle wave
    newlevel2 = phi2*4;

    newlevel = (newlevel1>>1) + (newlevel2>>1);

    // need to boost up the level
    if (newlevel>level)
    {
        byte delta = newlevel - level;
        if (delta>64) {delta=64;}
        unsigned int boost = (delta * boosttable[level]) >> 6;
        level += delta;

        unsigned int val = 1023 - boost;
        OCR1AH = 3;             // idle pin A
        OCR1AL = 255;           
        OCR1BH = (byte)(val>>8);  // prepare high byte 
        OCR1BL = (byte)(val);     // write to 16-bit register 
    }
    // need to boost down the level
    else if (newlevel<level)
    {
        byte delta = level - newlevel;
        if (delta>64) {delta=64;}
        unsigned int boost = (delta * boosttable[255-level]) >> 6;
        level -= delta;

        unsigned int val = 1023 - boost;
        OCR1AH = (byte)(val>>8);  // prepare high byte 
        OCR1AL = (byte)(val);     // write to 16-bit register 
        OCR1BH = 3; 
        OCR1BL = 255;             // idle pin B        
    }
    // no change
    else
    {
        OCR1AH = 3;    // idle pin A
        OCR1AL = 255;    
        OCR1BH = 3; 
        OCR1BL = 255;  // idle pin B              
    }


    PORTD = B11111111;
}
