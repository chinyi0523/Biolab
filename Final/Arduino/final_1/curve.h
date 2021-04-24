//#define flex_pin A0

int get_strength(int flex_pin) {
    /*
    Serial.print("    --Detect--Strength--Of-");
    Serial.print(flex_pin);
    Serial.println("--");
    */
    int flex_value_S = analogRead(flex_pin);
    /*
    Serial.print("    Now_Flex: ");
    Serial.print(flex_value_S);
    */
    // delay(3000);
    return flex_value_S;
}
float push_strength(int strength) {
    int strength_min = 5;
    int strength_max = 300;
    if (strength <= strength_min)
        return 0;
    else if (strength >= strength_max)
        return 1;
    else
        return float(strength - strength_min) / strength_max;
}
