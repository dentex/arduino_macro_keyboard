/* 
 * Ultimate Macro Keyboard
 * 
 * Copyright (C) 2020 Samuele Rini <samuele.rini76 [AT] gmail.com>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

/* 
 * ----------------------------------------
 * Realized adapting and modifing code from 
 * the projects specified under CREDITS.md
 * ----------------------------------------
*/

#include <Arduino.h>

#include "Settings.h"

// #include <EEPROMWearLevel.h>
#include <EEPROM.h>

#include <Adafruit_NeoPixel.h>
Adafruit_NeoPixel strip = Adafruit_NeoPixel(LEDS_NUM, DIN_PIN, NEO_GRB + NEO_KHZ800);

#include "SSD1306Ascii.h"
#include "SSD1306AsciiWire.h"
SSD1306AsciiWire oled;

#include "DeadSimpleMenu.h"
DeadSimpleMenu dsm;

#include <Keypad.h>

byte rowPins[ROWS] = {15, 14};     //connect to the row pinouts of the keypad
byte colPins[COLS] = {4, 5, 6, 7}; //connect to the column pinouts of the keypad

Keypad keypad = Keypad(makeKeymap(keys), rowPins, colPins, ROWS, COLS);

int encButtonState = LOW, lastEncButtonState = LOW;
int menuButtonState = LOW, lastMenuButtonState = LOW;
int touchSensorState = LOW, lastTouchSensorState = LOW;

unsigned long lastDebounceTime = 0, debounceDelay = 50;
int _position = 0, _positionExt = 0;
int8_t _oldState;
bool btnPressed = false, btnReleased = false, pressedRotary = false;

const int8_t encoderStates[] = {
    0, -1, 1, 0,
    1, 0, 0, -1,
    -1, 0, 0, 1,
    0, 1, -1, 0};

//Functions prototypes (due to the optional parameter)
//********************************************************************************
void oledOn(const char *msg, bool isIgnoring = false);
void drawMenu(int page = INITIAL_PAGE);
//void keySend(KeyboardKeycode key, bool useCtrl = false, bool useShift = false);
//********************************************************************************

void colorWipe(uint32_t c, uint8_t wait)
{
    for (uint16_t i = 0; i < strip.numPixels(); i++)
    {
        strip.setPixelColor(i, c);
        strip.show();
        delay(wait);
    }
}

void stripSetColor(uint8_t encP = -1, uint8_t keyP = -1)
{
    uint8_t wait = 100;

    switch (keyP)
    {
    case OLIVE_KP:
        colorWipe(strip.Color(0, 255, 0), wait); // Green
        break;
    case NAV_KP:
        colorWipe(strip.Color(255, 0, 0), wait); // Red
        break;
    default:
        switch (encP)
        {
        case MMEDIA_ENC:
            switch (keyP)
            {
            case APPS_KP:
                colorWipe(strip.Color(0, 0, 255), wait); // Blue
                break;
            case KRITA_KP:
                colorWipe(strip.Color(127, 0, 255), wait); // Purple/Blue
                break;
            }
            break;
        case KRITA_ENC:
            switch (keyP)
            {
            case APPS_KP:
                colorWipe(strip.Color(255, 255, 0), wait); // Yellow
                break;
            case KRITA_KP:
                colorWipe(strip.Color(255, 0, 127), wait); // Purple/Red
                break;
            }
            break;
        default:
            colorWipe(strip.Color(0, 0, 0), wait); // Black
            break;
        }
    }
}

/*
// colors 
colorWipe(strip.Color(255, 0, 0), wait); // Red
colorWipe(strip.Color(0, 255, 0), wait); // Green
colorWipe(strip.Color(0, 0, 255), wait); // Blue
colorWipe(strip.Color(0, 0, 0), wait); // Black 
*/

bool menuMode = false;
bool showcaseMode = false;

int storedEncoderProfile;
int currentEncoderProfile;
int encoderProfile;

int storedKeypadProfile;
int currentKeypadProfile;
int keypadProfile;

void quitMenu()
{
    //Serial.print("\n Exting Menu");
    menuMode = false;
    showcaseMode = false;
    exitMenu();
}

void assignEncoderProfile(int encoderProfile)
{
    //Serial.print("\n Assigning encoder profile ");
    //Serial.print(encoderProfile);
    currentEncoderProfile = encoderProfile;

    EEPROM.update(ENCODER_PROFILE_SETTING, encoderProfile);

    stripSetColor(encoderProfile, currentKeypadProfile);
    stripSetColor();
}

void assignKeypadProfile(int keypadProfile)
{
    //Serial.print("\n Assigning keypad profile ");
    //Serial.print(keypadProfile);
    currentKeypadProfile = keypadProfile;

    EEPROM.update(KEYPAD_PROFILE_SETTING, keypadProfile);

    stripSetColor(currentEncoderProfile, keypadProfile);
    stripSetColor();
}

void setup()
{
    pinMode(encoderPinA, INPUT);
    pinMode(encoderPinB, INPUT);
    pinMode(encoderButton, INPUT);

    digitalWrite(encoderButton, LOW);
    _oldState = 3;

    pinMode(TOUCH_SENSOR, INPUT);

    pinMode(MENU_BUTTON, INPUT);
    digitalWrite(MENU_BUTTON, LOW);

    //Serial.begin(9600 * 2);
    //EEPROMwl.begin(EEPROM_LAYOUT_VERSION, AMOUNT_OF_INDEXES);

    // uncomment to wait for serial connection before continuing
    /* while (!Serial)
    {
        ;
    } */

    storedEncoderProfile = EEPROM.read(ENCODER_PROFILE_SETTING);

    currentEncoderProfile = storedEncoderProfile;
    encoderProfile = storedEncoderProfile;

    storedKeypadProfile = EEPROM.read(KEYPAD_PROFILE_SETTING);

    currentKeypadProfile = storedKeypadProfile;
    keypadProfile = storedKeypadProfile;

    Wire.begin();
    oled.begin(&Adafruit128x64, I2C_ADDRESS);
    oled.setFont(menuFont);

    oledOn(" Macro-Keyboard\n   by Dentex");
    delay(STANDARD_MS);

    oled.clear();

    dsm.attachOled(oled);

    pinMode(TICK_LED, OUTPUT);
    ledOn();
    ledTimeout += ledDelay;

    strip.begin();
    strip.setBrightness(BRIGHTNESS);

    stripSetColor(currentEncoderProfile, currentKeypadProfile);
    stripSetColor();

#ifndef ENCODER_DEBUG
    {
        Consumer.begin();
    }
#endif

    Keyboard.begin();
}

void keySend(KeyboardKeycode key, bool useCtrl = false, bool useShift = false /*, bool useAlt = false */)
{
    if (!menuMode)
    {
        // Turn the LED on
        ledOn();

        // Send the key
        if (useCtrl)
        {
            Keyboard.press(KEY_LEFT_CTRL);
        }
        if (useShift)
        {
            Keyboard.press(KEY_LEFT_SHIFT);
        }
        /* if (useAlt)
        {
            Keyboard.press(KEY_LEFT_ALT);
        } */
        Keyboard.press(key);
        delay(5);
        Keyboard.releaseAll();
    }
}

void consumerSend(ConsumerKeycode key)
{
    ledOn();
    Consumer.write(key);
}

/*
  Turn the LED on and set the timeout
*/
void ledOn()
{
    if (!ledLight)
    {
        ledLight = true;
        digitalWrite(TICK_LED, HIGH);
    }
    ledTimeout = millis() + ledDelay;
}

void oledOn(const char *msg, bool isIgnoring)
{
    const char *oldMsg = "";
    if (!menuMode || isIgnoring)
    {
        if (!oledLight || msg != oldMsg)
        {
            oledLight = true;
            oled.clear();
            oled.setCursor(0, 0);
            oled.println(msg);
            oldMsg = msg;
        }
        oledTimeout = millis() + oledDelay;
    }

    if (showcaseMode)
    {
        oled.setCursor(0, 3);
        oled.println(msg);
        oldMsg = msg;
    }
}

/*
  Check the timeout and turn the LED off

  @param nowMillis cached millis()
*/
void ledOff(unsigned long nowMillis)
{
    if (ledLight and (nowMillis > ledTimeout))
    {
        ledLight = false;
        digitalWrite(TICK_LED, LOW);
    }
}

void oledOff(unsigned long nowMillis)
{
    if (oledLight and (nowMillis > oledTimeout) && !menuMode)
    {
        oledLight = false;
        oled.clear();
    }
}

int getPosition()
{
    return _positionExt;
}

/* void setPosition(int newPosition)
{
    _position = ((newPosition << 2) | (_position & 0x03));
    _positionExt = newPosition;
} */

void tick(void)
{
    int sig1 = digitalRead(encoderPinA);
    int sig2 = digitalRead(encoderPinB);
    int8_t thisState = sig1 | (sig2 << 1);
    if (_oldState != thisState)
    {
        _position += encoderStates[thisState | (_oldState << 2)];
         if ((thisState == 0) || (thisState == 3))
            _positionExt = _position >> 2;
        _oldState = thisState;
    }
}

void exitMenu()
{
    oledOn("EXIT\n Menu");
}

void drawMenu(int page)
{
    oled.clear();
    dsm.menuPage = page;

    switch (page)
    {
    case INITIAL_PAGE: // initial menu page
        dsm.currentLine = 0;
        dsm.maxLinesNum = 3;
        dsm.setLine(0, menuLineEnc + EncProfileNames[currentEncoderProfile]);
        dsm.setLine(1, menuLineKp + KpProfileNames[currentKeypadProfile]);
        dsm.setLine(2, menuLineTest);
        dsm.setLine(3, menuLineBack);
        break;

    case ENC_PROFILE_PAGE: // choose Encoder profile page
        dsm.currentLine = currentEncoderProfile;
        dsm.maxLinesNum = 3;
        dsm.setLine(0, MMEDIA_N);
        dsm.setLine(1, OLIVE_N);
        dsm.setLine(2, KRITA_N);
        dsm.setLine(3, menuLineBack);
        break;

    case KP_PROFILE_PAGE: // choose Keypad profile page
        dsm.currentLine = currentKeypadProfile;
        dsm.maxLinesNum = 3;
        dsm.setLine(0, APPS_N);
        dsm.setLine(1, OLIVE_N);
        dsm.setLine(2, KRITA_N);
        dsm.setLine(3, NAV_N);
        //dsm.setLine(4, menuLineBack);
        break;

    case TEST_KEYS_PAGE: // keypad's keys test page
        //TODO
        dsm.currentLine = 0;
        dsm.maxLinesNum = 0; //just one line [temp]
        dsm.setLine(0, menuLineBack);
        dsm.setLine(1, "");
        dsm.setLine(2, "");
        dsm.setLine(3, "*press any key*");
        break;
    }

    dsm.updateLines();
    delay(100);
    dsm.updateIndicators();
}

void handleEncoderClick(int line)
{
    switch (dsm.menuPage)
    {
    case INITIAL_PAGE: // initial menu page
        switch (line)
        {
        case ENC_PROFILE_PAGE: // Encoder profiles
        case KP_PROFILE_PAGE:  // Keypad profiles
            drawMenu(line);
            break;
        case TEST_KEYS_PAGE: // Test keys
            showcaseMode = true;
            drawMenu(line);
            break;

        case 3: // Go back (EXIT)
            quitMenu();
            break;
        }
        break;

    case ENC_PROFILE_PAGE: // choose Encoder profile menu page
        switch (line)
        {
        case MMEDIA_ENC:
        case OLIVE_ENC:
        case KRITA_ENC:
            assignEncoderProfile(line);
            drawMenu();
            break;
        case 3:
            drawMenu();
            break;
        }
        break;

    case KP_PROFILE_PAGE: // choose Keypad profile menu page
        switch (line)
        {
        case APPS_KP:
        case OLIVE_KP:
        case KRITA_KP:
        case NAV_KP:
            assignKeypadProfile(line);
            drawMenu();
            break;
        /* case 3:
            drawMenu();
            break;*/
        } 
        break;

    case TEST_KEYS_PAGE: // Test keys menu page
        showcaseMode = false;
        drawMenu(INITIAL_PAGE);
        break;
    }
}

void enterMenu()
{
    //Serial.println("Entering menu");
    oledOn("ENTER\n Menu", true);
    delay(STANDARD_MS);

    drawMenu();
}

void watchButtons(unsigned long nowMillis)
{
    // read the state of the switch into a local variable:
    int menuReading = digitalRead(MENU_BUTTON);
    int touchSensorReading = digitalRead(TOUCH_SENSOR);

    // check to see if you just pressed the button
    // (i.e. the input went from LOW to HIGH), and you've waited long enough
    // since the last press to ignore any noise:

    // If the switch changed, due to noise or pressing:
    if ((menuReading != lastMenuButtonState) || (touchSensorReading != lastTouchSensorState))
    {
        // reset the debouncing timer
        lastDebounceTime = millis();
    }

    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        // whatever the menu button reading is at, it's been there for longer than the debounce
        // delay, so take it as the actual current state:

        // if the button state has changed:
        if (menuReading != menuButtonState)
        {
            menuButtonState = menuReading;

            // only toggle the LED if the new button state is HIGH
            if (menuButtonState == HIGH)
            {
                // start button press code
                ledOn();
                menuMode = !menuMode;
                // end button press code

                if (menuMode)
                {
                    enterMenu();
                }
                else
                {
                    if (showcaseMode)
                    {
                        showcaseMode = false;
                    }

                    exitMenu();
                }

                // delay before considering a 'long-press'
                //delay(STANDARD_MS);

                /* while (digitalRead(MENU_BUTTON) == HIGH)
                {
                    //this section runs every 250ms
                    //while keeping MENU_BUTTON pressed

                    // delay between subsequent keys during 'long-press' phase
                    delay(250);
                } */
            }
        }

        // if the touch sensor state has changed:
        if (touchSensorReading != touchSensorState)
        {
            touchSensorState = touchSensorReading;

            if (touchSensorState == HIGH)
            {
                //ledOn();
                //delay(STANDARD_MS);
                stripSetColor(currentEncoderProfile, currentKeypadProfile);
            }
            else
            {
                stripSetColor();
            }
        }
    }
    ledOff(nowMillis);

    lastMenuButtonState = menuReading;
    lastTouchSensorState = touchSensorReading;
}

void normalOps(bool isMenuMode)
{
    static int pos = 0;
    tick();
    int newPos = getPosition();
    if (pos != newPos)
    {
        if (newPos < pos)
        {
            if (isMenuMode)
            {
                dsm.menuUp();
            }
            else
            {
                if (!btnPressed)
                {
#ifndef ENCODER_DEBUG
                    {
                        switch (currentEncoderProfile)
                        {
                        case MMEDIA_ENC:
                            consumerSend(LEFT_ACTION_MM);
                            break;
                        case OLIVE_ENC:
                            keySend(LEFT_ACTION_O);
                            break;
                        case KRITA_ENC:
                            keySend(LEFT_ACTION_K, true);
                            break;
                        default:
                            // statements
                            break;
                        }
                    }
#else
                    {
                        keySend(LEFT_ACTION_D, false, true); // SHIFT for uppercase
                    }
#endif
                }
                else
                {
                    pressedRotary = true;
#ifndef ENCODER_DEBUG
                    {
                        switch (currentEncoderProfile)
                        {
                        case MMEDIA_ENC:
                            consumerSend(LEFT_ACTION_MODE1_MM);
                            break;
                        case OLIVE_ENC:
                            keySend(LEFT_ACTION_MODE1_O);
                            break;
                        case KRITA_ENC:
                            keySend(LEFT_ACTION_MODE1_K, true);
                            break;
                        default:
                            // statements
                            break;
                        }
                    }
#else
                    {
                        keySend(LEFT_ACTION_MODE1_D);
                    }
#endif
                }
            }
        }
        else if (newPos > pos)
        {
            if (isMenuMode)
            {
                dsm.menuDown();
            }
            else
            {
                if (!btnPressed)
                {
#ifndef ENCODER_DEBUG
                    {
                        switch (currentEncoderProfile)
                        {
                        case MMEDIA_ENC:
                            consumerSend(RIGHT_ACTION_MM);
                            break;
                        case OLIVE_ENC:
                            keySend(RIGHT_ACTION_O);
                            break;
                        case KRITA_ENC:
                            keySend(RIGHT_ACTION_K, true);
                            break;
                        default:
                            // statements
                            break;
                        }
                    }
#else
                    {
                        keySend(RIGHT_ACTION_D, false, true); // SHIFT for uppercase
                    }
#endif
                }
                else
                {
                    pressedRotary = true;
#ifndef ENCODER_DEBUG
                    {
                        switch (currentEncoderProfile)
                        {
                        case MMEDIA_ENC:
                            consumerSend(RIGHT_ACTION_MODE1_MM);
                            break;
                        case OLIVE_ENC:
                            keySend(RIGHT_ACTION_MODE1_O);
                            break;
                        case KRITA_ENC:
                            keySend(RIGHT_ACTION_MODE1_K, true);
                            break;
                        default:
                            // statements
                            break;
                        }
                    }
#else
                    {
                        keySend(RIGHT_ACTION_MODE1_D);
                    }
#endif
                }
            }
        }
        pos = newPos;
    }

    int encReading = digitalRead(encoderButton);
    if (encReading != lastEncButtonState)
    {
        lastDebounceTime = millis();
    }
    if ((millis() - lastDebounceTime) > debounceDelay)
    {
        if (encReading != encButtonState)
        {
            encButtonState = encReading;
            if (encButtonState == HIGH)
            {
                btnPressed = true;
            }
            else if (encButtonState == LOW)
            {
                btnReleased = true;
            }
        }
    }
    lastEncButtonState = encReading;

    if (btnPressed == true && btnReleased == true && pressedRotary == false)
    {
        if (isMenuMode)
        {
            handleEncoderClick(dsm.currentLine);
        }
        else
        {
#ifndef ENCODER_DEBUG
            {
                switch (currentEncoderProfile)
                {
                case MMEDIA_ENC:
                    consumerSend(BUTTON_ACTION_MM);
                    break;
                case OLIVE_ENC:
                    keySend(BUTTON_ACTION_O);
                    break;
                case KRITA_ENC:
                    keySend(BUTTON_ACTION_K, true);
                    break;
                default:
                    // statements
                    break;
                }
            }
#else
            {
                keySend(BUTTON_ACTION_D, false, true); // SHIFT for uppercase
            }
#endif
        }

        btnPressed = false;
        btnReleased = false;
    }
    else if (btnPressed == true && btnReleased == true && pressedRotary == true)
    {
        //RELEASED_AFTER_HELD_ROTATION
        btnPressed = false;
        btnReleased = false;
        pressedRotary = false;
    }
}

void watchKeypad()
{
    char key = keypad.getKey();

    if (key)
    {
        //Serial.println(key);
        if (currentKeypadProfile == OLIVE_KP)
        {
            switch (key)
            {
            case '0':
                keySend(KB[20].key, KB[20].ctrl, KB[20].shift); // Pointer
                oledOn(KB[20].descr);
                break;
            case '1':
                keySend(KB[21].key, KB[21].ctrl, KB[21].shift); // Razor
                oledOn(KB[21].descr);
                break;
            case '2':
                keySend(KB[22].key, KB[22].ctrl, KB[22].shift); // Zoom in
                oledOn(KB[22].descr);
                break;
            case '3':
                keySend(KB[23].key, KB[23].ctrl, KB[23].shift); // Zoom out
                oledOn(KB[23].descr);
                break;
            case '4':
                keySend(KB[24].key, KB[24].ctrl, KB[24].shift); // Snapping
                oledOn(KB[24].descr);
                break;
            case '5':
                keySend(KB[25].key, KB[25].ctrl, KB[25].shift); // Export
                oledOn(KB[25].descr);
                break;
            case '6':
                keySend(KB[26].key, KB[26].ctrl, KB[26].shift); // In point
                oledOn(KB[26].descr);
                break;
            case '7':
                keySend(KB[27].key, KB[27].ctrl, KB[27].shift); // Out point
                oledOn(KB[27].descr);
                break;
            }
        }
        else if (currentKeypadProfile == NAV_KP)
        {
            switch (key)
            {
            case '0':
                keySend(KB[12].key, KB[12].ctrl, KB[12].shift); // TAB
                oledOn(KB[12].descr);
                break;
            case '1':
                keySend(KB[13].key, KB[13].ctrl, KB[13].shift); // ESC
                oledOn(KB[13].descr);
                break;
            case '2':
                keySend(KB[14].key, KB[14].ctrl, KB[14].shift); // UP ARROW
                oledOn(KB[14].descr);
                break;
            case '3':
                keySend(KB[15].key, KB[15].ctrl, KB[15].shift); // ENTER
                oledOn(KB[15].descr);
                break;
            case '4':
                keySend(KB[16].key, KB[16].ctrl, KB[16].shift); // SHIFT+TAB
                oledOn(KB[16].descr);
                break;
            case '5':
                keySend(KB[17].key, KB[17].ctrl, KB[17].shift); // LEFT ARROW
                oledOn(KB[17].descr);
                break;
            case '6':
                keySend(KB[18].key, KB[18].ctrl, KB[18].shift); // DOWN ARROW
                oledOn(KB[18].descr);
                break;
            case '7':
                keySend(KB[19].key, KB[19].ctrl, KB[19].shift); // RIGHT ARROW
                oledOn(KB[19].descr);
                break;
            }
        }
        else // APPS_KP and KRITA_KP profiles are mixed here
        {
            // Profile independent keys, on first row
            switch (key)
            {
            case '0':
                keySend(KB[0].key, KB[0].ctrl, KB[0].shift); // Undo
                oledOn(KB[0].descr);
                break;
            case '1':
                keySend(KB[1].key, KB[1].ctrl, KB[1].shift); // Redo
                oledOn(KB[1].descr);
                break;
            case '2':
                keySend(KB[2].key, KB[2].ctrl, KB[2].shift); // Copy
                oledOn(KB[2].descr);
                break;
            case '3':
                keySend(KB[3].key, KB[3].ctrl, KB[3].shift); // Paste
                oledOn(KB[3].descr);
                break;
            }

            // Profile dependent keys, on second row
            switch (currentKeypadProfile)
            {
            case APPS_KP:
                switch (key)
                {
                case '4':
                    keySend(KB[4].key, KB[4].ctrl, KB[4].shift);
                    oledOn(KB[4].descr);
                    break;
                case '5':
                    keySend(KB[5].key, KB[5].ctrl, KB[5].shift);
                    oledOn(KB[5].descr);
                    break;
                case '6':
                    keySend(KB[6].key, KB[6].ctrl, KB[6].shift);
                    oledOn(KB[6].descr);
                    break;
                case '7':
                    keySend(KB[7].key, KB[7].ctrl, KB[7].shift);
                    oledOn(KB[7].descr);
                    break;
                }
                break;
            case KRITA_KP:
                switch (key)
                {
                case '4':
                    keySend(KB[8].key, KB[8].ctrl, KB[8].shift);
                    oledOn(KB[8].descr);
                    break;
                case '5':
                    keySend(KB[9].key, KB[9].ctrl, KB[9].shift);
                    oledOn(KB[9].descr);
                    break;
                case '6':
                    keySend(KB[10].key, KB[10].ctrl, KB[10].shift);
                    oledOn(KB[10].descr);

                    /* Keyboard.press(KEY_LEFT_CTRL); // Press CTRL for 5 seconds (for the color picker)
                    delay(5000);
                    Keyboard.release(KEY_LEFT_CTRL); */
                    break;
                case '7':
                    keySend(KB[11].key, KB[11].ctrl, KB[11].shift);
                    oledOn(KB[11].descr);

                    /* Keyboard.press(KEY_TAB);
                    delay(25);
                    Keyboard.release(KEY_TAB); */
                    break;
                }
                break;
            }
        }
    }
}

void loop()
{
    // Keep the time
    unsigned long nowMillis = millis();

    watchButtons(nowMillis);

    watchKeypad();

    normalOps(menuMode);

    // Turn LED and OLED off, if timed out
    ledOff(nowMillis);
    oledOff(nowMillis);
}