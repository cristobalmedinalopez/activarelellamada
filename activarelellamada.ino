/*
 * Fecha: 14/07/2012
 * Autor: Cristóbal Medina López
 * Licencia:
 * Este Software está distribuido bajo la licencia general pública de GNU, GPL. Puedes encontrar esta licencia completa en http://www.gnu.org.
 * Es gratuito, puedes copiar y utlizar el código líbremente sin cargo alguno. Tienes derecho a modificar
 * el código fuente y a distribuirlo por tu cuenta, siempre informando de la procedencia original.
 *
*/

//Hacemos uso de SoftwareSerial para usar dos pines cualquiera de arduino
#include <SoftwareSerial.h>

SoftwareSerial mySerial(10,11); //rx,tx
String inGSM; //para almacenar la informacion leida desde el modulo gsm
String numList[]={"600112233","612345678"}; //lista de numeros permitidos
boolean intrusos=false;

void setup()
{
  pinMode(9, OUTPUT); //pin para activar el rele
  pinMode(2, INPUT); //pin pulsador alarma
  pinMode(3, OUTPUT); //pin altavoz (buzzer)
  pinMode(12, OUTPUT); //pin ign modulo gsm tc35
  mySerial.begin(9600);
  initGSM(); //iniciar el modulo gsm
}

//Funcion para inicializar el modulo gsm 
void initGSM(){
  digitalWrite(12, LOW); //activamos el modulo gsm (activo a nivel bajo)
  delay(4000);
  mySerial.print("AT+CPIN=1234\r\n"); //Enviar PIN (1234 en este caso)
  delay(8000);
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
   delay(600); //tiempo que permanece activo el rele
   digitalWrite(9,LOW);
}

void loop() {
  Serial.flush();
  inGSM="";
  
  //Detectamos si la alarma se ha activado (se ha liberado el pulsador)
  if (digitalRead(2) == HIGH && intrusos==false){
      intrusos=true;
      mySerial.print("ATD600112233;\r\n"); //llamada avisando de intrusos al numero 600112233
  }
  
  //Si hay intrusos se activa un pitido intermitente por el altavoz
  if (intrusos){
     tone(3,800,100); 
     delay(200);
  }
  
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
      
      //Si el numero es el 600112233 se detiene la alarma
      if (num=="600112233"){
         intrusos=false; 
      }
      
      //Si tiene permiso se acciona el rele    
      if (isAllow(num)){
        fire();
      }
  }
}
