/*****************************************************************************/
/*
  Test for Atmel SIGFOX shield ATAB0101A.
  This Arduino sketch sends power supply voltage measure and temperature measure
  every hour (or when shield button is pressed), using SIGFOX network.
  This sketch uses the Atmel SIGFOX shield library included.
*/
/*****************************************************************************/

/*
    Copyright (c) 2015 Daniele Denaro and
  - FuturaGroup srl (www.futurashop.it)
  - Open-Electronics (www.open-electronics.org)
  - Atmel Corporation.  
  All right reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.
*/

#include "AtmSIGFOXshd.h"    // Atmel SIGFOX shield library 
#include <Sleep_n0m1.h> // https://github.com/n0m1/Sleep_n0m1

Sleep sleep;


SIGFOXshield SIGsh;          // load class SIGFOXshield

#define TIMEINTERVALL 3600000 // 3600s ec -> 1 hour sleep time

#define lpb 60
#define photores A0
#define moisture A1
char pbuff[lpb];

unsigned long timer;          // timer for sleeping
unsigned int count=0;         // samples counter
unsigned int lum;
unsigned int moist;

boolean ishere;

void setup() {
  Serial.begin(9600);
  ishere=SIGsh.begin();             // initialize library
  if (!ishere) {Serial.println("Shield error or not present!");return;}
  unsigned char *buffer;
  buffer=SIGsh.getSigVersion();              // get version
  unsigned char ver[2]={buffer[0],buffer[1]};
  buffer=SIGsh.getID();                      // get ID
  unsigned char ID[4]={buffer[0],buffer[1],buffer[2],buffer[3]};
  unsigned char *PAC=SIGsh.getPAC();         // get PAC
                                  // display informations
  Serial.print("\r\n\r\n---------------- ArduinoUNO Demo (c)2015 FuturaGroup V1.0.0 ----------------\r\n");
  snprintf(pbuff,lpb,"                        ATmega328P ATA8520-V%d.%d\r\n",ver[0],ver[1]);
  Serial.print(pbuff);
  Serial.print("                       Sensor Sigfox Transmitter\r\n");
  Serial.print("--------------------------------------------------------------------------\r\n");
  snprintf(pbuff,lpb,"                    ID  = %02X%02X%02X%02X\r\n",ID[3],ID[2],ID[1],ID[0]);
  Serial.print(pbuff);
  Serial.print("                    PAC = ");
  int i;for(i=0; i<8; i++) {snprintf(pbuff,lpb,"%02X",PAC[i]); Serial.print(pbuff);}
  Serial.print("\r\n---------------------------------------------------------------------------\r\n");
  Serial.print("\r\nwait for Timer... or press button\r\n");
                                   
  SIGsh.SIGFOXoff();               // switch off SIGFOX module
  SIGsh.TempOff();                 // switch off temperature sensor

  //timer=millis()+TIMEINTERVALL;    // initialize timer for wake-up
  timer = TIMEINTERVALL;
}

void loop() 
{
  if (!ishere) return;
//  if (SIGsh.getButton())            // if it is sleeping but button pressed
//  {
//    sensorActivity();               // act
//    delay(300);
//  }
//  if (millis()>timer)               // if timer expired
//   {sensorActivity(); timer=millis()+TIMEINTERVALL;}  // act and reset timer
  delay(300);
  sensorActivity();
  sleep.pwrDownMode(); //set sleep mode
  sleep.sleepDelay(timer); //sleep for: sleepTime
  delay(300);
  
}

void sensorActivity()
{
    count++;                         // increment samples counter
    Serial.print("Sample number ");Serial.println(count);
    
    // Get photoresistance values and convert it in percentage
    lum = analogRead(photores);
    lum = map(lum, 0, 1023, 0, 100); // values between 0 and 1023. Need to be calibrated
    Serial.print("Luminosity : ");
    Serial.print(lum);
    Serial.println(" %");
    
    //Get moisture sensor values
    moist = analogRead(moisture);
    moist = map(moist, 0, 512, 0, 100); // values between 0 and 1023. Need to be calibrated
    Serial.print("Ground humidity  : ");
    Serial.print(moist);
    Serial.println(" %");
    
    SIGsh.SIGFOXon();                // switch on SIGFOX module and temp. module
    SIGsh.TempOn();
    
    //Get power using lib
    unsigned int v=SIGsh.getVpwr();  // read power supply voltage
    Serial.print("  Power supply: ");Serial.print(v);Serial.println("mV");
    //get temp using lib
    float t=SIGsh.readTemp();        // read temperature (Celsius degrees)
    SIGsh.TempOff();                 // switch off modules
    Serial.print("  Temperature : ");Serial.print(t);Serial.println(" C");
    sendvalues(v,t, moist, lum);                 // send values
    SIGsh.SIGFOXoff();
}

void sendvalues(int v,float t, int moist, int lum)
{
    int i;
    unsigned char sbuff[20];         // sending buffer
    //for(i=0; i<9; i++) sbuff[i]  = 0x00;   // initialize first 9 bytes
    int it=int(t);                   // float temp to int
    sbuff[0]=lowByte(it);            // temp to buffer
    sbuff[1]=lowByte(moist);         // moisture to buffer
    sbuff[2]=lowByte(lum);           // lum to buffer
    sbuff[3]=highByte(v);           // voltage to buffer
    sbuff[4]=lowByte(v);
    Serial.print("  Sending(hex): ");
    for(i=0; i<5; i++) {snprintf(pbuff,lpb,"%02X",sbuff[i]); Serial.print(pbuff);} Serial.println();
    int ret=SIGsh.sendMessage(sbuff,5);     // send buffer to SIGFOX network
    Serial.println(SIGsh.getStatusSig());
    if (ret>0) Serial.println("No transmission!");
}


