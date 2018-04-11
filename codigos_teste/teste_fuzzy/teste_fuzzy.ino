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
FuzzySet* baixoPositivo;

//1: OBJETO FUZZY
Fuzzy* fuzzy = new Fuzzy();
float angle;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
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
  
  baixoPositivo = new FuzzySet(0,10,10,20);
  angulo->addFuzzySet(baixoPositivo);
  
  FuzzySet* medioPositivo = new FuzzySet(10,20,20,30);
  angulo->addFuzzySet(medioPositivo);
  
  FuzzySet* altoPositivo = new FuzzySet(20,30,30,30);
  angulo->addFuzzySet(altoPositivo);

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

  FuzzySet* lentoPositivo = new FuzzySet(0,15,15,20);
  velocidade->addFuzzySet(lentoPositivo);
  
  FuzzySet* normalPositivo = new FuzzySet(20,40,60,80);
  velocidade->addFuzzySet(normalPositivo);
  
  FuzzySet* rapidoPositivo = new FuzzySet(60,70,90,100);
  velocidade->addFuzzySet(rapidoPositivo);

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
  //REGRA 5: FuzzyRule "SE angulo = baixoPositivo ENTAO velocidade = lentoPositivo"
  FuzzyRuleAntecedent* ifAngleLowPos = new FuzzyRuleAntecedent();
  ifAngleLowPos->joinSingle(baixoPositivo);
  FuzzyRuleConsequent* entaoVeloLentoPos = new FuzzyRuleConsequent();
  entaoVeloLentoPos->addOutput(lentoPositivo);
  FuzzyRule* fuzzyRule05 = new FuzzyRule(1,ifAngleLowPos,entaoVeloLentoPos);

  fuzzy->addFuzzyRule(fuzzyRule05);


  //REGRA 6: FuzzyRule "SE angulo = medioPositivo ENTAO velocidade = normalPositivo"
  FuzzyRuleAntecedent* ifAngleMediumPos = new FuzzyRuleAntecedent();
  ifAngleMediumPos->joinSingle(medioPositivo);
  FuzzyRuleConsequent* entaoVeloNormalPos = new FuzzyRuleConsequent();
  entaoVeloNormalPos->addOutput(normalPositivo);
  FuzzyRule* fuzzyRule06 = new FuzzyRule(2,ifAngleMediumPos,entaoVeloNormalPos);

  fuzzy->addFuzzyRule(fuzzyRule06);

  //REGRA 7: FuzzyRule "SE angulo = altoPositivo ENTAO velocidade = rapidoPositivo"
  FuzzyRuleAntecedent* ifAngleHighPos = new FuzzyRuleAntecedent();
  ifAngleHighPos->joinSingle(altoPositivo);
  FuzzyRuleConsequent* entaoVeloAltaPos = new FuzzyRuleConsequent();
  entaoVeloAltaPos->addOutput(rapidoPositivo);
  FuzzyRule* fuzzyRule07 = new FuzzyRule(3,ifAngleHighPos,entaoVeloAltaPos);

  fuzzy->addFuzzyRule(fuzzyRule07);


  for(int i=0;i<20; i++){
    angle = calcular_angulo();
  }

}

void loop() {
  // put your main code here, to run repeatedly:

  //5: ENTRADA DO FUZZY
  angle = calcular_angulo();

  if (angle > 30)
    angle = 30;
  if (angle < 0)
    angle = 0;
  
  fuzzy->setInput(1,angle);

  //6: EXECUTAR FUZZY
  fuzzy->fuzzify();

  //7: EXECUTAR DESFUZZY
  float output = fuzzy->defuzzify(1);


  Serial.print("Input: ");
  Serial.print(angle);

 
  Serial.print(" | Output: ");
  Serial.print(output);
 

  float pertinence = baixoPositivo->getPertinence();
  Serial.print(" | pertinencia: ");
  Serial.println(pertinence);
 
  delay(100);
}
