// running on an atmega328


void setup() {
  
  pinMode(9,OUTPUT);   // timer1 A
  pinMode(10,OUTPUT);  // timer1 B

  digitalWrite(9,LOW);
  digitalWrite(10,HIGH);
}


void loop() 
{
  int i;
  
  noInterrupts();
  for (i=0; i<100; i++) digitalWrite(10, LOW);
  
  digitalWrite(10, HIGH);
  interrupts();

  delay(100);  
}
