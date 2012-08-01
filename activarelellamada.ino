/*
 * Fecha: 14/07/2012
 * Autor: Cristóbal Medina López
 * Licencia:
 * Este Software está distribuido bajo la licencia general pública de GNU, GPL. Puedes encontrar esta licencia completa en http://www.gnu.org.
 * Es gratuito, puedes copiar y utlizar el código líbremente sin cargo alguno. Tienes derecho a modificar
 * el codigo fuente y a distribuirlo por tu cuenta, siempre informando de la procedencia original.
 *
*/

//Hacemos uso de SoftwareSerial para usar dos pines cualquiera de arduino
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11); //rx,tx
String inGSM; //para almacenar la informacion leida desde el modulo gsm
String numList[]={"600000000","612345678"}; //lista de numeros permitidos

void setup()
{
  pinMode(9, OUTPUT); //pin para activar el rele
  mySerial.begin(9600);
  initGSM(); //iniciar el modulo gsm
}

//Funcion para inicializar el modulo gsm 
void initGSM(){
  mySerial.print("AT+CPIN=1234\r\n"); //Enviar PIN (1234 en este caso)
  delay(5000);
  mySerial.print("AT+CLIP=1\r\n"); //Activar alerta de llamada
  delay(100);  
}

//Función para comprobar si ese numero tiene permiso
//devuelve true si tiene permiso, false si no lo tiene.
boolean isAllow(String num){
  for (int i=0;i<sizeof(numList);i++){
     if (num==numList[i])
         return true;
  }
  return false;
}

//Función que activa el rele durante 1.5 segundos
void fire(){
   digitalWrite(9,HIGH);
   delay(1500);
   digitalWrite(9,LOW);
}

void loop() {
  Serial.flush();
  inGSM="";
  
  //Leer del modulo gsm si hay informacion disponible
  while (mySerial.available()>0){
     delay(10);
     inGSM += (char)mySerial.read(); 
  }
  
  //En caso de recibir una llamada
  if (inGSM.indexOf("RING")>-1){
      
      //Se rechaza la llamada
      mySerial.print("AT+CHUP\r\n");
     
      //Se obtiene el numero de telefono que ha realizado la llamada 
      String num;
      int inicio=inGSM.indexOf('"')+1;
      int fin=inicio+9;
      num=inGSM.substring(inicio,fin);      
       
      //Si tiene permiso se acciona el rele    
      if (isAllow(num)){
        fire();
      }
  }
}
