//#define flex_pin A0

int get_strength(int flex_pin) {
    Serial.print("    --Detect--Strength--Of-");
    Serial.print(flex_pin);
    Serial.println("--");
    int flex_value_S = analogRead(flex_pin);
    Serial.print("    Now_Flex: ");
    Serial.print(flex_value_S);
    // delay(3000);
    return flex_value_S;
}
int push_strength(int strength) {
    if (strength < 5)
        return 0;
    if (strength < 25)
        return 1;
    if (strength < 60)
        return 2;
    if (strength < 100)
        return 3;
    if (strength < 140)
        return 4;
    if (strength >= 300)
        return 5;
}
