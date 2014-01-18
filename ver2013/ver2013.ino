/*
 * Fecha: 10/07/2013
 * Autor: Cristóbal Medina López
 * Licencia:
 * Este Software está distribuido bajo la licencia general pública de GNU, GPL. Puedes encontrar esta licencia completa en http://www.gnu.org.
 * Es gratuito, puedes copiar y utlizar el código líbremente sin cargo alguno. Tienes derecho a modificar
 * el código fuente y a distribuirlo por tu cuenta, siempre informando de la procedencia original.
 *
 * Activar relé mediante llamada de teléfono. Versión 2013.
*/

//Hacemos uso de SoftwareSerial para usar dos pines cualquiera de arduino
#include <SoftwareSerial.h>

SoftwareSerial mySerial(5,6); //rx,tx
String inGSM; //Almacenar la informaci'on leida del modulo gsm
String numList[]={"600000000","612345678"}; //N'umeros autorizados
boolean intrusos=false;
double ttl=0; //Tiempo de vida

void setup()
{
  pinMode(9, OUTPUT); // rel'e
  pinMode(2, INPUT); // pulsador
  pinMode(11, OUTPUT); // altavoz
  pinMode(10, OUTPUT); // igt modulo gsm tc35
  mySerial.begin(9600);
  Serial.begin(9600);
  initGSM(); //iniciar el modulo gsm
}
 
void initGSM(){
  digitalWrite(9,HIGH); //desactivamos el rel'e (activo a nivel bajo)
  digitalWrite(10,LOW); //activamos el modulo gsm (activo a nivel bajo)
  Serial.println("Iniciando...");
  mySerial.print("AT+CPIN=1234\r\n"); //Enviar PIN (1234)
  delay(10000);
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
   mySerial.print("AT+CMGS=666554433\r\n"); //Comando AT enviar SMS
   delay(1000);
   mySerial.print("Estoy operativo ;)"); //Mensaje
   delay(1000);
   mySerial.print("\x1A"); //Enviar ascii SUB  
}

void loop() {
  inGSM="";
  ttl++;
  
  //Si se detectan intrusos llamar al administrador (666554433)
  //Se ha liberado el pulsador
  if (digitalRead(2) == HIGH && intrusos==false){
      intrusos=true;
      mySerial.print("ATD666554433;\r\n");
      delay(500);
  }

  //Si intrusos, emitir sonido (alarma).
  if (intrusos){
     tone(11,800,100);
     delay(200);
  }
  
  //Leer enventos en modulo gsm
  while (mySerial.available()>0){
     delay(10);
     inGSM += (char)mySerial.read(); 
  }
  
  Serial.print(inGSM);

  //Si llamada
  if (inGSM.indexOf("RING")>-1){
      delay(1000);
      mySerial.print("AT+CHUP\r\n"); //Rechazar llamada
     
      String num;
      int inicio=inGSM.indexOf('"')+1;
      int fin=inicio+9;
      
      num=inGSM.substring(inicio,fin); //Extraer n'umero    
      
      //Si es el administrador, detener alarma.
      if (num=="666554433"){
         intrusos=false; 
      }
      
      //Si autorizado, accionar rel'e    
      if (isAllow(num)){
        fire();
      }else{ // *** Opcional ***
         // Si n'umero de control (950000000)
         if (num=="950000000"){
	   // Enviar SMS al administrador para consumir saldo.
           sendSMS();
         }
      }
  }
   
  //En caso de perder la cobertura, volver a conectarse a la red
  if (inGSM.indexOf("CREG:")>-1 && inGSM.indexOf("CREG: 0,1")==-1 && inGSM.indexOf("CREG: 0,5")==-1){
    Serial.println("Se ha perdido la cobertura, reiniciando...");
    mySerial.print("AT+CFUN=1,1\r\n");
    delay(4000);
    initGSM();
  }
    
  //Comprobar estado de la red
  if (ttl==150000){
     mySerial.print("AT+CREG?\r\n");
     ttl=0;
  }   
}

