#include <TinyGPS++.h>
#include <SoftwareSerial.h>
#include <SD.h>
#include <SPI.h>

//static const int RXPin = 18, TXPin = 19;  //Para el sensor GPS
static const uint32_t GPSBaud = 57600;

TinyGPSPlus gps;                        //Para el GPS
//SoftwareSerial ss(RXPin, TXPin);

File myFile;                            //Para la tarjeta SD
double tSeconds = 0;
double latAnt;
double longAnt;
double tDistancia = 0;

//Para los botones
const int BOTON = 2;
int val = 0; //val se emplea para almacenar el estado del boton
int state = 0; // 0 LED apagado, mientras que 1 encendido
int old_val = 0; // almacena el antiguo valor de val

//Para el sensor de cadencia
const int IN_D0 = 8; // digital input
bool value_D0;
unsigned int cadencia[60];
unsigned int contador= 0;

void setup() {
  //pinMode(BOTON,INPUT); // y BOTON como señal de entrada
  Serial.begin(57600);
  Serial.println("The GPS recived signal");
  Serial1.begin(9600);       
  
  Serial.print("Conectando a SD...");     //Para tener preparada la SD para ejecutar
  if (!SD.begin(53)) {
    Serial.println("Conexion fallida");
    while (1);
  }
  Serial.println("Conexion correcta");
  pinMode (IN_D0, INPUT);
  rellenaUnos();
  
 
  headersXML();
 
}

void loop() {
  val= digitalRead(BOTON); // lee el estado del Boton
  if ((val == HIGH) && (old_val == LOW)){
    state=1-state;
    delay(10);
  }
  old_val = val; // valor del antiguo estado
  if (state==1){
    //Serial.println("MISUYY");
    //delay(10000);
    while (Serial1.available() > 0) {          //Bucle para ver que hay conexion con el GPS
      gps.encode(Serial1.read());              //Lee los datos del GPS
      if (gps.location.isUpdated()) {     //Comprueba que los datos se hayan actualizado
        cadenciaPorSegundo();
        trackpointXML();
       
        Serial.println("Iteraccion");
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
  fecha += ".00Z";
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
  myFile = SD.open("trackP.txt", FILE_WRITE);   //Abre el archivo donde va guardando la informacion
  if (myFile) {
    Serial.println("Imprime headers");
    myFile.println(xmlComent("----------------------------------------------------"));
    myFile.println(xmlComent("--   Hecho por Raul Capellan -> ciclocomputador   --"));
    myFile.println(xmlComent("----------------------------------------------------"));
    myFile.println(xmlheader());
    myFile.println(xmlInicio("TrainingCenterDatabase xmlns=\"http://www.garmin.com/xmlschemas/TrainingCenterDatabase/v2\""));
    myFile.println(xmlInicio("Activities"));
    myFile.println(xmlInicio("Activity Sport=\"Biking\""));
    myFile.println(xmlNodo("Id", formatoFecha()));
    myFile.println(xmlInicio("Lap StartTime="+formatoFecha()));
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
  myFile = SD.open("trackP.txt", FILE_WRITE);   //Abre el archivo donde va guardando la informacion
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
  myFile = SD.open("trackP.txt", FILE_WRITE);   //Abre el archivo donde va guardando la informacion
  if (myFile) {
    Serial.println("Imprime trackpoint");
        myFile.println(xmlInicio("Trackpoint"));
        myFile.print(xmlNodo("Time", formatoFecha()));
        myFile.print(xmlInicio("Position"));
        myFile.print(xmlNodoNum("LatitudeDegrees", gps.location.lat()));
        myFile.print(xmlNodoNum("LongitudeDegrees", gps.location.lng()));
        myFile.print(xmlCerrar("Position"));
        myFile.print(xmlNodoNum("AltitudeMeters", 0.00));
        
        if (tSeconds == 1) {
          latAnt = gps.location.lat();
          longAnt = gps.location.lng();
        }
          tDistancia += calcularDistancia(latAnt, gps.location.lat(), longAnt, gps.location.lng());
          myFile.print(tDistancia);
          latAnt = gps.location.lat();
          longAnt = gps.location.lng();
          
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
