#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte rxAddr[6] = "00001";

void setup()
{
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(1);
  radio.stopListening();

   pinMode(4, OUTPUT);
}

void loop()
{
  digitalWrite(4,HIGH);
  const char text[] = "Hello World";
  radio.write(&text, sizeof(text));
  digitalWrite(4,LOW);
  delay(1000);
}
