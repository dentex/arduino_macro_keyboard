# Arduino Macro Keyboard

DIY *Macro Keyboard* using an Arduino Pro Micro, 3D printed parts and some other hardware modules.


### Arduino sketch

Find it at [./sketch/pro-micro_dentex_macro-keyboard.ino](/sketch/pro-micro_dentex_macro-keyboard.ino)
Libraries used:
- HID-Project
- EEPROM
- Adafruit_NeoPixel
- SSD1306Ascii
- Keypad


### 3D printed parts

Printable STL files are available in the [relative folder](/STL). The [./FreeCAD](/FreeCAD) folder contains the 3D project (the main file to be loaded in FreeCAD is `#_MACRO-KEYBOARD_v12.FCStd`)


### Hardware

- 1 Arduino Micro
- 8 Cherry MX switches (or similar/compatible, i.e.: https://www.aliexpress.com/item/4001111706889.html)
- 8 Keycaps (4xR1 and 4xR2: https://www.aliexpress.com/item/33056163268.html)
- 1 Rotary Encoder (I used a KY_040 module, it should be this one: https://www.aliexpress.com/item/32726570531.html)
- 6 short M3 hexagonal screws
- 6 M3X4.6X4.0 Brass Insert Nuts (i.e.: https://www.aliexpress.com/item/4000688990203.html)
- 1 6x6x5mm tactile mini switch (i.e.: https://www.aliexpress.com/item/1005003536610720.html)
- 1 3mm LED
- 1 Addressable LED module (i.e.: https://www.aliexpress.com/item/32851699524.html)
- 1 I2C OLED Display Module (i.e.: https://fr.aliexpress.com/item/32584349265.html type 0.96 white)
- 1 TTP223 Capacitive Touch Switch Module(i.e.: https://www.aliexpress.com/item/32869836574.html)


### Wiring

A partial connection diagram is present into the [./images](/images) folder. It covers the keyboard and the rotary encoder. For the other connections, rely on the Arduino sketch and `./images/pro-micro-pinout.jpg`.
