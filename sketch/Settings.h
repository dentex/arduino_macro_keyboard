#include <HID-Project.h>
#include <HID-Settings.h>

// Pin settings
#define encoderPinA PIN_A1 // DT -- (volume UP)
#define encoderPinB PIN_A0 // CLK - (volume DOWN)
#define encoderButton PIN_A2
#define TICK_LED 16 //8
#define WS2812_LED 10
#define MENU_BUTTON 8  //9
#define TOUCH_SENSOR 9 //16

// Tick LED stuff
bool ledLight = false;           // Status flag for the LED (on or off)
unsigned long ledTimeout = 0UL;  // The time when the LED will turn off
unsigned long ledDelay = 20UL;   // Time delay after which the LED turns off
unsigned long STANDARD_MS = 500; // A generic delay

// Tick OLED screen message stuff
bool oledLight = false;          // Status flag for the OLED screen (on or off)
unsigned long oledTimeout = 0UL; // The time when the OLED screen will turn off
unsigned long oledDelay = 750UL; // Time delay after which the OLED screen turns off

// WS2812 LEDs stuff
#define DIN_PIN 10
#define LEDS_NUM 3
#define BRIGHTNESS 64

//Keypad stuff
#define ROWS 2
#define COLS 4

char keys[ROWS][COLS] = {
    {'0', '1', '2', '3'},
    {'4', '5', '6', '7'}};

// EEPROMwl setup
// #define EEPROM_LAYOUT_VERSION 0
// #define AMOUNT_OF_INDEXES 1
#define ENCODER_PROFILE_SETTING 10
#define KEYPAD_PROFILE_SETTING 11

// Menu and profiles
String menuLineEnc = "Encoder: ";
String menuLineKp = "Keypad: ";
String menuLineTest = "Test Keys";
String menuLineBack = "<=";

enum MenuPages
{
  INITIAL_PAGE = -1,
  ENC_PROFILE_PAGE = 0,
  KP_PROFILE_PAGE = 1,
  TEST_KEYS_PAGE = 2
};

enum EncProfilesNum
{
  MMEDIA_ENC = 0,
  OLIVE_ENC = 1,
  KRITA_ENC = 2
};

enum KpProfilesNum
{
  APPS_KP = 0,
  OLIVE_KP = 1,
  KRITA_KP = 2,
  NAV_KP = 3
};

String MMEDIA_N = "MMedia";
String OLIVE_N = "Olive";
String KRITA_N = "Krita";
String APPS_N = "Apps";
String NAV_N =  "Nav";

String EncProfileNames[] = { MMEDIA_N, OLIVE_N, KRITA_N };
String KpProfileNames[] = { APPS_N, OLIVE_N, KRITA_N, NAV_N };


// ENCODER profiles:
//========================================================================================
// MultiMedia
#define LEFT_ACTION_MM MEDIA_VOL_DOWN
#define LEFT_ACTION_MODE1_MM MEDIA_PREV

#define RIGHT_ACTION_MM MEDIA_VOL_UP
#define RIGHT_ACTION_MODE1_MM MEDIA_NEXT

#define BUTTON_ACTION_MM MEDIA_PLAY_PAUSE

// Olive
#define LEFT_ACTION_O KEY_LEFT_ARROW
#define LEFT_ACTION_MODE1_O KEY_UP_ARROW

#define RIGHT_ACTION_O KEY_RIGHT_ARROW
#define RIGHT_ACTION_MODE1_O KEY_DOWN_ARROW

#define BUTTON_ACTION_O KEY_SPACE

// Krita
// (Zoom +/- and reset
//  increase/decrease brush via CTRL+8 and CTRL+9 custom shortcuts)
#define ctrlKey KEY_LEFT_CTRL;

#define LEFT_ACTION_K KEY_SLASH        // '-' is '/' on the US keyboard
#define LEFT_ACTION_MODE1_K KEY_8

#define RIGHT_ACTION_K KEY_RIGHT_BRACE  // '+' is ']' on the US keyboard
#define RIGHT_ACTION_MODE1_K KEY_9

#define BUTTON_ACTION_K KEY_0

// DEBUG
#define LEFT_ACTION_D KEY_L
#define LEFT_ACTION_MODE1_D KEY_0

#define RIGHT_ACTION_D KEY_R
#define RIGHT_ACTION_MODE1_D KEY_1

#define BUTTON_ACTION_D KEY_B
//========================================================================================


// Keypad profiles
struct KEYPAD_BUTTON
{
  const KeyboardKeycode key;
  const bool ctrl;
  const bool shift;
  const char *descr;
} KB[] = {
    //      16 chars -->             |1234567890123456|
    //KEY   CTRL  SHIFT              |-----DESCR------|
    //[0-3]
    {KEY_Z, true, false,              "     UNDO     "},    // Button 0 "Apps & Krita" profiles
    {KEY_Z, true, true,               "     REDO     "},    // Button 1 "Apps & Krita" profiles
    {KEY_C, true, false,              "     COPY     "},    // Button 2 "Apps & Krita" profiles
    {KEY_V, true, false,              "     PASTE    "},    // Button 3 "Apps & Krita" profiles
    //[4-7]
    {KEY_F14, true, false,            "   AUDIO SINK "},    // Button 4 "Apps" profile
    {KEY_F15, true, false,            "  VLC+TORRENTS"},    // Button 5 "Apps" profile
    {KEY_A, true, false,              "   SELECT ALL "},    // Button 6 "Apps" profile
    {KEY_X, true, false,              "      CUT     "},    // Button 7 "Apps" profile
    //[8-11]
    {KEY_I, false, false,             "  Incr Opacity"},    // Button 4 "Krita" profile
    {KEY_O, false, false,             "  Decr Opacity"},    // Button 5 "Krita" profile
    {KEY_L, false, false,             "  Light Brush "},    // Button 6 "Krita" profile
    {KEY_K, false, false,             "   Dark Brush "},    // Button 7 "Krita" profile
    //[12-19]
    {KEY_TAB, false, false,           "      TAB     "},    // Button 0 "NAV" profile
    {KEY_ESC, false, false,           "      ESC     "},    // Button 1 "NAV" profile
    {KEY_UP_ARROW, false, false,      "    UP ARROW  "},    // Button 2 "NAV" profile
    {KEY_ENTER, false, false,         "     ENTER    "},    // Button 3 "NAV" profile
    {KEY_TAB, false, true,            "    SHIFT TAB "},    // Button 4 "NAV" profile
    {KEY_LEFT_ARROW, false, false,    "   LEFT ARROW "},    // Button 5 "NAV" profile
    {KEY_DOWN_ARROW, false, false,    "   DOWN ARROW "},    // Button 6 "NAV" profile
    {KEY_RIGHT_ARROW, false, false,   "   RIGHT ARROW"},    // Button 7 "NAV" profile
    //[20-27]
    {KEY_V, false, false,              "    Pointer   "},   // Button 0 "Olive" profile
    {KEY_C, false, false,             "     Razor    "},   // Button 1 "Olive" profile
    {KEY_SLASH, false, false,         "   Zooom Out  "},   // Button 3 "Olive" profile // '-' is '/' on the US keyboard
    {KEY_0, false, true,              "    Zooom In  "},   // Button 2 "Olive" profile // '=' is ')', done with SHIFT+0, on the US keyboard
    {KEY_S, false, false,             "    Snapping  "},   // Button 6 "Olive" profile
    {KEY_M, true, false,              "    Export... "},   // Button 7 "Olive" profile
    {KEY_I, false, false,             "    In Point  "},   // Button 4 "Olive" profile
    {KEY_O, false, false,             "   Out Point  "},   // Button 5 "Olive" profile
};

/* List of Supported Fonts
  Arial14,
  Arial_bold_14,
  Callibri11,
  Callibri11_bold,
  Callibri11_italic,
  Callibri15,
  Corsiva_12,
  fixed_bold10x15,
  font5x7,
  font8x8,
  Iain5x7,
  lcd5x7,
  Stang5x7,
  System5x7,
  TimesNewRoman16,
  TimesNewRoman16_bold,
  TimesNewRoman16_italic,
  utf8font10x16,
  Verdana12,
  Verdana12_bold,
  Verdana12_italic,
  X11fixed7x14,
  X11fixed7x14B,
  ZevvPeep8x16
*/

// FONT definition
#define menuFont ZevvPeep8x16
#define fontW 8
#define fontH 16

// OLED stuff
constexpr int OLED_SDA = 2;
constexpr int OLED_SDC = 3;
#define I2C_ADDRESS 0x3C

// debug mode: for outputting keyboards keys only
//#define ENCODER_DEBUG;
