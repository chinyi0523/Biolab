#include <SoftwareSerial.h>
#include "curve.h"
#include "TouchButton16.h"
CTtP229TouchButton g_ttp229Button;

#define touchr1 g_ttp229Button
#define flex_pin1 A1
#define flex_pin2 A2
#define flex_pin3 A3
#define flex_pin4 A4
#define flex_pin5 A5

int prev[5] = {0,0,0,0,0}; //right_hand_status_last_iter
int now[5] = {0,0,0,0,0};  //right_hand_status_current_iter
float strength[5] = {0,0,0,0,0}; //difference
bool _1pressed[17] = {false}; //left_hand_first_16
bool dimension[4]={false};

//CTtP229TouchButton touchr1;

void setup()
{
  Serial.begin(115200);
  //Serial.println("First Board Touch16&Curve");
  prev[0] = get_strength(flex_pin1);
  prev[1] = get_strength(flex_pin2);
  prev[2] = get_strength(flex_pin3);
  prev[3] = get_strength(flex_pin4);
  prev[4] = get_strength(flex_pin5);
  //Serial.println();
  DEBUG_STATUS(Serial.println("===================================================================="));
  DEBUG_STATUS(Serial.println("Button Input tester started"));  
  DEBUG_STATUS(Serial.println("===================================================================="));
  
  touchr1.Configure(7, 2);//scl7,sdo2
}
void loop()
{
  //Serial.println("---Start--Loop---");
  //Serial.println("  --Start--Flex--");
  now[0] = get_strength(flex_pin1);
  now[1] = get_strength(flex_pin2);
  now[2] = get_strength(flex_pin3);
  now[3] = get_strength(flex_pin4);
  now[4] = get_strength(flex_pin5);
  //Serial.println("  --Start--Touch--");
  TestEvent(touchr1,_1pressed);
  
  ////
  //printTouchStatus(1,_1pressed);
  for(int i =0; i<4;i++)
    dimension[i]= false; 

  if(_1pressed[1]||_1pressed[2]||_1pressed[6]||_1pressed[5]) dimension[3]=true;
  if(_1pressed[3]||_1pressed[4]||_1pressed[7]||_1pressed[8]) dimension[0]=true;
  if(_1pressed[9]||_1pressed[10]||_1pressed[13]||_1pressed[14]) dimension[2]=true;
  if(_1pressed[11]||_1pressed[12]||_1pressed[15]||_1pressed[16]) dimension[1]=true;

  Serial.print('d');
  Serial.print(dimension[0]);
  Serial.print(dimension[1]);
  Serial.print(dimension[2]);
  Serial.println(dimension[3]);
  ////
  //
  //Serial.println("  --Start--ECG--");
  //int ECG = analogRead(A0);
  //Serial.println("  --Start--Analyze--");
  for(int i=0;i<5;i++){
    strength[i] = push_strength(now[i]-prev[i]);
    
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
  /*
  bool SIGNALS[95]={false};
  for(int i=0;i<5;i++){
    if(strength[i]>0){
      SIGNALS[64+5*i+strength[i]]=true;
    }
  }
  for(int i=1;i<5;i++){
    SIGNALS[i]=dimension[i-1];  
  }
  SIGNALS[5]=dimension[1]&&dimension[2];
  SIGNALS[6]=dimension[1]&&dimension[3];
  SIGNALS[7]=dimension[1]&&dimension[4];
  SIGNALS[8]=dimension[2]&&dimension[3];
  SIGNALS[9]=dimension[2]&&dimension[4];
  SIGNALS[10]=dimension[3]&&dimension[4];
  //if(ECG>900) SIGNALS[94]=true;
  */
  Serial.print('s');
  Serial.print(strength[0]);
  Serial.print(strength[1]);
  Serial.print(strength[2]);
  Serial.print(strength[3]);
  Serial.println(strength[4]);
  
  //TODO: pass by USB
  delay(25);
  for(int i=0;i<5;i++){
    prev[i]=now[i];
    now[i]=0;
    strength[i]=0;
  }
}
void printTouchStatus(int num,bool* save)
{
  
  Serial.print("Board ");
  Serial.print(num);
  Serial.println("'s pressed num: ");
  
  for(int i=0;i<=16;i++){
    if(save[i]) 
    {
      Serial.print(i);
      Serial.print(" ");
    }
    
  }
  Serial.println(" |");
}
