//librerias
#include <Arduino.h>

//antena
#include <SPI.h>
#include "printf.h"
#include "RF24.h"

#include <Wire.h>

//Antena
#define CE_PIN 7
#define CSN_PIN 8
RF24 radio(CE_PIN, CSN_PIN);
uint8_t address[][6] = { "1Node", "2Node" };
bool radioNumber = 0;

//datos
float caida = 0.0;
float datos [3] = {0, 0, 0};

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

//Inicio del codigo

void setup() {
  radio.begin();
  radio.setPALevel(RF24_PA_LOW);
  radio.setPayloadSize(sizeof(caida));
  radio.openWritingPipe(address[radioNumber]);
  radio.openReadingPipe(1, address[!radioNumber]);
  radio.startListening();
  radio.setDataRate(rf24_datarate_e::RF24_2MBPS);
  radio.enableDynamicPayloads();

  pinMode(3, OUTPUT);
  pinMode(5, OUTPUT);
  pinMode(6, OUTPUT);
  pinMode(10, OUTPUT);
  pinMode(2, INPUT_PULLUP);
}

void loop() {
  uint8_t pipe;
  if (radio.available(&pipe)) {
    radio.read(&datos, radio.getDynamicPayloadSize());
    caida = datos[0];
    caida_();
  }
  delay(10);
}

//https://automaticaddison.com/2-way-communication-between-raspberry-pi-and-arduino/