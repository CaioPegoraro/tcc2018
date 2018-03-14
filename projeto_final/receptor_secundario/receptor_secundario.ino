#include <Wire.h>
#include <Servo.h>

//Bibliotecas globais
#include <Wire.h>

//Bibliotecas locais
#include "Pacote.h"
#include "Led.h"
#include "MpuAngulo.h"

//Endereco da interface serial
#define RECEPTOR_SECUNDARIO_ADDR 0x60

//Pacote de dados para armazenamento da informacao
//recebida pela porta serial
pacote dados;
String data = "";

//Variaveis de controle dos motores
Servo motor1; //frente esquerda
Servo motor2; //frente direita
Servo motor3; //tras esquerda
Servo motor4; //tras direita

int flag_calibrar = 0; //solicita operacao de calibracao dos motores
int flag_operacao = 0; //solicita o tratamento de comando sobre os motores
int flag_ok = 0; //habilita operacao sobre os motores
int flag_controle =0; //ativa o processo de controle de estabilizacao

//Leds de controle
Led LED_MOTOR(9);
Led LED_CONTROLE(11);
Led LED_MPU(8);

/////////// SISTEMAS DE CONTROLE ///////////

//[1] -> PID: Controle de estabilizacao automatico//

//Velocidades: A velocidade de cada motor eh definida por dois fatores:
//I: O emissor envia um valor base para o motor, no qual podemos controlar a altura do drone, ja
//que eh possivel aumentar ou diminuir todas as intensidades.
//II: O valor calculado pelo PID, que define um valor de acrescimo ou decrescimo para cada motor
//afim de controlar a estabilidade

void TesteLeds(){
  //DESCRICAO: Funcao para testar os LEDS do dispositivo ao ligar.
  //USO: chamado na função de SETUP
  //RETORNO: nenhum
  //PARAMETROS: nenhum
  
  LED_MOTOR.setOn();
  delay(500);
  LED_CONTROLE.setOn();
  delay(500);
  LED_MPU.setOn();
  delay(500);
  LED_MOTOR.setOff();
  LED_CONTROLE.setOff();
  LED_MPU.setOff();
  delay(500);

  LED_MOTOR.setOn();
  delay(200);
  LED_CONTROLE.setOn();
  delay(200);
  LED_MPU.setOn();
  delay(200);
  LED_MOTOR.setOff();
  LED_CONTROLE.setOff();
  LED_MPU.setOff();
  delay(200);
  
  LED_MOTOR.setOn();
  delay(100);
  LED_CONTROLE.setOn();
  delay(100);
  LED_MPU.setOn();
  delay(100);
  LED_MOTOR.setOff();
  LED_CONTROLE.setOff();
  LED_MPU.setOff();
  delay(500);
}

void setup() {

  TesteLeds();

  Serial.begin(9600);
  
  //////////////////////////////////////////////////////////////
  ///MPU5060
  Serial.println("INICIALIZANDO MPU");
  int error;
  uint8_t c;
  Wire.begin(RECEPTOR_SECUNDARIO_ADDR);
  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
  LED_MPU.setOn();
  //Usar a funcao calcular_angulo() para obter os valores de inclinacao nos eixos
  //////////////////////////////////////////////////////////////
  
  //Motores
  motor1.attach(7);
  motor2.attach(6);
  motor3.attach(4);
  motor4.attach(5);

  //Configuracao comunicacao IC2:
  //Wire.begin(RECEPTOR_SECUNDARIO_ADDR);
  Wire.onReceive(msgReceptorPrimario);
  Serial.begin(9600);
  Serial.setTimeout(50);

  //Variaveis de controle inicializacao:
  dados.valor = 0;

  //Calibracao dos motores
  motor1.writeMicroseconds(2100);
  motor2.writeMicroseconds(2100);
  motor3.writeMicroseconds(2100);
  motor4.writeMicroseconds(2100);
  delay(3);

  motor1.writeMicroseconds(800);
  motor2.writeMicroseconds(800);
  motor3.writeMicroseconds(800);
  motor4.writeMicroseconds(800);
  delay(2);

  motor1.writeMicroseconds(1500);
  motor2.writeMicroseconds(1500);
  motor3.writeMicroseconds(1500);
  motor4.writeMicroseconds(1500);
  delay(2);

  motor1.writeMicroseconds(2100);
  motor2.writeMicroseconds(2100);
  motor3.writeMicroseconds(2100);
  motor4.writeMicroseconds(2100);
  delay(2);

  motor1.write(65);
  motor2.write(65);
  motor3.write(65);
  motor4.write(65);
  delay(2);
}

void loop() {

  //Loop: realiza operacÃƒÂµes de calibracao nos motores (constitui uma das acÃƒÂµes criticas de seguranca para evitar casos em que
  //      os motores hajam de maneira inapropriada (operando em velocidade maxima sem controle por exemplo).

  //Antes de destravar os motores o loop fica executando a calibracao dos motores, quando se libera os motores essa operacao sera
  //executada apenas quando enviado um comando pelo painel de controle.
  //Ao liberar os motores a operacao padrÃƒÂ£o sera o controle de estabilidade realizado pelo PID utilizando os valores do acelerÃƒÂ´metro.
  //intercalado a essa operacao seram executados operacÃƒÂµes de controle manual (controlado pelo painel de controle).

  //Operacao de estabilizacao do VANT
  if(flag_controle==1){
    LED_CONTROLE.setOn();
    calcular_angulo();
    LED_CONTROLE.setOff();
  }

  //flag de calibrar: indica a prioridade da operacao de calibacao (mantendo os motores na velocidade minima/desligados)
  //flag de ok: se for 0 indica que nao houve comandos que utilizam os motores do drone
  if (flag_calibrar == 1 || flag_ok == 0) {
    //calibrar motores

    //Serial.println("calibrando");
    flag_calibrar = 0;

    motor1.write(65);
    motor2.write(65);
    motor3.write(65);
    motor4.write(65);
    delay(2);

  }
  
  //flag de operacao: indica execucao de algum comando que
  //utiliza algum dos motores
  //flag de ok: indica que os motores estao liberados por software
  if (flag_operacao == 1 && flag_ok == 1) {
    //nao precisa calibrar, entao vai processar a operacao dos motores
    flag_operacao = 0; //a flag eh valida para uma operacao

    if (dados.valor < 180 && dados.valor > 60) {
      //faixa de operacao dos motores

      switch (dados.cmd) {

        case 1:
          //Serial.println("acionando motor 1");
          motor1.write(dados.valor);
          break;
        case 2:
          //Serial.println("acionando motor 2");
          motor2.write(dados.valor);
          break;
        case 3:
          //Serial.println("acionando motor 3");
          motor3.write(dados.valor);
          break;
        case 4:
          //Serial.println("acionando motor 4");
          motor4.write(dados.valor);
          break;

        case 6:
          //Serial.println("acionando todos os motores");
            motor1.write(dados.valor);
            motor2.write(dados.valor);
            motor3.write(dados.valor);
            motor4.write(dados.valor);
          //Serial.println(dados.valor);
          delay(15);
          break;
      }
    }
  }
}

//A funcao trata de eventos recebidos do mestre (pela hierarquia IC2)
//quando ha um envio por parte dele o escravo (esse receptor)
//desvia para tratamento do dado recebido, que consiste em
//ativar um flag (como se fosse uma senha de banco) para o loop
//executar uma operacao unicamente.
//e tb para salvar em uma variavel o valor do dado associado.
//a funcao foi previamente configurada no setup para esse tipo
//de tratamento.
void msgReceptorPrimario(int howMany) {
  LED_CONTROLE.setOn();
  data = "";
  while (Wire.available()) { //enqnto estiver disponivel bytes
    data += (char)Wire.read();
  }
  int valor_data = data.toInt();
  //Serial.println(valor_data);

  if(valor_data==1000){
    //tratamento de comandos de ordem >=10 sem dados anexados
    dados.cmd = 10;
  }
  else if(valor_data==2000){
    dados.cmd = 20;
  }
  else { //tratamento padrao, comando de acionamento
        //dos motores com intensidades determinadas
    dados.cmd = valor_data / 1000;
    dados.valor = valor_data - dados.cmd * 1000;
  }
  
  if (dados.cmd < 7) { //conjunto de comandos que vai
    //acionar um ou mais motores
    flag_operacao = 1;
  }
  else if (dados.cmd == 7) {
    flag_calibrar = 1;
  }
  else if (dados.cmd == 8) {
    //liberar flag para operar os motores
    flag_ok = 1;
    LED_MOTOR.setOn();
  }
  else if (dados.cmd == 9) {
    //travar flag para operar os motores
    flag_ok = 0;
    LED_MOTOR.setOff();
  }
  else if(dados.cmd ==10){
    flag_controle=1;
  }
  else if(dados.cmd ==20){
    flag_controle=0;
  }
  //Serial.println(dados.cmd);
  //Serial.println(dados.valor);
 LED_CONTROLE.setOff();
}


