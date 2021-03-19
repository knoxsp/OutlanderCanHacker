#include "Button.h"
#include <TouchScreen.h>
Button::Button(TouchScreen touchscreen) {
  this->touchscreen = touchscreen;
  render();
}
void Button::render() {
  update();
}

byte Button::update() {
  
}
