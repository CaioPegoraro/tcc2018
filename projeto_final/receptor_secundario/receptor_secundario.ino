//Bibliotecas globais
#include <Wire.h>
#include <LiquidCrystal_I2C.h>
#include <Servo.h>

#include <PID_v1.h>

//Bibliotecas locais
#include "Pacote.h"
#include "Led.h"
#include "MpuAngulo.h"

#include <Fuzzy.h>
#include <FuzzyComposition.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzyOutput.h>
#include <FuzzyRule.h>
#include <FuzzyRuleAntecedent.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzySet.h>

//Endereco da interface serial
#define RECEPTOR_SECUNDARIO_ADDR 0x60

int contador_cliente=0;
String escrita_serial;

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
int flag_controle = 0; //ativa o processo de controle de estabilizacao
      
//Leds de controle
Led LED_MOTOR(9);
Led LED_CONTROLE(11);
Led LED_MPU(8);

//LCD
LiquidCrystal_I2C lcd(0x27,16,2);
unsigned long pMillis = 0;
const long intervalo = 300; //ms
boolean escrever_idle=true;

/////////// VARIAVEIS DOS SISTEMAS DE CONTROLE ///////////////////
Angulo angulo;
double angulo_y,angulo_x;

double intensidade_esquerda, intensidade_direita, intensidade_atras, intensidade_frente;

//[1] -> PID: Controle de estabilizacao automatico//
float tempoGasto, time, tempoAnterior, tempo_decorrido_PID;
bool inicio_processo=true;
bool erro_mpu = false;

double AceleracaoPID=1300; 

double angulo_desejado, saida_PID;
double Kp=0.45,Ki=0.24,Kd=0.185;

PID PID_Emp(&angulo_y,&saida_PID,&angulo_desejado,Kp,Ki,Kd, DIRECT);

/// FIM VARIAVEIS PID EMPÍRICO//

//[2] -> PID: Ziegler-Nichols//
#include <PID_v1.h>
double Setpoint1, Output1;

//double kpZN_x=0.42, kiZN_x=0.583, kdZN_x=0.22;
double kpZN_y=0.36, kiZN_y=0.468, kdZN_y=0.192;

PID myPID(&angulo_x, &Output1, &Setpoint1, kpZN_y, kiZN_y, kdZN_y, DIRECT);

/// FIM VARIAVEIS PID Z-N//

//[3] -> FUZZY: Controle de estabilização nebuloso //

Fuzzy* fuzzy = new Fuzzy();
int flag_direcao; //indica a orientacao de giro do VANT
float AceleracaoFuzzy = 1300;
float tempo_decorrido_FUZZY;

/// FIM VARIAVEIS FUZZY ///

/////////// FIM DAS VARIAVEIS DOS SISTEMAS DE CONTROLE ///////////

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

void calibrar_motores(){
  motor1.write(65);
  motor2.write(65);
  motor3.write(65);
  motor4.write(65);
  delay(2);
 
  //Serial.println("calibrando");
  flag_calibrar = 0;  
}

void setup() {
  TesteLeds();
  Serial.begin(115200);

  //DEBUG GRAFICO
/*
  flag_controle = 1;
  flag_ok = 1;
  LED_MOTOR.setOn();
*/

  ///// SETUP VARIAVEIS DOS SISTEMAS DE CONTROLE ///////////////

  intensidade_esquerda=0;
  intensidade_direita=0;
  
  ///[1] -> PID EMPÍRICO ///
  time = millis();
  tempo_decorrido_PID=0;
  angulo_desejado=0;
  PID_Emp.SetMode(AUTOMATIC);
  PID_Emp.SetOutputLimits(-30,30);
  
  ///[2] -> FUZZY ///
  tempo_decorrido_FUZZY=0;
  
  //ENTRADA FUZZY
  FuzzyInput* AnguloFuzzyEntrada = new FuzzyInput(1);// Entrada fuzzy e como parametro seu ID

  //Conjunto de FuuzySet de entrada
  FuzzySet* anguloPositivo1 = new FuzzySet(0,0,0,5);
  AnguloFuzzyEntrada->addFuzzySet(anguloPositivo1);
  
  FuzzySet* anguloPositivo2 = new FuzzySet(0,5,5,10);
  AnguloFuzzyEntrada->addFuzzySet(anguloPositivo2);
  
  FuzzySet* anguloPositivo3 = new FuzzySet(5,10,10,15);
  AnguloFuzzyEntrada->addFuzzySet(anguloPositivo3);

  FuzzySet* anguloPositivo4 = new FuzzySet(10,15,15,20);
  AnguloFuzzyEntrada->addFuzzySet(anguloPositivo4);

  FuzzySet* anguloPositivo5 = new FuzzySet(15,30,30,30);
  AnguloFuzzyEntrada->addFuzzySet(anguloPositivo5);

  fuzzy->addFuzzyInput(AnguloFuzzyEntrada);

  //SAIDA FUZZY
  FuzzyOutput* VelocidadeFuzzySaida = new FuzzyOutput(1);
  
  //Conjunto FuzzySet de saida
  FuzzySet* velocidadePositivo1 = new FuzzySet(0,1,1,2);
  VelocidadeFuzzySaida->addFuzzySet(velocidadePositivo1);
  
  FuzzySet* velocidadePositivo2 = new FuzzySet(1,2,2,3);
  VelocidadeFuzzySaida->addFuzzySet(velocidadePositivo2);
  
  FuzzySet* velocidadePositivo3 = new FuzzySet(4,5,5,6);
  VelocidadeFuzzySaida->addFuzzySet(velocidadePositivo3);

  FuzzySet* velocidadePositivo4 = new FuzzySet(6,7,7,8);
  VelocidadeFuzzySaida->addFuzzySet(velocidadePositivo4);

  FuzzySet* velocidadePositivo5 = new FuzzySet(7,9,9,11);
  VelocidadeFuzzySaida->addFuzzySet(velocidadePositivo5);
  
  fuzzy->addFuzzyOutput(VelocidadeFuzzySaida);

  //REGRAS FUZZY
  //REGRA 1: FuzzyRule "SE angulo = anguloPositivo1 ENTAO velocidade = velocidadePositivo1"
  FuzzyRuleAntecedent* seAnguloPositivo1 = new FuzzyRuleAntecedent();
  seAnguloPositivo1->joinSingle(anguloPositivo1);
  FuzzyRuleConsequent* entaoVeloPositivo1 = new FuzzyRuleConsequent();
  entaoVeloPositivo1->addOutput(velocidadePositivo1);
  FuzzyRule* fuzzyRule01 = new FuzzyRule(1,seAnguloPositivo1,entaoVeloPositivo1);

  fuzzy->addFuzzyRule(fuzzyRule01);

  //REGRA 2: FuzzyRule "SE angulo = anguloPositivo2 ENTAO velocidade = velocidadePositivo2"
  FuzzyRuleAntecedent* seAnguloPositivo2 = new FuzzyRuleAntecedent();
  seAnguloPositivo2->joinSingle(anguloPositivo2);
  FuzzyRuleConsequent* entaoVeloPositivo2 = new FuzzyRuleConsequent();
  entaoVeloPositivo2->addOutput(velocidadePositivo2);
  FuzzyRule* fuzzyRule02 = new FuzzyRule(2,seAnguloPositivo2,entaoVeloPositivo2);

  fuzzy->addFuzzyRule(fuzzyRule02);

  //REGRA 3: FuzzyRule "SE angulo = anguloPositivo3 ENTAO velocidade = velocidadePositivo3"
  FuzzyRuleAntecedent* seAnguloPositivo3 = new FuzzyRuleAntecedent();
  seAnguloPositivo3->joinSingle(anguloPositivo3);
  FuzzyRuleConsequent* entaoVeloPositivo3 = new FuzzyRuleConsequent();
  entaoVeloPositivo3->addOutput(velocidadePositivo3);
  FuzzyRule* fuzzyRule03 = new FuzzyRule(3,seAnguloPositivo3,entaoVeloPositivo3);

  fuzzy->addFuzzyRule(fuzzyRule03);

  //REGRA 4: FuzzyRule "SE angulo = anguloPositivo4 ENTAO velocidade = velocidadePositivo4"
  FuzzyRuleAntecedent* seAnguloPositivo4 = new FuzzyRuleAntecedent();
  seAnguloPositivo4->joinSingle(anguloPositivo4);
  FuzzyRuleConsequent* entaoVeloPositivo4 = new FuzzyRuleConsequent();
  entaoVeloPositivo4->addOutput(velocidadePositivo4);
  FuzzyRule* fuzzyRule04 = new FuzzyRule(4,seAnguloPositivo4,entaoVeloPositivo4);

  fuzzy->addFuzzyRule(fuzzyRule04);

  //REGRA 5: FuzzyRule "SE angulo = anguloPositivo5 ENTAO velocidade = velocidadePositivo5"
  FuzzyRuleAntecedent* seAnguloPositivo5 = new FuzzyRuleAntecedent();
  seAnguloPositivo5->joinSingle(anguloPositivo5);
  FuzzyRuleConsequent* entaoVeloPositivo5 = new FuzzyRuleConsequent();
  entaoVeloPositivo5->addOutput(velocidadePositivo5);
  FuzzyRule* fuzzyRule05 = new FuzzyRule(5,seAnguloPositivo5,entaoVeloPositivo5);

  fuzzy->addFuzzyRule(fuzzyRule05);
  //////////////////////////////////////////////////////////////

  //LCD
  lcd.init(); //initialize the lcd
  lcd.backlight(); //open the backlight 
  lcd.setCursor(0,0);
  lcd.print("ANGULO: idle       ");
  lcd.setCursor(0,1);
  lcd.print("=VANT INICIADO=");
  
  ///MPU5060
  //Serial.println("INICIALIZANDO MPU");
  int error;
  uint8_t c;
  Wire.begin(RECEPTOR_SECUNDARIO_ADDR);
  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);
  LED_MPU.setOn();
  //Usar a funcao atualizaAngulos() para obter os valores de inclinacao nos eixos
  //////////////////////////////////////////////////////////////
  
  //Motores
  motor1.attach(7);
  motor2.attach(6);
  motor3.attach(4);
  motor4.attach(5);

  //Configuracao comunicacao IC2:
  //Wire.begin(RECEPTOR_SECUNDARIO_ADDR);
  Wire.onReceive(msgReceptorPrimario);
  Serial.setTimeout(50);

  //Variaveis de controle inicializacao:
  dados.valor = 0;

  //Calibracao dos motores
/* motor1.writeMicroseconds(1000);
  motor2.writeMicroseconds(1000);
  motor3.writeMicroseconds(1000);
  motor4.writeMicroseconds(1000);
  */
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

  motor1.writeMicroseconds(65);
  motor2.writeMicroseconds(65);
  motor3.writeMicroseconds(65);
  motor4.writeMicroseconds(65);
  delay(2);
  
  calibrar_motores();

  time = millis();
}

//Faz a leitura de um potenciometro na porta analógica A0 e define ao valor do angulo que seria lido no MPU
float simular_angulo(){
  float angulo = analogRead(0);
  angulo = map(angulo,0,1023,-30,30);
  //delay(10);
  return angulo;
}

//Encapsulamento das funcoes de leitura dos angulos
void atualizaAngulos(){
  angulo = calcular_angulo();
  angulo_y = angulo.y;
  angulo_x = angulo.x;
}

void valida_angulos(){
  if((angulo_y > 50) || (angulo_y <-50) || (angulo_x > 50) || (angulo_x < -50)){
      //erro na leitura
      erro_mpu = true;
    }
}

//Exibe o angulo de inclinacao no display LCD
void escreve_angulo_lcd(float angulo){
  lcd.setCursor(8,0);
  lcd.print("         "); //clear
  lcd.setCursor(8,0);
  lcd.print(angulo,3);
}

//Define a velocidade de operacao de um motor específico
void motor(int m, float velo){
  if(velo<=1400){
    switch (m){
      case 1:
        motor1.writeMicroseconds(velo);
        break;
      case 2:
        motor2.writeMicroseconds(velo-1);
        break;
      case 3:
        motor3.writeMicroseconds(velo+1);
         break;
      case 4:
        motor4.writeMicroseconds(velo+1);
        break;
    }
  }
}

void loop() {
  //Loop: realiza operacoes de calibracao nos motores (constitui uma das acoes criticas de seguranca para evitar casos em que
  //      os motores hajam de maneira inapropriada (operando em velocidade maxima sem controle por exemplo).

  //Antes de destravar os motores o loop fica executando a calibracao dos motores, quando se libera os motores essa operacao sera
  //executada apenas quando enviado um comando pelo painel de controle.
  //Ao liberar os motores a operacao padrao sera o controle de estabilidade realizado pelo PID utilizando os valores do acelerometro.
  //intercalado a essa operacao seram executados operacoes de controle manual (controlado pelo painel de controle).

   if (Serial.available() > 0) {
    byte buff[2];
    Serial.readBytes(buff, 2);

    //e armazena na variavel dados
    dados.cmd = (int)buff[0];
    dados.valor = (int)buff[1];

    int valor_data = dados.cmd*100 + dados.valor;
    
    if(valor_data==1000){
    //tratamento de comandos de ordem >=10 sem dados anexados
      dados.cmd = 10;
      //Serial.println(">> Sistema de estabilizacao ONLINE");
    }
    else if(valor_data==2000){
      dados.cmd = 20;
      //Serial.println(">> Sistema de estabilizacao OFFLINE");
    }

    //Outros casos de controle
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
      //habilita rotina de controle automatico
      flag_controle=1;
      LED_MPU.setOff();
    }
    else if(dados.cmd ==20){
      //desabilita rotina de controle automatico
      flag_controle=0;
      flag_calibrar = 1;
      inicio_processo=true;
     }
    //Serial.println(dados.cmd);
    //Serial.println(dados.valor);
      
   }

  //Operacao de estabilizacao do VANT

  //===========================================================
  //=========== SISTEMA DE CONTROLE UTILIZANDO FUZZY  =========
  //===========================================================

  //flag_controle=1;
  
  if(flag_controle==1){
    //Leitura do angulo de entrada
    atualizaAngulos();

    if((angulo_y > 50) || (angulo_y <-50)){
      //erro na leitura
      flag_controle=0;
      lcd.setCursor(0,0);
      lcd.print("ERRO DE LEITURA MPU");
      lcd.setCursor(1,0);
      lcd.print("Reinicie...");
    }
    else{
      if (angulo_y > 30)
        angulo_y = 30;
      if (angulo_y < -30)
        angulo_y = -30;
    
      if(angulo_y < 0){
        flag_direcao=2; //giro em um sentido de angulo negativo
        angulo_y = (-1)*angulo_y;
      }
      else{
        flag_direcao=1;
      }
      
      fuzzy->setInput(1,angulo_y);
  
      //Executar o calculo Fuzzy (Fuzificacao)
      fuzzy->fuzzify();
  
      //Recuperar dados de saida
      float FuzzyOutput = fuzzy->defuzzify(1);
  /*
      Serial.print("Input: ");
      Serial.print(angulo_y);
  
      Serial.print(" | Output: ");
      Serial.print(FuzzyOutput);
  */
      if(flag_direcao==1){ //y>0
        float correcao;
        intensidade_esquerda = AceleracaoFuzzy - FuzzyOutput;
        intensidade_direita = AceleracaoFuzzy + FuzzyOutput;
      }
      else{
        //flag_direcao=2; -> //y<0        
        intensidade_esquerda = AceleracaoFuzzy + FuzzyOutput;
        intensidade_direita = AceleracaoFuzzy - FuzzyOutput;
      }    
    
      //Validacao dos dados gerados antes de executar

      if(intensidade_esquerda>1400)
        intensidade_esquerda=1400;
    
      if(intensidade_direita>1400)
        intensidade_direita=1400;
    /*
      Serial.print(" | ESQ: ");
      Serial.print(intensidade_frente);
    
      Serial.print(" | DIR: ");
      Serial.println(intensidade_atras);
     */

     if(inicio_processo==true){
       inicio_processo=false;
       time = millis();
       tempo_decorrido_FUZZY=time;
     }
     else{
      time = millis();
     }

      intensidade_esquerda -=3;
      intensidade_direita +=0;
    
     //LOG

     if(flag_direcao==2){
      angulo_y = angulo_y*(-1);
     }
     
      escrita_serial = String(time-tempo_decorrido_FUZZY) + "#" + String(angulo_y) + "#" + String(intensidade_esquerda) + "#" + String(intensidade_direita);
      Serial.println(escrita_serial);

      //Serial.println(angulo_y);
      
      motor(1,intensidade_esquerda);
      motor(2,intensidade_direita);
      motor(3,intensidade_esquerda+1);
      motor(4,intensidade_direita+2);
      
    /*
      motor(3,intensidade_esquerda);
      motor(4,intensidade_direita);
     */
     
      //delay(100);
    }
  }

  //===========================================================
  //=========== SISTEMA DE CONTROLE UTILIZANDO PID ============
  //===========================================================

  if(flag_controle==2){
    LED_CONTROLE.setOn();
    //ALGORITMO DE CONTROLE 1: PID
    tempoAnterior = time;  //valor do instante te tempo anterior
    time = millis();  //leitura do instante de tempo atual
    tempoGasto = (time - tempoAnterior) / 1000; 

     if(inicio_processo==true){
       inicio_processo=false;
       tempo_decorrido_PID=time;
       tempoGasto=1;
     }
     atualizaAngulos();
    //Serial.println(angulo_x);
    
/* 
    Serial.println(angulo_y);
    Serial.println(time-tempo_decorrido_PID);
*/
    
/*
    Serial.print("(");
    Serial.print(angulo_y);
    Serial.print(";");
    Serial.print(time-tempo_decorrido_PID);
    Serial.println(")");
*/
    valida_angulos();

    if(erro_mpu==true){
      //erro na leitura do giroscopio/acelerometro
      flag_controle=0;
      lcd.setCursor(0,0);
      lcd.print("ERRO DE LEITURA");
      lcd.setCursor(1,0);
      lcd.print("Reinicie...");
    }
    else{

      LED_CONTROLE.setOn();
    
      atualizaAngulos();
      
      PID_Emp.Compute();
      
      if(saida_PID < -1000)
      {
        saida_PID=-1000;
      }
      if(saida_PID > 1000)
      {
        saida_PID=1000;
      }
      
      // INICIO eixo ---------- y

      intensidade_esquerda = AceleracaoPID + saida_PID;
      intensidade_direita = AceleracaoPID - saida_PID;

      //Direita
      if(intensidade_direita < 1000)
      {
        intensidade_direita= 1000;
      }
      if(intensidade_direita > 1400)
      {
        intensidade_direita=1400;
      }
      
      //Esquerda
      if(intensidade_esquerda < 1000)
      {
        intensidade_esquerda= 1000;
      }
      if(intensidade_esquerda > 1400)
      {
        intensidade_esquerda=1400;
      }

      //LOG
      escrita_serial = String(time-tempo_decorrido_PID) + "#" + String(angulo_y) + "#" + String(intensidade_esquerda) + "#" + String(intensidade_direita);
      Serial.println(escrita_serial);

      //Serial.println(angulo_y);

      if((angulo_y<10)&&(angulo_y>-10)){
        motor(1,intensidade_esquerda);
        motor(3,intensidade_esquerda+3);
        motor(2,intensidade_direita);
        motor(4,intensidade_direita+1);
      }
      else{
        motor(1,intensidade_esquerda);
        motor(3,intensidade_esquerda);
        motor(2,intensidade_direita);
        motor(4,intensidade_direita+2);
      }
      
      
      // FIM eixo ---------- y

      // INICIO eixo ---------- x
    /*
      intensidade_frente = AceleracaoPID + saida_PID;
      intensidade_atras = AceleracaoPID - saida_PID;

      //Frente
      if(intensidade_frente < 1000)
      {
        intensidade_frente= 1000;
      }
      if(intensidade_frente > 1400)
      {
        intensidade_frente=1400;
      }
      
      //Atras
      if(intensidade_atras < 1000)
      {
        intensidade_atras= 1000;
      }
      if(intensidade_atras > 1400)
      {
        intensidade_atras=1400;
      }

      //Serial.println(angulo_x);
      //Serial.println(contador_dados);
/*
      Serial.print(">> intensidade_frente: ");
      Serial.print(intensidade_frente);
      Serial.print(" | intensidade_atras: ");
      Serial.println(intensidade_atras);
*/      
      //LOG
   /*   
      escrita_serial = String((time-tempo_decorrido_PID)/10) + "#" + String(angulo_x) + "#" + String(intensidade_frente) + "#" + String(intensidade_atras);
      //Serial.println(escrita_serial);   
      
      //Serial.println(angulo_x);
      
      motor(1,intensidade_frente);
      motor(2,intensidade_frente);
      motor(3,intensidade_atras);
      motor(4,intensidade_atras);
*/
     // FIM eixo ---------- x
      
      LED_CONTROLE.setOff();      
    }
  }
  
  //flag de calibrar: indica a prioridade da operacao de calibacao (mantendo os motores na velocidade minima/desligados)
  //flag de ok: se for 0 indica que nao houve comandos que utilizam os motores do drone
  if (flag_calibrar == 1 || flag_ok == 0) {
    //calibrar motores
    //lcd.setCursor(0,0);
    //lcd.print("ANGULO: idle       ");
    atualizaAngulos();
    //Serial.println(angulo_y);
    escreve_angulo_lcd(angulo_y);
       
    delay(200);
    calibrar_motores();
    LED_CONTROLE.setOff();
    flag_calibrar=0;
  }
  
  /*
  else if(flag_ok==1)
    escreve_angulo_lcd(angulo_y);
  */
    
  //flag de operacao: indica execucao de algum comando que
  //utiliza algum dos motores
  //flag de ok: indica que os motores estao liberados por software
  if (flag_operacao == 1 && flag_ok == 1) {
    //nao precisa calibrar, entao vai processar a operacao dos motores
    flag_operacao = 0; //a flag eh valida para uma operacao

    if (dados.valor <= 200 && dados.valor >= 100) {
      //Operacao dos motores: valores entre 1000us ate 2000us

      dados.valor = dados.valor*10;
      
      switch (dados.cmd) {

        case 1:
          //Serial.println("acionando motor 1");
          motor1.writeMicroseconds(dados.valor);
          break;
        case 2:
          //Serial.println("acionando motor 2");
          motor2.writeMicroseconds(dados.valor);
          break;
        case 3:
          //Serial.println("acionando motor 3");
          motor3.writeMicroseconds(dados.valor);
          break;
        case 4:
          //Serial.print("acionando motor 4: ");
          //Serial.println(dados.valor);
          motor4.writeMicroseconds(dados.valor);
          break;
        case 6:
          //Serial.println("acionando todos os motores");
            motor1.writeMicroseconds(dados.valor);
            motor2.writeMicroseconds(dados.valor);
            motor3.writeMicroseconds(dados.valor);
            motor4.writeMicroseconds(dados.valor);
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
    //habilita rotina de controle automatico
    flag_controle=1;
  }
  else if(dados.cmd ==20){
    //desabilita rotina de controle automatico
    flag_controle=0;
    flag_calibrar = 1;
    inicio_processo=true;
   }
  //Serial.println(dados.cmd);
  //Serial.println(dados.valor);
}



