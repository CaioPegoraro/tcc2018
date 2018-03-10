class Led {
private:
 int porta; //porta digital correspondente
 boolean estado; //true:acesso , false:apagado
 
public:
 Led();
 Led(int p);
 void setEstado(boolean e);
 void setOn();
 void setOff();
};


Led::Led(){
 this->porta = 0;
}

Led::Led(int p){
  this->porta = p;
}

void Led::setEstado(boolean e){
  this->estado = e;
}

void Led::setOn(){
  setEstado(true);
  digitalWrite(this->porta,HIGH);
}

void Led::setOff(){
  setEstado(false);
  digitalWrite(this->porta,LOW);
}


