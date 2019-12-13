//#define flex_pin A0


int get_strength(flex_pin)
{
  Serial.println("---Start--Detect--Strength---");
  int flex_value_S = analogRead(flex_pin);
  delay(500);
  int flex_value_F = analogRead(flex_pin);
  int strength = flex_value_S-flex_value_F;
  Serial.print("Difference: ");
  Serial.print(strength);
  Serial.print(" Init: ");
  Serial.print(flex_value_S);
  Serial.print(" Term: ");
  Serial.println(flex_value_F);
  Serial.print("Strength (push:min 1-5 max, 0:released): ");
  Serial.println(push_strength(strength));
  delay(3000);
}
int push_strength(int strength){
  if(strength<5)
    return 0;
  if(strength<25)
    return 1;
  if(strength<60)
    return 2;
  if(strength<100)
    return 3;
  if(strength<140)
    return 4;
  if(strength>=300)
    return 5;
}
