int count = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(57600);
}

void loop() {
  // put your main code here, to run repeatedly:
  if(count==50){
    count=0;
  }
  else{
    count++;
  }

  Serial.println(count);
  delay(100);

}
