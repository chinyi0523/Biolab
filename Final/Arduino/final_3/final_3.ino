#include <SoftwareSerial.h>
#define _A2 2
#define _B2 3
#define _C3 4
#define _D3 5
#define _E3 6
#define _F3 7
#define _G3 8
#define _A3 9
#define _B3 10

void setup()
{
  Serial.begin(115200);
 
  Serial.println("Third Board A2-B3");
  
}
void loop()
{
  bool SIGNALS[95]={false};
  for(int i=2;i<=10;i++){
    int pressed = digitalRead(i);
    if(pressed==1){
      SIGNALS[31+i] = true;
    }  
  }
  //////ASCII NUM to Signal
  //01-16 RIGHT_FIRST
  //17-32 RIGHT_SECOND
  //33-48 RIGHT_THIRD  
  //49-64 LEFT
  //65-69 THUMB 1-5
  //70-74 FORE FINGER 1-5
  //75-79 MIDDLE FINGER 1-5
  //80-84 RING FINGER 1-5
  //85-89 LITTLE FINGER 1-5
  //90-91 LEFTLEG EMG
  //92-93 RIGHTLEG EMG 
  //94    ECG
  //00    RESERVED
  ///////////////////////////
  
  printpressed(SIGNALS);
  //TODO: pass by USB
  delay(250);
}
void printpressed(bool* sig){
  for(int i=0;i<95;i++){
    if(sig[i]){
      printnote(i);
      Serial.print(" ");
    }
  }
  Serial.println();
}
void printnote(int i){
  if(i==33) Serial.print("La  Middle");
  if(i==34) Serial.print("Si  Middle");
  if(i==35) Serial.print("Do  High  ");
  if(i==36) Serial.print("Re  High  ");
  if(i==37) Serial.print("Mi  High  ");
  if(i==38) Serial.print("Fa  High  ");
  if(i==39) Serial.print("Sol High  ");
  if(i==40) Serial.print("La  High  ");
  if(i==41) Serial.print("Si  High  ");
}
