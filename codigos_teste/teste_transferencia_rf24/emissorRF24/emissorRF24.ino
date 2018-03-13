#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
 
//RF24(cePin, csnPi)
RF24 radio(7,8);
 

const uint64_t pipe = 0xE8E8F0F0E1LL;
 
int data = 0;
 
void setup(void)
{
  Serial.begin(57600);
  radio.begin();
  radio.openWritingPipe(pipe);
  pinMode(4, OUTPUT);
}

void loop(void)
{
      Serial.print("Sending:");
      Serial.print(data);
      digitalWrite(4,HIGH);

      bool ok;
      do{
        ok = radio.write(&data,sizeof(int));
        if(ok)
          Serial.println(".....successed");
        else
          Serial.println(".....failed");
      }while(!ok);
      
      digitalWrite(4,LOW);
      data++;
      delay(1000);
}


/*
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
  const char text[] = "J";
  radio.write(&text, sizeof(text));
  digitalWrite(4,LOW);
  delay(1000);
}
 */
