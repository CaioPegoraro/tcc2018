int led4=4;
int led5=2;
int buzzer=6;

void setup() {
  // put your setup code here, to run once:
  pinMode(led4, OUTPUT);
  pinMode(led5, OUTPUT);
  pinMode(buzzer, OUTPUT);
}

void loop() {
  // put your main code here, to run repeatedly:
  digitalWrite(led4, HIGH);
  digitalWrite(led5, HIGH);

  //tone(buzzer, 4000, 300);
  delay(6000);
  
}
