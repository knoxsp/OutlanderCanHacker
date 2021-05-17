#include "Screen.h"

Button** HomePage::getButtons(){
  return this->buttons;
}

int HomePage::getNumButtons(){
  return this->num_buttons;
}

String HomePage::getName(){
  return this->name;
}

void HomePage::render() {
  Serial.print("Rendering Page ");
  Serial.println(this->name);
  this->makeBlankScreen();
  this->drawButtons();
}

/*void HomePage::makeBlankScreen(){
  Serial.println("Making blank screen");
  this->tft->fillScreen(BLUE);
  this->tft->setTextColor(WHITE);
  this->tft->setTextSize(3);
}*/

void HomePage::drawButtons(){
  for (int i=0; i<this->num_buttons; i++){

    //buttons[i]->draw(this->tft);
    Button* b = this->buttons[i];
    if (!b){
      Serial.println("Button not found. Returning.");
      return;
    }
    Serial.print("Drawing Button ");
    Serial.println(b->name);
    //the '5' is the border radius
    this->tft->fillRoundRect(b->x, b->y, b->width, b->height, 5, b->color);
  }
}

int NotHomePage::getNumButtons(){
  //return this->num_buttons;
}
String NotHomePage::getName(){
  //return this->name;
}
/*void NotHomePage::makeBlankScreen(){
  HomePage::makeBlankScreen();
/*  Serial.println("Making blank screen");
  this->tft->fillScreen(GREEN);
  this->tft->setTextColor(WHITE);
  this->tft->setTextSize(3);*/


Button** NotHomePage::getButtons(){
}

void NotHomePage::render() {
  Serial.print("Rendering Page ");
  Serial.println(this->name);
  this->makeBlankScreen();
  this->drawButtons();
}

void NotHomePage::drawButtons(){
  Serial.println("drawing buttons");
  for (int i=0; i<this->num_buttons; i++){

    //buttons[i]->draw(this->tft);
    Button* b = this->buttons[i];
    if (!b){
      Serial.println("Button not found. Returning.");
      return;
    }
    Serial.print("Drawing Button ");
    Serial.println(b->name);
    //the '5' is the border radius
    this->tft->fillRoundRect(b->x, b->y, b->width, b->height, 5, b->color);
  }
}
