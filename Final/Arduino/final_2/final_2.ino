#include <SoftwareSerial.h>
#define _C1 2
#define _D1 3
#define _E1 4
#define _F1 5
#define _G1 6
#define _A1 7
#define _B1 8
#define _C2 9
#define _D2 10
#define _E2 11
#define _F2 12
#define _G2 13

#define lleg1 A1
#define lleg2 A2
#define rleg1 A3
#define rleg2 A4

int leftleg[2] = {0};
int rightleg[2] = {0};
void setup() {
    Serial.begin(115200);

    Serial.println("Second Board C1-G2 EMG");
}
void loop() {
    // Serial.println("  --Start--EMG--");
    leftleg[0] = analogRead(lleg1);
    leftleg[1] = analogRead(lleg2);
    rightleg[0] = analogRead(rleg1);
    rightleg[1] = analogRead(rleg2);
    //

    bool SIGNALS[95] = {false};
    for (int i = 2; i <= 13; i++) {
        int pressed = digitalRead(i);
        if (pressed == 1) {
            SIGNALS[19 + i] = true;
        }
    }
    int thres = 700;
    if (leftleg[0] > thres)
        SIGNALS[90] = true;
    if (leftleg[1] > thres)
        SIGNALS[91] = true;
    if (rightleg[0] > thres)
        SIGNALS[92] = true;
    if (rightleg[1] > thres)
        SIGNALS[93] = true;
    //////ASCII NUM to Signal
    // 01-16 RIGHT_FIRST
    // 17-32 RIGHT_SECOND
    // 33-48 RIGHT_THIRD
    // 49-64 LEFT
    // 65-69 THUMB 1-5
    // 70-74 FORE FINGER 1-5
    // 75-79 MIDDLE FINGER 1-5
    // 80-84 RING FINGER 1-5
    // 85-89 LITTLE FINGER 1-5
    // 90-91 LEFTLEG EMG
    // 92-93 RIGHTLEG EMG
    // 94    ECG
    // 00    RESERVED
    ///////////////////////////

    // TODO: pass by USB
    delay(25);
}
