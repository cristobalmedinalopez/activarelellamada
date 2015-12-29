/*
 * Fecha: 10/07/2013
 * Autor: Cristóbal Medina López
 * Licencia:
 * Este Software está distribuido bajo la licencia general pública de GNU, GPL. Puedes encontrar esta licencia completa en http://www.gnu.org.
 * Es gratuito, puedes copiar y utlizar el código líbremente sin cargo alguno. Tienes derecho a modificar
 * el código fuente y a distribuirlo por tu cuenta, siempre informando de la procedencia original.
 *
 * Activar relé mediante llamada de teléfono. Versión 2013. Actualizado en 2015.
*/

//Hacemos uso de SoftwareSerial para usar dos pines cualquiera de arduino
#include <SoftwareSerial.h>
#include <TimerOne.h>

SoftwareSerial mySerial(5,6); //rx,tx
String inGSM; //Almacenar la informaci'on leida del modulo gsm
String numList[]={"600000000","612345678"}; //N'umeros autorizados

void setup()
{
  pinMode(9, OUTPUT); // rel'e
  pinMode(2, INPUT); // pulsador
  pinMode(10, OUTPUT); // igt modulo gsm tc35
  mySerial.begin(9600);
  Serial.begin(9600);
  Timer1.initialize(20000000); //20sg (max. 21.47)
  Timer1.attachInterrupt(ISR_alive); // Activa la interrupcion y la asocia a ISR_alive
  initGSM(); //iniciar el modulo gsm
}
 
void ISR_alive(){
     on();
     Serial.println("Comprobando red...");
     mySerial.print("AT+CREG?\r\n");
}

void on(){
   digitalWrite(10,HIGH); //igt 
   delay(100);
   digitalWrite(10,LOW); //igt  
}
 
void initGSM(){
  digitalWrite(9,HIGH); //desactivamos el rel'e (activo a nivel bajo)
  on();
  Serial.println("Iniciando...");
  mySerial.print("AT+CPIN=1234\r\n"); //Enviar PIN (1234)
  delay(5000);
  mySerial.print("AT+CLIP=1\r\n"); //Activar alarma llamada
  delay(500);  
  mySerial.print("AT+CMGF=1\r\n"); //Activar modo texto
  delay(500); 
}

//Funci'on: Comprueba si el n'umero est'a autorizado
//true si tiene permiso, false si no lo tiene.
boolean isAllow(String num){
  for (int i=0;i<sizeof(numList);i++){
     if (num==numList[i])
         return true;
  }
  return false;
}

// Funci'on: Accionar rel'e
void fire(){
   digitalWrite(9,LOW);
   delay(800); //tiempo que permanece activo el rel'e
   digitalWrite(9,HIGH);
}

// Funci'on: Enviar sms
void sendSMS(){
   mySerial.print("AT+CMGF=1\r\n"); //Activar modo texto
   delay(500); 
   mySerial.print("AT+CMGS=\"666554433\"\r\n"); //AT command to send SMS
   delay(1000);
   mySerial.print(num); //Print the message
   delay(1000);
   mySerial.print("\x1A"); //Sendt ascii SUB  
   delay(5000);
}

void loop() {
  inGSM="";

  //Leer enventos en modulo gsm
  while (mySerial.available()>0){
     delay(10);
     inGSM += (char)mySerial.read(); 
  }
  
  Serial.print(inGSM);

  //Si llamada
  if (inGSM.indexOf("RING")>-1){
      delay(180);
      mySerial.print("AT+CHUP\r\n"); //Rechazar llamada
     
      String num;
      int inicio=inGSM.indexOf('"')+1;
      int fin=inicio+9;
      
      num=inGSM.substring(inicio,fin); //Extraer n'umero    
  
      //Si autorizado, accionar rel'e    
      if (isAllow(num)){
        fire();
      }else{
        delay(800);
        sendSMS(num);
      }
  }
   
  //En caso de perder la cobertura, volver a conectarse a la red
  if (inGSM.indexOf("CREG:")>-1 && inGSM.indexOf("CREG: 0,1")==-1 && inGSM.indexOf("CREG: 0,5")==-1){
    Serial.println("Se ha perdido la cobertura, reiniciando...");
    mySerial.print("AT+CFUN=1,1\r\n");
    delay(4000);
    initGSM();
  }
}

