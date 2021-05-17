#ifndef MY_BUTTON_H
#define MY_BUTTON_H
#include <Arduino.h>
#include <TouchScreen.h>
//#include <Adafruit_GFX.h>    // Core graphics library
//#include <Adafruit_TFTLCD.h> // Hardware-specific library

class Button {

  private:

  public:
    int width;
    int height;
    int x; // top-left
    int y; // top-left
    int color;

    Button(int x, int y, int width, int height, int color)
    {
      this->width = width;
      this->height = height;
      this->x = x;
      this->y = y;
      this->color = color;
    }

    void draw();
    bool is_pressed(TSPoint & p);
    void activate();
};
#endif
