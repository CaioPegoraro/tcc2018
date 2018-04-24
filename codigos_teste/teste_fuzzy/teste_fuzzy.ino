#include <Fuzzy.h>
#include <FuzzyComposition.h>
#include <FuzzyInput.h>
#include <FuzzyIO.h>
#include <FuzzyOutput.h>
#include <FuzzyRule.h>
#include <FuzzyRuleAntecedent.h>
#include <FuzzyRuleConsequent.h>
#include <FuzzySet.h>

#include <Wire.h>
#include "MpuAngulo.h"
#include <Servo.h>

FuzzySet* anguloPositivo1;
FuzzySet* anguloPositivo2;

//1: OBJETO FUZZY
Fuzzy* fuzzy = new Fuzzy();
float angle;
int flag_direcao; //indica a orientacao de giro do VANT

//Variaveis de controle dos motores
Servo motor1; //frente esquerda
Servo motor2; //frente direita
Servo motor3; //tras esquerda
Servo motor4; //tras direita

float aceleracao, intensidade_esquerda, intensidade_direita;

void setup() {
  aceleracao = 1300;
  
  // put your setup code here, to run once:
  Serial.begin(9600);

   //Motores
  motor1.attach(7);
  motor2.attach(6);
  motor3.attach(4);
  motor4.attach(5);

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
  
  ///MPU5060
  Serial.println("INICIALIZANDO MPU");
  int error;
  uint8_t c;
  #define RECEPTOR_SECUNDARIO_ADDR 0x60
  Wire.begin(RECEPTOR_SECUNDARIO_ADDR);
  error = MPU6050_read(MPU6050_WHO_AM_I, &c, 1);
  MPU6050_write_reg (MPU6050_PWR_MGMT_1, 0);
  calibrate_sensors();  
  set_last_read_angle_data(millis(), 0, 0, 0, 0, 0, 0);

  //2: ENTRADA FUZZY
  FuzzyInput* angulo = new FuzzyInput(1);// Como parametro seu ID

  // Criando os FuzzySet que compoem o FuzzyInput angulo
/*
 FuzzySet* altoNegativo = new FuzzySet(-30,-30,-30,-15); //Angulo alto 
  angulo->addFuzzySet(altoNegativo);
  FuzzySet* medioNegativo = new FuzzySet(-20,-10,-10,-5);
  angulo->addFuzzySet(medioNegativo);
  FuzzySet* baixoNegativo = new FuzzySet(-5,0,0,0);
  angulo->addFuzzySet(baixoNegativo);
  FuzzySet* equilibrado = new FuzzySet(0,0,0,0);
  angulo->addFuzzySet(equilibrado);
*/
  
  anguloPositivo1 = new FuzzySet(0,0,0,5);
  angulo->addFuzzySet(anguloPositivo1);
  
  anguloPositivo2 = new FuzzySet(0,5,5,10);
  angulo->addFuzzySet(anguloPositivo2);
  
  FuzzySet* anguloPositivo3 = new FuzzySet(5,10,10,15);
  angulo->addFuzzySet(anguloPositivo3);

  FuzzySet* anguloPositivo4 = new FuzzySet(10,15,15,20);
  angulo->addFuzzySet(anguloPositivo4);

  FuzzySet* anguloPositivo5 = new FuzzySet(15,20,20,25);
  angulo->addFuzzySet(anguloPositivo5);

  FuzzySet* anguloPositivo6 = new FuzzySet(20,25,25,35);
  angulo->addFuzzySet(anguloPositivo6);

  fuzzy->addFuzzyInput(angulo);

  //3: SAIDA FUZZY
  FuzzyOutput* velocidade = new FuzzyOutput(1);

  // Criando os FuzzySet que compoem o FuzzyOutput velocidade
/*
  FuzzySet* lentoNegativo = new FuzzySet(-40,-20,-20,0);
  velocidade->addFuzzySet(lentoNegativo);
  FuzzySet* normalNegativo = new FuzzySet(-20,-40,-60,-80);
  velocidade->addFuzzySet(normalNegativo);
  FuzzySet* rapidoNegativo = new FuzzySet(-60,-80,-80,-100);
  velocidade->addFuzzySet(rapidoNegativo);
  FuzzySet* veloNula = new FuzzySet(0,0,0,0);
  velocidade->addFuzzySet(veloNula);
*/
  FuzzySet* velocidadePositivo1 = new FuzzySet(0,0,0,3);
  velocidade->addFuzzySet(velocidadePositivo1);
  
  FuzzySet* velocidadePositivo2 = new FuzzySet(3,3,3,5);
  velocidade->addFuzzySet(velocidadePositivo2);
  
  FuzzySet* velocidadePositivo3 = new FuzzySet(5,5,5,8);
  velocidade->addFuzzySet(velocidadePositivo3);

  FuzzySet* velocidadePositivo4 = new FuzzySet(8,8,8,10);
  velocidade->addFuzzySet(velocidadePositivo4);

  FuzzySet* velocidadePositivo5 = new FuzzySet(10,10,10,15);
  velocidade->addFuzzySet(velocidadePositivo5);

  FuzzySet* velocidadePositivo6 = new FuzzySet(15,15,15,20);
  velocidade->addFuzzySet(velocidadePositivo6);
  
  fuzzy->addFuzzyOutput(velocidade);

  //4: REGRAS FUZZY
 /*
  //REGRA 1: FuzzyRule "SE angulo = altoNegativo ENTAO velocidade = rapidoNegativo"
  FuzzyRuleAntecedent* ifAngleHighNeg = new FuzzyRuleAntecedent();
  ifAngleHighNeg->joinSingle(altoNegativo);
  FuzzyRuleConsequent* entaoVeloAltaNeg = new FuzzyRuleConsequent();
  entaoVeloAltaNeg->addOutput(rapidoNegativo);
  FuzzyRule* fuzzyRule01 = new FuzzyRule(1, ifAngleHighNeg, entaoVeloAltaNeg);

  fuzzy->addFuzzyRule(fuzzyRule01);

  //REGRA 2: FuzzyRule "SE angulo = medioNegativo ENTAO velocidade = normalNegativo"
  FuzzyRuleAntecedent* ifAngleMediumNeg = new FuzzyRuleAntecedent();
  ifAngleMediumNeg->joinSingle(medioNegativo);
  FuzzyRuleConsequent* entaoVeloNormalNeg = new FuzzyRuleConsequent();
  entaoVeloNormalNeg->addOutput(normalNegativo);
  FuzzyRule* fuzzyRule02 = new FuzzyRule(2, ifAngleMediumNeg, entaoVeloNormalNeg);

  fuzzy->addFuzzyRule(fuzzyRule02);

  //REGRA 3: FuzzyRule "SE angulo = baixoNegativo ENTAO velocidade = lentoNegativo"
  FuzzyRuleAntecedent*  ifAngleLowNeg = new FuzzyRuleAntecedent();
  ifAngleLowNeg->joinSingle(baixoNegativo);
  FuzzyRuleConsequent* entaoVeloLentoNeg = new FuzzyRuleConsequent();
  entaoVeloLentoNeg->addOutput(lentoNegativo);
  FuzzyRule* fuzzyRule03 = new FuzzyRule(3, ifAngleLowNeg, entaoVeloLentoNeg);

  fuzzy->addFuzzyRule(fuzzyRule03);

  //REGRA 4: FuzzyRule "SE angulo = equilibrado ENTAO velocidade = nulo"
  FuzzyRuleAntecedent* ifAngleNull = new FuzzyRuleAntecedent();
  ifAngleNull->joinSingle(equilibrado);
  FuzzyRuleConsequent* entaoVeloNula = new FuzzyRuleConsequent();
  entaoVeloNula->addOutput(veloNula);
  FuzzyRule* fuzzyRule04 = new FuzzyRule(4,ifAngleNull,entaoVeloNula);

  fuzzy->addFuzzyRule(fuzzyRule04);
*/
  //REGRA 5: FuzzyRule "SE angulo = anguloPositivo1 ENTAO velocidade = velocidadePositivo1"
  FuzzyRuleAntecedent* seAnguloPositivo1 = new FuzzyRuleAntecedent();
  seAnguloPositivo1->joinSingle(anguloPositivo1);
  FuzzyRuleConsequent* entaoVeloPositivo1 = new FuzzyRuleConsequent();
  entaoVeloPositivo1->addOutput(velocidadePositivo1);
  FuzzyRule* fuzzyRule05 = new FuzzyRule(1,seAnguloPositivo1,entaoVeloPositivo1);

  fuzzy->addFuzzyRule(fuzzyRule05);

  //REGRA 6: FuzzyRule "SE angulo = anguloPositivo2 ENTAO velocidade = velocidadePositivo2"
  FuzzyRuleAntecedent* seAnguloPositivo2 = new FuzzyRuleAntecedent();
  seAnguloPositivo2->joinSingle(anguloPositivo2);
  FuzzyRuleConsequent* entaoVeloPositivo2 = new FuzzyRuleConsequent();
  entaoVeloPositivo2->addOutput(velocidadePositivo2);
  FuzzyRule* fuzzyRule06 = new FuzzyRule(2,seAnguloPositivo2,entaoVeloPositivo2);

  fuzzy->addFuzzyRule(fuzzyRule06);

  //REGRA 7: FuzzyRule "SE angulo = anguloPositivo3 ENTAO velocidade = velocidadePositivo3"
  FuzzyRuleAntecedent* seAnguloPositivo3 = new FuzzyRuleAntecedent();
  seAnguloPositivo3->joinSingle(anguloPositivo3);
  FuzzyRuleConsequent* entaoVeloPositivo3 = new FuzzyRuleConsequent();
  entaoVeloPositivo3->addOutput(velocidadePositivo3);
  FuzzyRule* fuzzyRule07 = new FuzzyRule(3,seAnguloPositivo3,entaoVeloPositivo3);

  fuzzy->addFuzzyRule(fuzzyRule07);

  //REGRA 8: FuzzyRule "SE angulo = anguloPositivo4 ENTAO velocidade = velocidadePositivo4"
  FuzzyRuleAntecedent* seAnguloPositivo4 = new FuzzyRuleAntecedent();
  seAnguloPositivo4->joinSingle(anguloPositivo4);
  FuzzyRuleConsequent* entaoVeloPositivo4 = new FuzzyRuleConsequent();
  entaoVeloPositivo4->addOutput(velocidadePositivo4);
  FuzzyRule* fuzzyRule08 = new FuzzyRule(4,seAnguloPositivo4,entaoVeloPositivo4);

  fuzzy->addFuzzyRule(fuzzyRule08);

  //REGRA 9: FuzzyRule "SE angulo = anguloPositivo5 ENTAO velocidade = velocidadePositivo5"
  FuzzyRuleAntecedent* seAnguloPositivo5 = new FuzzyRuleAntecedent();
  seAnguloPositivo5->joinSingle(anguloPositivo5);
  FuzzyRuleConsequent* entaoVeloPositivo5 = new FuzzyRuleConsequent();
  entaoVeloPositivo5->addOutput(velocidadePositivo5);
  FuzzyRule* fuzzyRule09 = new FuzzyRule(5,seAnguloPositivo5,entaoVeloPositivo5);

  fuzzy->addFuzzyRule(fuzzyRule09);

  //REGRA 10: FuzzyRule "SE angulo = anguloPositivo6 ENTAO velocidade = velocidadePositivo6"
  FuzzyRuleAntecedent* seAnguloPositivo6 = new FuzzyRuleAntecedent();
  seAnguloPositivo6->joinSingle(anguloPositivo6);
  FuzzyRuleConsequent* entaoVeloPositivo6 = new FuzzyRuleConsequent();
  entaoVeloPositivo6->addOutput(velocidadePositivo6);
  FuzzyRule* fuzzyRule10 = new FuzzyRule(6,seAnguloPositivo6,entaoVeloPositivo6);

  fuzzy->addFuzzyRule(fuzzyRule10);
  

  for(int i=0;i<20; i++){
    angle = calcular_angulo();
    Serial.print(angle);
    motor1.write(65);
    motor2.write(65);
    motor3.write(65);
    motor4.write(65);
    delay(2);
  }

}

void motor(int m, float velo){
  if(velo<=1400){
    switch (m){
      case 1:
        motor1.writeMicroseconds(velo);
        break;
      case 2:
        motor2.writeMicroseconds(velo);
        break;
      case 3:
        motor3.writeMicroseconds(velo+7);
        break;
      case 4:
        motor4.writeMicroseconds(velo+2);
        break;
    }
  }
}

double simular_angulo(){
  float angulo = analogRead(0);
  angulo = map(angulo,0,1023,-33,33);
  delay(10);
  return angulo;
}

void loop() {
  // put your main code here, to run repeatedly:

  //5: ENTRADA DO FUZZY
  //angle = simular_angulo();
  angle = calcular_angulo();

  if (angle > 30)
    angle = 30;
  if (angle < -30)
    angle = -30;

  if(angle < 0){
    flag_direcao=2; //giro em um sentido de angulo negativo
    angle = (-1)*angle;
  }
  else{
    flag_direcao=1;
  }
  
  fuzzy->setInput(1,angle);

  //6: EXECUTAR FUZZY
  fuzzy->fuzzify();

  //7: EXECUTAR DESFUZZY
  float output = fuzzy->defuzzify(1);

  Serial.print("Input: ");
  Serial.print(angle);

 
  Serial.print(" | Output: ");
  Serial.print(output);
 
/*
  float pertinence = anguloPositivo2->getPertinence();
  Serial.print(" | pertinencia: ");
  Serial.println(pertinence);
*/

  if(flag_direcao==1){
    intensidade_esquerda = aceleracao - output;
    intensidade_direita = aceleracao + output;
  }
  else{
    //flag_direcao=2;
    intensidade_esquerda = aceleracao + output;
    intensidade_direita = aceleracao - output;
  }

  //Validacao dos dados gerados antes de executar
  if(intensidade_esquerda<1300)
    intensidade_esquerda=1300;
  if(intensidade_esquerda>1350)
    intensidade_esquerda=1350;

  if(intensidade_direita<1300)
    intensidade_direita=1300;
  if(intensidade_direita>1350)
    intensidade_direita=1350;

  Serial.print(" | ESQ: ");
  Serial.print(intensidade_esquerda);

  Serial.print(" | DIR: ");
  Serial.println(intensidade_direita);

/*
  motor(1,intensidade_esquerda);
  motor(3,intensidade_esquerda);
  motor(2,intensidade_direita);
  motor(4,intensidade_direita);
 */
  
  motor(3,intensidade_esquerda);
  motor(4,intensidade_direita);
  
  delay(100);
}
