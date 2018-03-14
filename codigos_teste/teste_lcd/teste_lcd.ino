// Programa : Display LCD 16x2 e modulo I2C
// Autor : Arduino e Cia

#include <Wire.h>
#include <LiquidCrystal_I2C.h>

// Inicializa o display no endereco 0x27
//LiquidCrystal_I2C lcd(0x27,2,1,0,4,5,6,7,3, POSITIVE);
LiquidCrystal_I2C lcd(0x27,16,2);
 
void setup()
{
 lcd.begin (16,2);
}
 
void loop()
{
  lcd.setBacklight(HIGH);
  lcd.setCursor(0,0);
  lcd.print("VANT AUTO");
  lcd.setCursor(0,1);
  lcd.print("TCC UFSCAR");
  delay(1000);
  
}
