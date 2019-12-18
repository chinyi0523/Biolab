#include <SoftwareSerial.h>
int analogPin = A3;       // Voltage read in
char val[20] = {0};       // Value pass out
float raw = 0;
void setup()
{
  Serial.begin(9600); 
  BT.begin(9600);
}

void loop()
{
    if ( BT.available() ) 
    {
      int _iter = 0;
      for (_iter = 0; _iter<20; _iter++)
      {
          float volt = get_value();
          parse_char(volt,_iter);
          delay(20);
      }
      Serial.println("BT Pass Value");
      BT.write(val);
      for (int i=0;i<20;i++)
      {
         Serial.print(val[i]);
      }
      Serial.println();
    }
}
float get_value()
{
  raw = analogRead(analogPin);
  raw = (raw * 128)/1024;
  return raw;
}
void parse_char(float value, int iter)
{
    int a = 0;
    a = value ;
    char c ;
    c= a;
    val[iter] = c;
}
