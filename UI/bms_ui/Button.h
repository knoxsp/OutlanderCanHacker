#ifndef MY_BUTTON_H
#define MY_BUTTON_H
#include <Arduino.h>
class Button {
  
  private:
    int width;
    int height;
    int x; // top-left
    int y; // top-left
    
  public:
    Button();
    void render();
    void update();
};
#endif
