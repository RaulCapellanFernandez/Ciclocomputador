#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>
#include <SFE_BMP180.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_PCD8544.h>

//Pantalla
Adafruit_PCD8544 display = Adafruit_PCD8544(7, 6, 5, 4, 3);
int opcion = 0;

//static const int RXPin = 18, TXPin = 19;  //Para el sensor GPS
static const uint32_t GPSBaud = 57600;

TinyGPSPlus gps;                        //Para el GPS
//SoftwareSerial ss(RXPin, TXPin);

File myFile;                            //Para la tarjeta SD
int tSeconds = 0;
double latAnt;
double longAnt;
double tDistancia = 0;
int altAnt = 0;

//Para los botones
const int BOTON = 2;
int val = 0; //val se emplea para almacenar el estado del boton
int state = 0; // 0 LED apagado, mientras que 1 encendido
int old_val = 0; // almacena el antiguo valor de val

//Para los botones
const int BOTON1 = 9;
int val1 = 0; //val se emplea para almacenar el estado del boton
int state1 = 0; // 0 LED apagado, mientras que 1 encendido
int old_val1 = 0; // almacena el antiguo valor de val
int contCloser = 0;

//Para el sensor de cadencia
const int IN_D0 = 8; // digital input
bool value_D0;
unsigned int cadencia[60];
unsigned int contador= 0;

/*
//Para la altitud con el barometro
SFE_BMP180 bmp180;
double PresionNivelMar=1013.25; //presion sobre el nivel del mar en mbar
*/
//Contador para el nombre y variable para su nombre
int contNombre = 0;
String nombreDoc;

//Para calcular el porcentaje de pendiente
double partialDist = 0;
int partialAlt = 0;
int pendiente = 0;

void setup() {
    
  //pinMode(BOTON,INPUT); // y BOTON como señal de entrada
  Serial.begin(57600);
  Serial.println("The GPS recived signal");
  Serial1.begin(9600);       

/*
  //Inicia el barometro
  if (bmp180.begin())
    Serial.println("BMP180 iniciado correctamenten");
  else
  {
    Serial.println("Error al iniciar el BMP180");
   while(1); // bucle infinito
  }*/

  //Para la pantalla
  display.begin();
  display.clearDisplay();
  display.println("");
  display.setTextSize(3);
  display.println("INIT");
  display.display();
  
  //Para la tarjeta SD
  Serial.print("Conectando a SD...");     //Para tener preparada la SD para ejecutar
  if (!SD.begin(53)) {
    Serial.println("Conexion fallida");
    while (1);
  }
  Serial.println("Conexion correcta");
  pinMode (IN_D0, INPUT);
  rellenaUnos();
  
}

void loop() {
  val= digitalRead(BOTON); // lee el estado del Boton
  if ((val == HIGH) && (old_val == LOW)){
    state=1-state;
    delay(10);
  }
  old_val = val; // valor del antiguo estado
  if (state==1){
    
    while (Serial1.available() > 0) {          //Bucle para ver que hay conexion con el GPS
      gps.encode(Serial1.read());              //Lee los datos del GPS
      if (gps.location.isUpdated()) {     //Comprueba que los datos se hayan actualizado
        
        //Cuando pulsa el boton se crea un .tcx con los headers y con el nombre la fecha y la hora
        if(contNombre == 0){
          darNombreDoc();
          Serial.println(nombreDoc);
          headersXML();
          delay(200);
          contNombre ++;
        }
        val1= digitalRead(BOTON1); // lee el estado del Boton
        if ((val1 == HIGH) && (old_val1 == LOW)){
          state1=1-state1;
          delay(10);
        }
        old_val1 = val1; // valor del antiguo estado
        if (state1==0){
          tSeconds++;
          if(tSeconds % 8 == 0){
            opcion ++;
            imprimePantalla(opcion);
            if(opcion == 5)
              opcion = 0;
          }
          Serial.print(porcentajePendiente());
          Serial.println("%");
          cadenciaPorSegundo();
          trackpointXML();
        }
         if (state1==1 && contCloser == 0){
          closerXML();
          //contCloser ++;
          state = 0;
          state1=0;
          contNombre = 0;
          
          display.begin();
          display.clearDisplay();
          display.println("");
          display.setTextSize(3);
          display.println("INIT");
          display.display();
          return loop();
          
        }
      }
    }
  }
  else{
    //Serial.println("Pausa");
  }
}

double calcularDistancia(double lat1, double lat2, double lon1, double lon2) {
  double rTierra = 6371;//KM
  double distancia;
  double dLat = (lat2 - lat1) * M_PI / 180; //Radianes
  double dLon = (lon2 - lon1) * M_PI / 180; //Radianes
  double sindLat = sin(dLat / 2);
  double sindLon = sin(dLon / 2);
  double rlat1 = lat1 * M_PI / 180; //Grados
  double rlat2 = lat2 * M_PI / 180; //Grados
  double va1 = pow(sindLat, 2) + pow(sindLon, 2) * cos(rlat1) * cos(rlat2);
  double va2 = 2 * atan2(sqrt(va1), sqrt(1 - va1));

  distancia = rTierra * va2;
  distancia = distancia * 1000;//Metros
  //Serial.println(distancia, 5);
  return distancia;
}

String formatoFecha() {
  //char fecha[23]="";
  String fecha;
  char aux[5];
  fecha = gps.date.year();
  fecha += "-";
  if (gps.date.month() >= 10)
    fecha += gps.date.month();
  else {
    fecha += "0";
    fecha += gps.date.month();
  }
  fecha += "-";
  if (gps.date.day() >= 10)
    fecha += gps.date.day();
  else {
    fecha += "0";
    fecha += gps.date.day();
  }
  fecha += "T";
  if (gps.time.hour() >= 10)
    fecha += gps.time.hour() + 1;
  else {
    fecha += "0";
    fecha += gps.time.hour() + 1;
  }
  fecha += ":";
  if (gps.time.minute() >= 10)
    fecha += gps.time.minute();
  else {
    fecha += "0";
    fecha += gps.time.minute();
  }
  fecha += ":";
  if (gps.time.second() >= 10)
    fecha += gps.time.second();
  else {
    fecha += "0";
    fecha += gps.time.second();
  }
  fecha += ".000Z";
  //Serial.println(fecha);
  return fecha;
}
///////////////Metodos para crear el XML////////////////////
String xmlInicio(String cadena){
  String result = '<'+cadena+'>';
  return result;
}
String xmlCerrar(String cadena){
  String result = "</"+cadena+'>';
  return result;
}
String xmlNodo(String titulo, String contenido){
  String result = xmlInicio(titulo)+contenido+xmlCerrar(titulo);
  return result;
}
String xmlNodoNum(String titulo, double contenido){
  String result = xmlInicio(titulo)+contenido+xmlCerrar(titulo);
  return result;
}
String xmlComent(String cadena){
  String result = "<!"+cadena+'>';
  return result;
}
String xmlheader(){
  return "<?xml version=\"1.0\" encoding=\"UTF-8\"?>";
}

///////////////////Headers TCX//////////////////
void headersXML(){
  myFile = SD.open(nombreDoc, FILE_WRITE);   //Abre el archivo donde va guardando la informacion
  if (myFile) {
    Serial.println("Imprime headers");
    //myFile.println(xmlComent("----------------------------------------------------"));
    //myFile.println(xmlComent("--   Hecho por Raul Capellan -> ciclocomputador   --"));
    //myFile.println(xmlComent("----------------------------------------------------"));
    myFile.println(xmlheader());
    myFile.println(xmlInicio("TrainingCenterDatabase xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\""));
    myFile.println(xmlInicio("Activities"));
    myFile.println(xmlInicio("Activity Sport=\"Biking\""));
    myFile.println(xmlNodo("Id", formatoFecha()));
    myFile.println(xmlInicio("Lap StartTime= \""+formatoFecha()+"\""));
    myFile.println(xmlNodoNum("TotalTimeSeconds",0.00));
    myFile.println(xmlNodoNum("DistanceMeters",0.00));
    myFile.println(xmlNodoNum("MaximumSpeed",0.00));
    myFile.println(xmlNodo("Intensity","Active"));
    myFile.println(xmlNodo("TriggerMethod", "Manual"));
    myFile.println(xmlInicio("Track"));
 
    myFile.close();
  } else
    Serial.println("error opening trackP.txt");

    myFile.close();
}

/////////////////////////Headers del cierre lo que se repite vamos//////////////////////////////////////////
void closerXML(){
  Serial.println("Closer escribiendo");
  myFile = SD.open(nombreDoc, FILE_WRITE);   //Abre el archivo donde va guardando la informacion
  if (myFile) {
    myFile.println(xmlCerrar("Track"));
    myFile.println(xmlInicio("Extensions"));
    myFile.println(xmlInicio("LX xmlns=\"http://www.garmin.com/xmlschemas/ActivityExtension/v2\""));
    myFile.println(xmlNodoNum("AvgSpeed", 0.00));
    myFile.println(xmlCerrar("LX"));
    myFile.println(xmlCerrar("Extensions"));
    myFile.println(xmlCerrar("Lap"));
    myFile.println(xmlInicio("Training xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\" VirtualPartner=\"false\""));
    myFile.println(xmlInicio("Plan Type=\"Workout\" IntervalWorkout=\"false\""));
    myFile.println(xmlNodo("Extensions",""));
    myFile.println(xmlCerrar("Plan"));
    myFile.println(xmlCerrar("Training"));
    myFile.println(xmlInicio("Creator xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"Device_t\""));
    myFile.println(xmlNodo("Name", "My GPS DIY"));
    myFile.println(xmlNodoNum("UnitId", 0));
    myFile.println(xmlNodoNum("ProductID", 31));
    myFile.println(xmlInicio("Version"));
    myFile.println(xmlNodoNum("VersionMajor",0));
    myFile.println(xmlNodoNum("VersionMinor",0));
    myFile.println(xmlCerrar("Version"));
    myFile.println(xmlCerrar("Creator"));
    myFile.println(xmlCerrar("Activity"));
    myFile.println(xmlCerrar("Activities"));
    myFile.println(xmlInicio("Author xmlns:xsi=\"http://www.w3.org/2001/XMLSchema-instance\" xsi:type=\"Application_t\""));
    myFile.println(xmlNodo("Name", "PPT 2008"));
    myFile.println(xmlInicio("Build"));
    myFile.println(xmlInicio("Version"));
    myFile.println(xmlNodoNum("VersionMajor",0));
    myFile.println(xmlNodoNum("VersionMinor",0));
    myFile.println(xmlCerrar("Version"));
    myFile.println(xmlCerrar("Build"));
    myFile.println(xmlNodo("LangID", "EN"));
    myFile.println(xmlNodo("PartNumber", "XXX-XXXXX-XX"));
    myFile.println(xmlCerrar("Author"));
    myFile.println(xmlCerrar("TrainingCenterDataBase"));
    
    myFile.close();
  } else
    Serial.println("error opening trackP.txt");
    myFile.close();
}

////////////////////Trackpoint Data////////////////////////////////////////
void trackpointXML(){
  myFile = SD.open(nombreDoc, FILE_WRITE);   //Abre el archivo donde va guardando la informacion
  if (myFile) {
    Serial.println("Imprime trackpoint");
        myFile.println(xmlInicio("Trackpoint"));
        myFile.print(xmlNodo("Time", formatoFecha()));
        myFile.print(xmlInicio("Position"));
        myFile.print(xmlNodoNum("LatitudeDegrees", gps.location.lat()));
        myFile.print(xmlNodoNum("LongitudeDegrees", gps.location.lng()));
        myFile.print(xmlCerrar("Position"));
        //myFile.print(xmlNodoNum("AltitudeMeters", calcularAltitud()));
        myFile.print(xmlNodoNum("AltitudeMeters", 0));
        
        if (tSeconds == 1) {
          latAnt = gps.location.lat();
          longAnt = gps.location.lng();
        }else{
          tDistancia += calcularDistancia(latAnt, gps.location.lat(), longAnt, gps.location.lng());
          partialDist += calcularDistancia(latAnt, gps.location.lat(), longAnt, gps.location.lng());
          //partialAlt = partialAlt + (calcularAltitud()-altAnt);
        }
          latAnt = gps.location.lat();
          longAnt = gps.location.lng();
          //altAnt = calcularAltitud();
          
          myFile.print(xmlNodoNum("DistanceMeters", tDistancia));
          myFile.print(xmlNodoNum("Cadence", calcularCadencia()));
          myFile.println(xmlCerrar("Trackpoint"));
    
    myFile.close();
  } else
    Serial.println("error opening trackP.txt");
    myFile.close();
}
//Rellena el vector para que este como vacio a a la hora de detectar la cadencia
void rellenaUnos(){
  int i = 0;
  for(i = 0; i < 60; i++)
    cadencia[i] = 1;
}

void cadenciaPorSegundo(){
  value_D0 = digitalRead(IN_D0);// lee el valor del sensor
  //Serial.println(value_D0);
  
  if(contador == 59)//Resetea el contador para volver al principio del vector
    contador = 0;
    
  if(value_D0 == 0)
    cadencia[contador] = 0;
  else
    cadencia[contador] = 1;
    
  contador++;//Avanza el tiempo
  //Serial.print("Cadencia -> ");
  //Serial.println(calcularCadencia());
}

//Calcula la cadencia
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
//Calcular la altitud con el barometro
int calcularAltitud(){
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
          //Serial.print("Altitud: ");
          //Serial.println(A);
        }      
      }      
    }   
  } 
  return A;
}*/

//Metodo para dar  nombre unico a cada track
void darNombreDoc(){
  //nombreDoc = gps.date.year();
  //nombreDoc += "-";
  nombreDoc = gps.date.month();
  //nombreDoc += "-";
  nombreDoc += gps.date.day();
  //nombreDoc += "-";
  nombreDoc += gps.time.hour()+1;
  //nombreDoc += "-";
  nombreDoc += gps.time.minute();
  //nombreDoc += "-";
  //nombreDoc += gps.time.second();
  nombreDoc += ".tcx";
}

//Calcular el porcentaje de pendiente cada 50m
int porcentajePendiente(){
 
  if(partialDist >= 50){
    pendiente = partialAlt * 100 / partialDist;
    partialDist = 0;
    partialAlt = 0;
  }
  return pendiente;
}


void imprimePantalla(int opcion1){

  display.begin();
  display.clearDisplay(); 
  
  switch(opcion1){
    case 1: 
     display.setTextSize(2);
     display.println(" Speed");
     display.setTextSize(3);
     display.println(gps.speed.kmph());
     display.setTextSize(1);
     display.println("    Km/h");
     display.display();
    break;
    
    case 2:
     display.setTextSize(2);
     display.println(" Distan");
     display.setTextSize(3);
     display.println( ((tDistancia/1000)));
     display.setTextSize(1);
     display.println("   meters");
     display.display();
    break;
    
    case 3: 
     display.setTextSize(2);
     display.println(" Caden");
     display.setTextSize(3);
     display.println(  calcularCadencia());
     display.setTextSize(1);
     display.println("  cad/min");
     display.display();
    break;
    
    case 4:
     display.setTextSize(2);
     display.println(" Alti");
     display.setTextSize(3);
     display.println(" 830");
     display.setTextSize(1);
     display.println("   meters");
     display.display();
    break;
    
    case 5: 
     display.setTextSize(2);
     display.println(" Pend");
     display.setTextSize(3);
     display.println("  23");
     display.setTextSize(1);
     display.println("    %");
     display.display();
    break;
  }
}
