#include <SPI.h>
#include "nRF24L01.h"
#include "RF24.h"
 
// RF24(cePin, csnPi)
RF24 radio(7,8);
 
const uint64_t pipe = 0xE8E8F0F0E1LL;
 

int recibeData;
 
void setup(void){
  Serial.begin(57600);
  radio.begin();
  radio.openReadingPipe(1,pipe);
  radio.startListening();
}
 
void loop(void){
 
  if (radio.available()){
    Serial.print("recibe:");

    //radio.read(const void *buf, uint8_t len)
    radio.read( &recibeData, sizeof(int));     
    Serial.println(recibeData);
  }
}

/*#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(7, 8);

const byte rxAddr[6] = "00001";

void setup()
{
  while (!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.openReadingPipe(0, 1);
  
  radio.startListening();
  Serial.println("Aguardando dados");
}

void loop()
{
  if (radio.available())
  {
    char text[32] = {0};
    radio.read(&text, sizeof(text));
    
    Serial.println(text);
  }
}*/
