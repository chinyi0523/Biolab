#include <SPI.h>

#if 0
#define DEBUG_BUTTON16(a) (a)
#else
#define DEBUG_BUTTON16(a)
#endif

#if 1
#define DEBUG_STATUS(a) (a)
#else
#define DEBUG_STATUS(a)
#endif

#pragma pack(push, 0)

// TBD: Replace with proper interrupt pin macros. It does not seem to be defined
// for atmega328p or I am incapable of finding it
/*#ifndef INT0_PIN
#ifdef __AVR_ATmega328P__
#define INT0_PIN 2
#else
  #warning Define INT0_PIN for this microcontroller to use interrupt
#endif
#endif

#ifndef INT1_PIN
#ifdef __AVR_ATmega328P__
#define INT1_PIN 3
#else
  #warning Define INT1_PIN for this microcontroller to use interrupt
#endif
#endif*/

uint32_t g_intCount = 0;

struct CTtp229ButtonEvent {
    uint8_t ButtonNumber : 5;      // ButtonNumber != 0 : Event is present. if
                                   // equals 0, no event is received
    uint8_t IsButtonReleased : 1;  // True = Button is pressed. False = Button
                                   // is released
};

class CTtP229TouchButton {
    struct CTtp229Prop {
        uint16_t SclPin : 6;
        uint16_t SdoPin : 6;
        uint16_t Is16Button : 1;
        /*#if defined(INT0_PIN) || defined(INT1_PIN)
            uint16_t  HasPendingInterrupt : 1;
            uint16_t  IgnoreNextEvent : 1;            // When reading 16th key
        and if it is pressed, SDO stays low for 2ms.
                                                      // If we enable interrupt
        before that, then it will trigger after 2ms, only to find the same
        condition.
                                                      // To make things worse,
        at the end of reading the pin will stay low and generate another
        interrupt.
                                                      // TBD: One possible fix
        is to send more pulses to make it roll over to HIGH. Have to find out if
        all 16 keys can be pressed in multi-key scenario (NOT supported yet).
            uint16_t UnhandledButtonPresses;
        #endif*/
        uint16_t PreviousButtonValue;
    };

    static CTtp229Prop g_prop;

    //
    //    Internal function that captures the data from TTP229 on which key is
    //    pressed.
    //  Currently, this function only supports one key being pressed. Multi-key
    //  press needs to be added later.
    //
    //  Return Value : Bit field of buttons pressed
    //
    static uint16_t GetPressedButton() {
        DEBUG_BUTTON16(Serial.println("GetPressedButton : Enter "));
        uint16_t buttonsPressed = 0;
        // Need to generate the LOW and then HIGH on the clock and read the
        // value from data when clock is back high. As per the specification,
        // the TTP229 chip can support 512Khz. This translates to approximately
        // 2us for a cycle. So introducing clock with fall and raise each of
        // 1us.
        uint8_t maxCnt = g_prop.Is16Button ? 16 : 8;
        for (uint8_t ndx = 0; ndx < maxCnt; ndx++) {
            digitalWrite(g_prop.SclPin, LOW);
            delayMicroseconds(1);
            digitalWrite(g_prop.SclPin, HIGH);

            int val = digitalRead(g_prop.SdoPin);

            delayMicroseconds(
                1);  // Technically this can be moved after the if for most
                     // atmel microcontrollers. But if there is a really fast
                     // one (now/future) and the next call to GetPressedButton
                     // made immediately, we might overclock TTP229. Being safe
                     // here

            if (LOW == val) {
                buttonsPressed |= (1 << ndx);
            }
        }

        DEBUG_BUTTON16(
            Serial.print("GetPressedButton : Exit. Return Value : "));
        DEBUG_BUTTON16(Serial.println(buttonsPressed));

        return buttonsPressed;
    }

    //
    //    Returns button number being pressed. High bit indicates more changes
    //    present
    //
    static uint8_t GetButtonNumberFromFlag(uint16_t buttonsChanged) {
        uint16_t flag = 1;
        for (uint8_t ndx = 1; ndx <= 16; ndx++, flag <<= 1) {
            if ((buttonsChanged & flag) != 0) {
                if ((buttonsChanged & ~flag) != 0) {
                    // Some other bit is present
                    ndx |= 0x80;
                }

                return ndx;
            }
        }

        return 0;
    }

   public:
    //
    //    Setup the TTP229 Touch button on this input.
    //
    // Inputs:
    //     sclPin  - Clock Pin of the button (3rd from left on button, connected
    //     to arduino's digital pin number) sdoPin  - Data pin to read from the
    //     button (4th pin from left on button, connected to arduino's digital
    //     pin number) is16Button - true = 16 buttons board. false = 8 button
    //     board
    //
    static void Configure(int sclPin, int sdoPin, bool is16Button = true) {
        DEBUG_BUTTON16(Serial.println("Configure : Enter"));

        g_prop.SclPin = sclPin;
        g_prop.SdoPin = sdoPin;
        g_prop.Is16Button = is16Button;

        g_prop.PreviousButtonValue = 0;

        // Configure clock as output and hold it high
        pinMode(sclPin, OUTPUT);
        digitalWrite(sclPin, HIGH);

        // Configure data pin as input
        pinMode(sdoPin, INPUT);

        DEBUG_BUTTON16(Serial.print("Button Configuration\n\rSCL Pin : "));
        DEBUG_BUTTON16(Serial.println(sclPin));
        DEBUG_BUTTON16(Serial.print("SDO Pin : "));
        DEBUG_BUTTON16(Serial.println(sdoPin));
        DEBUG_BUTTON16(Serial.print("Number of Keys : "));
        DEBUG_BUTTON16(Serial.println(is16Button ? 16 : 8));

#if defined(INT0_PIN) || defined(INT1_PIN)
        g_prop.UnhandledButtonPresses = 0;
        g_prop.HasPendingInterrupt = false;
        g_prop.IgnoreNextEvent = false;
        SetInterruptHandler();
#endif

        DEBUG_BUTTON16(Serial.println("Configure : Exit"));
    }

    //
    //    Get the current status from the 16 button touch device
    //
    //   Return Value : Returns the bitflag of the keys pressed. returns 0, if
    //   no key is pressed.
    //
    static uint16_t GetButtonStatus() {
        uint16_t returnValue = GetPressedButton();

        g_prop.PreviousButtonValue = returnValue;

        return returnValue;
    }

    //
    //    Gets the event from the button. This is useful for monitoring press
    //    and release only.
    // Each button press will generate max 2 events, one for press and another
    // for release. When the button is press and held, this method will return
    // no event. If the calls were not made often enough, the events could be
    // missed. For instance, you might get 2 pressed, followed by 4 pressed,
    // which automatically means 2 released in single key mode.
    //
    // Return Value : if ButtonNumber is 0, then no event
    //
    static CTtp229ButtonEvent GetButtonEvent() {
        CTtp229ButtonEvent returnValue = {0, 0};
        uint8_t buttonNumber;

        DEBUG_BUTTON16(Serial.print("Old Value  : "));
        DEBUG_BUTTON16(Serial.println(g_prop.PreviousButtonValue));

        uint16_t currValue = GetPressedButton();
        uint16_t changes = g_prop.PreviousButtonValue ^ currValue;
        uint16_t pressed = (changes & currValue);
        uint16_t released = (changes & g_prop.PreviousButtonValue);

        // First check if any key is that is pressed and generate press event
        if (0 != pressed) {
            buttonNumber = GetButtonNumberFromFlag(pressed);
            returnValue.ButtonNumber = (buttonNumber & 0x7F);

            uint16_t mask = (1 << (returnValue.ButtonNumber - 1));
            // set the new notified button into prev
            g_prop.PreviousButtonValue |= mask;

        } else if (0 != released) {
            buttonNumber = GetButtonNumberFromFlag(released);
            returnValue.ButtonNumber = (buttonNumber & 0x7F);

            // The unmatching bit whose previous value of 1 means, it is
            // released
            returnValue.IsButtonReleased = true;

            // clear the notified release button
            g_prop.PreviousButtonValue &=
                ~(1 << (returnValue.ButtonNumber - 1));
        }
        return returnValue;
    }
};

////Start from here Above is class!!!
CTtP229TouchButton::CTtp229Prop CTtP229TouchButton::g_prop;
// CTtP229TouchButton g_ttp229Button;
CTtP229TouchButton aaa;
CTtP229TouchButton bbb;
//#define TTP16Button g_ttp229Button
#pragma pack(pop)

void setup() {
    Serial.begin(115200);

    DEBUG_STATUS(
        Serial.println("======================================================="
                       "============="));
    DEBUG_STATUS(Serial.println("Button Input tester started"));
    DEBUG_STATUS(
        Serial.println("======================================================="
                       "============="));
    bbb.Configure(8, 3);
    aaa.Configure(7, 2);
}

void TestStatus() {
    uint8_t buttonNumber = aaa.GetButtonStatus();

    if (0 != buttonNumber) {
        DEBUG_STATUS(Serial.print("Button Pressed : "));
        DEBUG_STATUS(Serial.println(buttonNumber));
    }

    delayMicroseconds(2500);  // TTP229 document says it will reset the output
                              // if 2ms idle + little bit safety
}

void TestEvent(CTtP229TouchButton rrr) {
    {
        CTtp229ButtonEvent buttonEvent = rrr.GetButtonEvent();

        if (0 != buttonEvent.ButtonNumber) {
            if (buttonEvent.IsButtonReleased) {
                DEBUG_STATUS(Serial.print("Button Released: "));
                DEBUG_STATUS(Serial.println(buttonEvent.ButtonNumber));
            } else {
                DEBUG_STATUS(Serial.print("Button Pressed : "));
                DEBUG_STATUS(Serial.println(buttonEvent.ButtonNumber));
            }
            // DEBUG_STATUS(Serial.println(buttonEvent.ButtonNumber));
        }

        // Serial.print("CurrentTime : "); Serial.println(millis());
        delayMicroseconds(
            2500);  // TTP229 document says it will reset the output if 2ms idle
                    // + little bit safety. Not required if using interrupts
    }
}

void loop() {
    // TestStatus();
    TestEvent(aaa);
    TestEvent(bbb);
}
