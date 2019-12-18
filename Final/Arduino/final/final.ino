#include <SoftwareSerial.h>
#include "curve.h"
#include "TouchButton16.h"

#define flex_pin1 A1
#define flex_pin2 A2
#define flex_pin3 A3
#define flex_pin4 A4
#define flex_pin5 A5
#define lleg1 6
#define lleg2 11
#define rleg1 12
#define rleg2 13
int prev[5] = {0,0,0,0,0}; //right_hand_status_last_iter
int now[5] = {0,0,0,0,0};  //right_hand_status_current_iter
int strength[5] = {0,0,0,0,0}; //difference
bool _1pressed[16] = {false}; //right_hand_first_16
bool _2pressed[16] = {false}; //right_hand_second_16
bool _3pressed[16] = {false}; //right_hand_third_16
bool _4pressed[16] = {false}; //left_hand_16
bool leftleg[2] = {false}; 
bool rightleg[2] = {false};
CTtP229TouchButton touchr1;
CTtP229TouchButton touchr2;
CTtP229TouchButton touchr3;
CTtP229TouchButton touchl4;
void setup()
{
  Serial.begin(115200);
  prev[0] = get_strength(flex_pin1);
  prev[1] = get_strength(flex_pin2);
  prev[2] = get_strength(flex_pin3);
  prev[3] = get_strength(flex_pin4);
  prev[4] = get_strength(flex_pin5);
  DEBUG_STATUS(Serial.println("===================================================================="));
  DEBUG_STATUS(Serial.println("Button Input tester started"));  
  DEBUG_STATUS(Serial.println("===================================================================="));
  
  touchr1.Configure(7, 2);//scl7,sdo2
  touchr2.Configure(8, 3);//scl8,sdo3
  touchr3.Configure(9, 4);//scl9,sdo4
  touchl4.Configure(10,5);//scl10,sdo5
}
void loop()
{
  Serial.println("---Start--Loop---");
  Serial.println("  --Start--Flex--");
  now[0] = get_strength(flex_pin1);
  now[1] = get_strength(flex_pin2);
  now[2] = get_strength(flex_pin3);
  now[3] = get_strength(flex_pin4);
  now[4] = get_strength(flex_pin5);
  Serial.println("  --Start--Touch--");
  TestEvent(touchr1,_1pressed);
  TestEvent(touchr2,_2pressed);
  TestEvent(touchr3,_3pressed);
  TestEvent(touchl4,_4pressed);
  ////
  printTouchStatus(1,_1pressed);
  printTouchStatus(2,_2pressed);
  printTouchStatus(3,_3pressed);
  printTouchStatus(4,_4pressed);
  ////
  Serial.println("  --Start--EMG--");
  leftleg[0]=digitalRead(6);
  leftleg[1]=digitalRead(11);
  rightleg[0]=digitalRead(12);
  rightleg[1]=digitalRead(13);
  //
  Serial.println("  --Start--ECG--");
  int ECG = analogRead(A0);
  Serial.println("  --Start--Analyze--");
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
  bool SIGNALS[95]={false};
  analyze(SIGNALS);
  if(ECG>900) SIGNALS[94]=true;
  //TODO: pass by USB
  delay(200);
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
  for(int i=0;i<16;i++){
    if(save[i]) 
    {
      Serial.print(i);
      Serial.print(" ");
    }
    Serial.println(" |");
  }
}
void analyze(bool *Signal){
  //1-64
  for(int i=1;i<=16;i++){
    Signal[i] = _1pressed[i];
  }
  for(int i=17;i<=32;i++){
    Signal[i] = _2pressed[i];
  }
  for(int i=33;i<=48;i++){
    Signal[i] = _3pressed[i];
  }
  for(int i=49;i<=64;i++){
    Signal[i] = _4pressed[i];
  }
  //65-89
  for(int i=0;i<5;i++){
    if(strength[i]>0){
      Signal[64+5*i+strength[i]]=true;
    }
  }
  //90-93
  Signal[90]=leftleg[0];
  Signal[91]=leftleg[1];
  Signal[92]=rightleg[0];
  Signal[93]=rightleg[1];
}
