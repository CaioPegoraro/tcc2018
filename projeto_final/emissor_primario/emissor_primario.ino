//Emissor primário
//Modelo: Arduino UNO
//Objetivo: Recebe um comando via USB do cliente (pc) e envia via wireless para o receptor primário no vant.

//bibliotecas globais
#include "Arduino.h"
#include <SPI.h>
#include <nRF24L01.h>
#include <RF24.h>

//bibliotecas locais
#include "pacote.h"
#include "led.h"

//inicialização do radio controle, caso fosse um arduino mega os pinos seriam: radio(48, 49);
//com os canais de leitura/escrita (no receptor os valores devem estar invertidos para a comunicação).
RF24 radio(7, 8);
const byte rxAddr[6] = "00001";
const byte wxAddr[6] = "00002";

//a estrutura para armazenar um comando recebido
pacote dados;

//comunicação serial
int done; //delimitador de leitura dos dados da porta serial.

//ldes de controle do emissor primário
Led LED_LIGADO(2);
Led LED_CONEXAO(3);
Led LED_COMUNICACAO(4);

void TesteLeds(){
  //DESCRICAO: Funcao para testar os LEDS do dispositivo ao ligar.
  //USO: chamado na função de SETUP
  //RETORNO: nenhum
  //PARAMETROS: nenhum
  
  LED_LIGADO.setOn();
  delay(1000);
  LED_CONEXAO.setOn();
  delay(1000);
  LED_COMUNICACAO.setOn();
  delay(1000);
  LED_LIGADO.setOff();
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();
  
  LED_LIGADO.setOn();
  delay(500);
  LED_CONEXAO.setOn();
  delay(500);
  LED_COMUNICACAO.setOn();
  delay(500);
  LED_LIGADO.setOff();
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();
  
  LED_LIGADO.setOn();
  delay(200);
  LED_CONEXAO.setOn();
  delay(200);
  LED_COMUNICACAO.setOn();
  delay(200);
  LED_LIGADO.setOff();
  LED_CONEXAO.setOff();
  LED_COMUNICACAO.setOff();

}

void setup() {
  //testes dos LEDS
  TesteLeds();

  //comunicação serial com timeout (para ajustar a velocidade de açãoo
  Serial.begin(9600);
  Serial.setTimeout(50);

  //inicialização da comunicação sem fio
  radio.begin();
  radio.setRetries(15, 15);
  radio.openWritingPipe(wxAddr);
  radio.openReadingPipe(0, rxAddr);
  radio.stopListening();

  //dispositivo ligado
  LED_LIGADO.setOn();
}

void loop() {
  //Descrição do loop: O emissor fica checando continuamente a porta serial na espera de um comando enviado pelo
  //                   operador C#, quando recebe um comando (inteiro, de 2 bytes) ele envia para o receptor
  //                   primário e também uma resposta para o cliente c# (para casos em que é necessário receber
  //                   algum valor do vant.

  
  //verifica se a porta serial possui dados para leitura
  if (Serial.available() > 0) {
    //Inicio: leitura do valor enviado pelo pc host via usb
    LED_COMUNICACAO.setOn();

    //realiza a leitura dos 2 bytes transferidos (padrao adotado)
    byte buff[2];
    Serial.readBytes(buff, 2);

    //e armazena na variavel dados
    dados.cmd = (int)buff[0];
    dados.valor = (int)buff[1];

    //Serial.print("Comando enviado: ");
    //Serial.println(dados.cmd);
    //Serial.print("Valor associado: ");
    //Serial.println(dados.valor);

    //envia os dados via conexão sem fio para o receptor primário
    radio.write(&dados, sizeof(dados));

    //Fim: leitura do valor enviado pelo pc via usb
    LED_COMUNICACAO.setOff();

    //avaliar tipo de comando: simples ou composto
    //simples: um comando de uma via, apenas é enviado e pronto
    //composto: é enviado mas precisa de uma resposta (como obter o nível da bateria atual)

    //por questões técnicas e de otimização foi considerado o intervalo númerico para
    //determinar se um comando é simples ou composto (uma faixa de valores para cada).
    //dessa forma não seria necessário adicionar mais um componente na estrutura da mensagem
    //permanecendo assim nos 2 bytes (ou no caso 1 inteiro).

    //se o valor for um comando composto, aguarda uma resposta do vant:
    if (dados.cmd >= 125) {
      //Serial.println("");
      //Serial.println("Aguardando resposta \n");
      radio.startListening();
      for (int i = 0; i < 500; i++) {
        done = radio.available();
        if (done) {
          break;
        }
        delay(4);
      }

      if (done) {
        //recebeu uma resposta do vant, agora precisa tratar de acordo com o comando original
        LED_COMUNICACAO.setOn();
        radio.read(&dados, sizeof(dados));

        switch (dados.cmd) {
          case 125: //estabelecer conexao
            //Serial.println("");
            //Serial.println("== Retorno ==");
            //Serial.println("Conexao estabelecida!, 125, 1");
            //Serial.println(125);
            //Serial.println(1);

            //escrevendo na saida serial o terminal de controle (painel c#) recebera os dados
            //enviados pelo vant.
            dados.valor = 1;
            Serial.println(dados.cmd * 100000 + dados.valor);
            LED_CONEXAO.setOn();
            break;

          case 126: //leitura valor da bateria
            Serial.println(dados.cmd * 100000 + dados.valor);
            break;
        }
        LED_COMUNICACAO.setOff();
      }
      else {
        //não foi recebido uma resposta durante a analise
        //Serial.println("");
        //Serial.println("== Retorno ==");
        //Serial.println("Nenhum dado recebido, verificar alcance!, 125, 0");

        dados.valor = 0;
        Serial.println(dados.cmd * 100000 + dados.valor);

        LED_CONEXAO.setOff(); //conexao deverá ser reestabelecida
      }
      radio.stopListening();
    }
  }
}
