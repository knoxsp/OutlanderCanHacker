// Paint example specifically for the TFTLCD breakout board.
// If using the Arduino shield, use the tftpaint_shield.pde sketch instead!
// DOES NOT CURRENTLY WORK ON ARDUINO LEONARDO

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library
#include <TouchScreen.h>

#if defined(__SAM3X8E__)
    #undef __FlashStringHelper::F(string_literal)
    #define F(string_literal) string_literal
#endif

// When using the BREAKOUT BOARD only, use these 8 data lines to the LCD:
// For the Arduino Uno, Duemilanove, Diecimila, etc.:
//   D0 connects to digital pin 8  (Notice these are
//   D1 connects to digital pin 9   NOT in order!)
//   D2 connects to digital pin 2
//   D3 connects to digital pin 3
//   D4 connects to digital pin 4
//   D5 connects to digital pin 5
//   D6 connects to digital pin 6
//   D7 connects to digital pin 7

// For the Arduino Mega, use digital pins 22 through 29
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 22
//   D1 connects to digital pin 23
//   D2 connects to digital pin 24
//   D3 connects to digital pin 25
//   D4 connects to digital pin 26
//   D5 connects to digital pin 27
//   D6 connects to digital pin 28
//   D7 connects to digital pin 29

// For the Arduino Due, use digital pins 33 through 40
// (on the 2-row header at the end of the board).
//   D0 connects to digital pin 33
//   D1 connects to digital pin 34
//   D2 connects to digital pin 35
//   D3 connects to digital pin 36
//   D4 connects to digital pin 37
//   D5 connects to digital pin 38
//   D6 connects to digital pin 39
//   D7 connects to digital pin 40

#define YP A3  // must be an analog pin, use "An" notation!
#define XM A2  // must be an analog pin, use "An" notation!
#define YM 9   // can be a digital pin
#define XP 8   // can be a digital pin

#define TS_MINX 130
#define TS_MINY 90
#define TS_MAXX 890
#define TS_MAXY 890

// For better pressure precision, we need to know the resistance
// between X+ and X- Use any multimeter to read it
// For the one we're using, its 300 ohms across the X plate
TouchScreen ts = TouchScreen(XP, YP, XM, YM, 300);

#define LCD_CS A3
#define LCD_CD A2
#define LCD_WR A1
#define LCD_RD A0
// optional
#define LCD_RESET A4

// Assign human-readable names to some common 16-bit color values:
#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF

Adafruit_TFTLCD tft(LCD_CS, LCD_CD, LCD_WR, LCD_RD, LCD_RESET);

#define BOXWIDTH 160
#define BOXHEIGHT 120

int currentscreen = 0;
bool is_charging = false;
bool hv_on = false;

void setup(void) {
  Serial.begin(9600);
  Serial.println(F("Paint!"));

  tft.reset();

  uint16_t identifier = tft.readID();

  if(identifier == 0x9325) {
    Serial.println(F("Found ILI9325 LCD driver"));
  } else if(identifier == 0x9328) {
    Serial.println(F("Found ILI9328 LCD driver"));
  } else if(identifier == 0x7575) {
    Serial.println(F("Found HX8347G LCD driver"));
  } else if(identifier == 0x9341) {
    Serial.println(F("Found ILI9341 LCD driver"));
  } else if(identifier == 0x8357) {
    Serial.println(F("Found HX8357D LCD driver"));
  } else {
    Serial.print(F("Unknown LCD driver chip: "));
    Serial.println(identifier, HEX);
    Serial.println(F("If using the Adafruit 2.8\" TFT Arduino shield, the line:"));
    Serial.println(F("  #define USE_ADAFRUIT_SHIELD_PINOUT"));
    Serial.println(F("should appear in the library header (Adafruit_TFT.h)."));
    Serial.println(F("If using the breakout board, it should NOT be #defined!"));
    Serial.println(F("Also if using the breakout, double-check that all wiring"));
    Serial.println(F("matches the tutorial."));
    return;
  }

  tft.begin(identifier);

  tft.setRotation(3);

  currentscreen = 0;

  pinMode(13, OUTPUT);

  drawScreenOne();
}

#define MINPRESSURE 10
#define MAXPRESSURE 1000

void makeBlankScreen(){
  tft.fillScreen(BLACK);
  tft.setTextColor(WHITE);
  tft.setTextSize(3);
}

void drawScreenOne(){
  makeBlankScreen();
  Serial.println("Drawing the text");

  drawPowerButton();
  drawChargeButton();
}


void drawPowerButton(){
  /*draw the button which controlls the high voltage junctio box*/
  //Draw 'power button'
  tft.fillRect(0, 0, BOXWIDTH, BOXHEIGHT, WHITE);
  tft.setCursor(120, 160);
}

void drawChargeButton(){
  /*draw the button that allows the user to start and stop charging
  This will only work if HV is activated with the other button.
  */
  //Draw 'power button'
  //draw charge button
  tft.fillRect(160, 0, BOXWIDTH, BOXHEIGHT, RED);
  tft.setCursor(120, 160);
}

void changeHVState(){
    /*if high-voltage is on, turn it off, and if off, turn it on*/
    if (hv_on == true){
      deactivateHV();
    }else{
      activateHV();
    }
}

void activateHV(){
  /*turn on the High Voltage bus*/
  Serial.println("Activating Precharge");
  //close the negative relay

  //close the precharge relay

  //wait until there is max voltage on the bus

  //close the positive relay
  hv_on = true;
  Serial.println("Activating Precharge");
}

void deactivateHV(){
  Serial.println("Deactivating HV");
  //open the negative relay

  //open the precharge relay

  //wait until there is max voltage on the bus

  //open the positive relay
  hv_on = false;
}

void changeChargingState(){
  //If the charger is charging, stop. Otherwise start
  Serial.println("Changing charging state");
  if (is_charging == true){
    //stop sending charging messages
    //close precharge relay??
    is_charging = false;
  }else{
    //send charging messages
    is_charging = true;
  }
}


void loop()
{
  digitalWrite(13, HIGH);
  TSPoint p = ts.getPoint();
  digitalWrite(13, LOW);

  // if sharing pins, you'll need to fix the directions of the touchscreen pins
  //pinMode(XP, OUTPUT);
  pinMode(XM, OUTPUT);
  pinMode(YP, OUTPUT);
  //pinMode(YM, OUTPUT);

  // we have some minimum pressure we consider 'valid'}
  // pressure of 0 means no pressing!

  if (p.z > MINPRESSURE && p.z < MAXPRESSURE) {
    /*
    Serial.print("X = "); Serial.print(p.x);
    Serial.print("\tY = "); Serial.print(p.y);
    Serial.print("\tPressure = "); Serial.println(p.z);
    */

    Serial.print("("); Serial.print(p.x);
    Serial.print(", "); Serial.print(p.y);
    Serial.print(") --> ");


    // scale from 0->1023 to tft.width
    //TODO When the TouchScreen calibration is off, change the TS_MIN* around
    //as appropriate.

    int x = map(p.y, 84, 876, 0, 320);
    int y = map(p.x, 150, 915, 0, 240);

    //p.x = map(p.y, TS_MINY, TS_MAXY, 0, tft.width());
    //p.y = map(p.x, TS_MINX, TS_MAXX, 0, tft.height());

    Serial.print("("); Serial.print(x);
    Serial.print(", ");Serial.print(y);
    Serial.println(")\n\n");
    if (y > 0 && y < BOXHEIGHT && x > 0 && x < BOXWIDTH){
      changeHVState();//Power button -- turn on and off HV
    }else if (y > 0 && y < BOXHEIGHT && x > 160 && x < (160 + BOXWIDTH)){
        changeChargingState(); // start or stop charging
    }
  }
}
