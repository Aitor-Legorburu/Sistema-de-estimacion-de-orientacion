#include <Arduino.h>

//Librerias del sensor
#include "I2Cdev.h"
#include "MPU6050_6Axis_MotionApps20.h"

//Librerias de la antena
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

//Variables del sensor y de los datos
MPU6050 mpu;
uint8_t fifoBuffer[64];
Quaternion q;
VectorFloat gravity;
VectorInt16 aa;
VectorInt16 aaReal;
float caida;
float ace_x;
float giro_y;
float datos [3] = {0, 0, 0};
int error_x = 0;
int error_y = 0;

//Cosas de la radio
#define CE_PIN 7
#define CSN_PIN 8
RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = 1;

//Función para actualizar los leds. He cambiado el comportamiento de los leds. Voy a ir cambiando la sensibilidad hasta dar con 
//la adecuada (por ahora solo la he ido bajando y bajando, no se hasta cuando lo tendre que bajar). Ahora los leds se encienden 
//gradualmente en vez de de golpe.
void caida_(){
  float primer_valor;
  float segundo_valor;
  float tercer_valor;
  if(digitalRead(2) == HIGH){ //sensible
    primer_valor = 1;
    segundo_valor = 6.5;
    tercer_valor = 15;
  }
  else{ // muy sensible
    primer_valor = 0.5;
    segundo_valor = 5;
    tercer_valor = 12.5;
  }

  if( primer_valor < caida && caida < segundo_valor){
    digitalWrite(3, LOW);
    Serial.println(((caida - primer_valor) * 255) / (segundo_valor-primer_valor));
    analogWrite(5,((caida - primer_valor) * 255) / (segundo_valor-primer_valor));
    digitalWrite(6, LOW);
    digitalWrite(10, LOW);
  }
  else if ( segundo_valor < caida && caida < 90){
    if(caida < tercer_valor){
      analogWrite(3, ((caida - segundo_valor) * 255) / (tercer_valor-segundo_valor));
    }
    else{
      digitalWrite(3, HIGH);
    }
    digitalWrite(5, HIGH);
    digitalWrite(6, LOW);
    digitalWrite(10, LOW);
  }
  else if( -primer_valor > caida && caida > -segundo_valor){
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    analogWrite(6,((-caida - primer_valor) * 255) / (segundo_valor-primer_valor));
    digitalWrite(10, LOW);
  }
  else if ( -segundo_valor > caida && caida > -90){
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, HIGH);
    if(caida > -tercer_valor){
      analogWrite(10, ((-caida - segundo_valor) * 255) / (tercer_valor-segundo_valor));
    }
    else{
      digitalWrite(10, HIGH);
    }
  }
  else if( -primer_valor < caida && caida < primer_valor){
    digitalWrite(3, LOW);
    digitalWrite(5, LOW);
    digitalWrite(6, LOW);
    digitalWrite(9, LOW);
  }
}

void setup() {
  //iniciar sensor
  Wire.begin();
  mpu.initialize();
  mpu.dmpInitialize();
  mpu.setDMPEnabled(true);

  //Los leds y el conmutador de sensibilidad
  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT_PULLUP);

  //Cosas de la radio
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.enableDynamicPayloads();
  radio.openWritingPipe(address[radioNumber]);
  radio.openReadingPipe(1, address[!radioNumber]);
  radio.stopListening();
  radio.setDataRate(rf24_datarate_e::RF24_2MBPS);
  radio.setPALevel(RF24_PA_MAX,true);

  //Para quitar ese pequeño error de que la placa en la caja no esta totalmente recta, tal y como me dijiste, al principio del 
  //  codigo va a leer como está la placa orientada para sacar ese error. Ahora, al encender el emimsor (el receptor da igual), debe 
  //  de ser encendida en un lugar plano (preferidamente en el pantalan, que siempre esta plano)
  if(digitalRead(2) == HIGH){
    delay(5000);
    while (error_x == 0 || error_y == 0) {
      if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
        mpu.dmpGetQuaternion(&q, fifoBuffer);
        mpu.dmpGetGravity(&gravity, &q);
        error_x = atan(gravity.y/gravity.z)*180/PI;
        error_y = atan(gravity.x/gravity.z)*180/PI;
      }
    }
  }
}

void loop() {
  if (mpu.dmpGetCurrentFIFOPacket(fifoBuffer)) {
    //Los datos del sensor son leidos y enviados al receptor
    mpu.dmpGetQuaternion(&q, fifoBuffer);
    mpu.dmpGetAccel(&aa, fifoBuffer);
    mpu.dmpGetGravity(&gravity, &q);
    mpu.dmpGetLinearAccel(&aaReal, &aa, &gravity);
    caida = (atan(gravity.y/gravity.z)*180/PI) - error_x;
    ace_x = aaReal.x;
    giro_y = (atan(gravity.x/gravity.z)*180/PI) - error_y;
    datos [0] = caida;
    datos [1] = ace_x;
    datos [2] = giro_y;
    radio.write(&datos, sizeof(datos));
    //Los leds se actualizan
    caida_();
  }
}

//Este codigo, si bien mas corto, creo que como la libreria del sensor pesa un muerto, peta la memoria flash del arduino 
//(tarda 12 segundos en subir el código)