#include "Screen.h"

void Button::draw(Adafruit_TFTLCD& tft) {
  Serial.println("Drawing button");
  tft.fillRoundRect(this->x, this->y, this->width, this->height, 5, this->color);
}

bool Button::is_pressed(TSPoint & p) {
  if (p.y > this->y & p.y < (this->y+this->height) & p.x > this->x && p.x < (this->x+this->width)) {
    /* run the callback*/
    Serial.println("Pressed");
    return true;
  }else{
    return false;
  }
}

void Button::activate(){
  //  this->page->screen.makeBlankScreen();
}
