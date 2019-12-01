#include <Wire.h>

const int IN_D0 = 8; // digital input
static const uint32_t GPSBaud = 9600;
bool value_D0;
unsigned int cadencia[60];
unsigned int contador= 0;

void setup() {
  Serial.begin(9600);
  pinMode (IN_D0, INPUT);
  rellenaUnos();
}



void loop() {
  value_D0 = digitalRead(IN_D0);// reads the digital input from the IR distance sensor
  Serial.println(value_D0);
  
  if(contador == 59)//Resetea el contador para volver al principio del vector
    contador = 0;
    
  if(value_D0 == 0)
    cadencia[contador] = 0;
  else
    cadencia[contador] = 1;
    
  contador++;//Avanza el tiempo
  Serial.print("Cadencia -> ");
  Serial.println(calcularCadencia());
  delay(1000);
}

void rellenaUnos(){
  int i = 0;
  for(i = 0; i < 60; i++)
    cadencia[i] = 1;
}

int calcularCadencia(){
  int i;
  int final= 0;
  for(i=0; i < 60; i++){
    if(cadencia[i] == 0)
      final = final +2;
  }
  return final;
}
 /*
 * 1 No detecta nada
 * 0 Detecta movimiento
 */
