#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

RF24 radio(48, 49);

const byte rxAddr[6] = "00001";
const byte wxAddr[6] = "00002";

typedef struct{
  int valor;
} dados;
dados infos;

int cont=300;


void setup()
{
  while (!Serial);
  Serial.begin(9600);
  
  radio.begin();
  radio.openWritingPipe(wxAddr);
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();

  
}

void enviaMsg(){
  Serial.println("Enviando resposta");
  radio.stopListening();
  infos.valor=cont;
  cont++;

  //tentativas de enviar ao emissor original
  for(int i=0; i<10; i++){
    radio.write(&infos, sizeof(infos));
  }
  
  //delay(1);
  radio.startListening();
}

void loop()
{
  if(radio.available())
  {
    radio.read(&infos, sizeof(infos));

    Serial.print("recebido: ");
    Serial.println(infos.valor);

    //enviar resposta:
    //if(infos.valor==10){
      //enviaMsg();
    //}
    
  }
}
