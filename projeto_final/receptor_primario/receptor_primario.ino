//Receptor primário
//Modelo: Arduino UNO
//Objetivo: primeiro receptor, está conectado ao receptor wireless que é capaz
//de receber e enviar comandos, tem uma rotina de checagem por instruções
//do emissor, em caso positivo, envia as mesmas via IC2 para o receptor secundário.

#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//bibliotecas locais
#include "Pacote.h"
#include "Led.h"

//canais de comunicação sem fio
//como ele pode alternar entre emissor
//e receptor temos que indicar o endereço
//para leitura/escrita
//no emissor primario os valores estao invertidos
//pois sempre um for emissor o outro eh o receptor
RF24 radio(7, 8);
const uint64_t rxAddr = 0xE8E8F0F0E1LL;
const uint64_t wxAddr = 0xA8E8F0F0E1LL;

//Configuração para comunicação IC2 com o arduino mega
#include <Wire.h>
#define RECEPTOR_SECUNDARIO_ADDR 0x60 //endereco do receptor primario

pacote dados;

byte buff[2];

//Variáveis de hardware
int pin_buzzer = 6;
int status_buzzer = 0;//inicialmente desligado
unsigned long previousMillis = 0;
const long interval = 700;

//Leds de controle
Led LED_CONEXAO(2);
Led LED_COMUNICACAO(4);

void TesteLeds(){
  //DESCRICAO: Funcao para testar os LEDS do dispositivo ao ligar.
  //USO: chamado na função de SETUP
  //RETORNO: nenhum
  //PARAMETROS: nenhum
  
  LED_CONEXAO.setOn();
  delay(500);
  LED_COMUNICACAO.setOn();
  delay(500);
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();
  delay(500);

  LED_CONEXAO.setOn();
  delay(200);
  LED_COMUNICACAO.setOn();
  delay(200);
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();
  delay(200);
  
  LED_CONEXAO.setOn();
  delay(100);
  LED_COMUNICACAO.setOn();
  delay(100);
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();
  delay(500);
}

void setup()
{
  //comunicação IC2 (entre os dispositivos do barramento serial)
  Wire.begin();

  //Serial
  //while (!Serial);
  Serial.begin(9600);

  //Wireless
  //inicia as configurações para funcionar como
  //um receptor
  radio.begin();
  //radio.setRetries(15, 15);
  //radio.setPayloadSize(8);
  radio.openWritingPipe(wxAddr);
  radio.openReadingPipe(0, rxAddr);
  radio.startListening();

  //Hardware
  pinMode(A0, INPUT);
  TesteLeds();
}

//Função para enviar uma resposta para o emissor primário
//eh feito a troca para função de "emissor", monta-se
//o pacote de dados e eh feito algumas tentativas de
//envio, dado que o emissor original também precisa
//de um tempo para trocar de contexto e tornar um receptor.

void enviaMsg(int cmd, int valor) {
  //Serial.println("Enviando resposta");
  radio.stopListening();
  dados.cmd = cmd;
  dados.valor = valor;
  //tentativas de enviar ao emissor original
  for (int i = 0; i < 20; i++) {
    LED_COMUNICACAO.setOn();
    radio.write(&dados, sizeof(dados));
    LED_COMUNICACAO.setOff();
  }
  //delay(1);
  radio.startListening();
}

void requestEvent() {
//funcao originalmente para tratar de requisicoes 
//do slave, mas nao foi utilizada.
};

void buzzer() {
  //Serial.println("toca o som dj!!");
  tone(pin_buzzer, 4000, 300); //freq = 4000hz
  //delay(500);
}


//Função para enviar uma mensagem para o arduino mega 
//(responsável pelo controle dos motores)
void enviaIC2() {
  Serial.println("enviando");
  //constroi-se o pacote de dados montando um inteiro
  //de 2 bytes, sendo cada byte formado pelo comando
  //e por um valor associado:
  int valor_pacote = dados.cmd * 1000 + dados.valor;
  //Serial.println(dados.cmd);
  //Serial.println(dados.valor);

  String pacote_ic2 = String(valor_pacote);
  Wire.beginTransmission(RECEPTOR_SECUNDARIO_ADDR);
  for (int x = 0; x < 4; x++) {
    Wire.write(pacote_ic2.charAt(x));
  }
  //Serial.println(pacote_ic2);
  Wire.endTransmission();
}

void loop()
{
  if (status_buzzer == 1) {
    unsigned long currentMillis = millis();
    if (currentMillis - previousMillis >= interval) {
      previousMillis = currentMillis;
      buzzer();
    }
  }

  //Descrição do loop: Mantém uma checagem contínua pelo sinal de wireless vindo do emissor primário,
  //                   ao receber um comando checa o tipo, caso seja simples executa a tarefa ou transmite
  //                   para o receptor secundário, caso seja composto, produz os dados necessários e
  //                   faz o envio de retorno ao emissor primário
  if (radio.available()){
    LED_COMUNICACAO.setOn();
    radio.read(&dados, sizeof(dados));
      
      //int tmp = dados.cmd;
      //dados.cmd = dados.valor;
      //dados.valor = tmp;
  
      //Serial.print("\n cmd: ");
      //Serial.println(dados.cmd);
      //Serial.print("\n valor: ");
      //Serial.println(dados.valor);
  
      //Serial.println("Rprimário recebido: " + dados.cmd);
      //Serial.println("valor: " + dados.valor);
      //Serial.println(dados.cmd);
  
      //avalia o tipo do comando
      if (dados.cmd <= 124) { //comandos simples
  
        switch (dados.cmd) {
            case 14: //Acionamento do buzzer
            status_buzzer = 1;
            break;
  
            case 15: //Desligamento do buzzer
            status_buzzer = 0;
            break;
  
            case 1: //Aciona motor1
            enviaIC2();
            break;
  
            case 2: //Aciona motor2
            enviaIC2();
            break;
  
            case 3: //Aciona motor3
            enviaIC2();
            break;
  
            case 4: //Aciona motor4
            enviaIC2();
            break;
  
          case 6: //Acionar todos os motores em uma velocidade
            enviaIC2();
            break;
  
          case 7: //Calibrar motores
            enviaIC2();
            break;
  
          case 8: //Liberar motores
            enviaIC2();
            break;
  
          case 9: //trava motores
            enviaIC2();
            break;
        }
      }
      else { // >=125 comandos compostos
  
        switch (dados.cmd) {
          case 125: //Conexao inicial
            LED_CONEXAO.setOn();
            enviaMsg(125, 0);
            break;
  
          case 126: //Ler carga da bateria
         /*   sensorValue = analogRead(A0);
            float voltage = sensorValue * (5.0 / 1023.0);
            voltage = voltage * 100; //ajustar casas decimais para emular um inteiro
            int volt_tmp = voltage;
            */
            enviaMsg(126, 0);
            break;
        }
      }
      /*
          if (dados.valor == 10) {
            enviaMsg(); //envia uma mensagem de volta ao controlador
          }
          else if (dados.valor == 15) {
            enviaIC2(); //envaminha o comando ao receptor secundário
          }
      */
    LED_COMUNICACAO.setOff();
  }
}
