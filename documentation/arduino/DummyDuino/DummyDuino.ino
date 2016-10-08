void setup() {
  Serial.begin(57600);
}

void loop() {
  int count = 0;
  while (1)
  {
    count ++;
    if (count > 150)
      count = 0;
    
    double temp;
    if (count < 50 || count > 100)
    {
      temp = 19.3;
    }
    else
    {
      temp = 20.25;
    }
    Serial.print(temp+1);
    Serial.print("; ");
    Serial.println((count/20)+10);
    delay (50); // Delay between each measurement
  }
}
