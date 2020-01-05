#include <SFE_BMP180.h>
#include <Wire.h>

SFE_BMP180 bmp180;
double PresionNivelMar=1013.25; //presion sobre el nivel del mar en mbar

void setup()
{
  Serial.begin(9600);

  if (bmp180.begin())
    Serial.println("BMP180 iniciado correctamenten");
  else
  {
    Serial.println("Error al iniciar el BMP180");
    while(1); // bucle infinito
  }
}

void loop()
{
  calcularAltitud();
}

void calcularAltitud(){
  char status;
  double T,P,A;
  
  status = bmp180.startTemperature();//Inicio de lectura de temperatura
  if (status != 0)
  {   
    delay(status); //Pausa para que finalice la lectura
    status = bmp180.getTemperature(T); //Obtener la temperatura
    if (status != 0)
    {
      status = bmp180.startPressure(3);//Inicio lectura de presión
      if (status != 0)
      {        
        delay(status);//Pausa para que finalice la lectura        
        status = bmp180.getPressure(P,T);//Obtenemos la presión
        if (status != 0)
        {                  
          //-------Calculamos la altitud--------
          A= bmp180.altitude(P,PresionNivelMar);
          //Medir al nivel de calle para hacer calculos a nivel de calle son 815
          A = A+129;
          Serial.print("Altitud: ");
          Serial.println(A);
        }      
      }      
    }   
  } 
}
