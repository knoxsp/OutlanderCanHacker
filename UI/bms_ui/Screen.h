#ifndef MY_BUTTON_H
#define MY_BUTTON_H
#include <Arduino.h>
#include <TouchScreen.h>
#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_TFTLCD.h> // Hardware-specific library

#define	BLACK   0x0000
#define	BLUE    0x001F
#define	RED     0xF800
#define	GREEN   0x07E0
#define CYAN    0x07FF
#define MAGENTA 0xF81F
#define YELLOW  0xFFE0
#define WHITE   0xFFFF


#define BOXSIZE 50
#define PENRADIUS 3

class Button {

  private:

  public:
    String name;
    int width;
    int height;
    int x; // top-left
    int y; // top-left
    int color;

    Button(){

    }

    Button(String name, int x, int y, int width, int height, int color)
    {
      Serial.print("Creating Button"); Serial.println(name);
      this->name = name;
      this->width = width;
      this->height = height;
      this->x = x;
      this->y = y;
      this->color = color;
    }

    void draw(Adafruit_TFTLCD& tft);
    bool is_pressed(TSPoint & p);
    void activate();
};

class Page {
  public:
    virtual void initialize();
    virtual void render();
    virtual void makeBlankScreen();
    virtual void drawButtons();
    virtual String getName();
    virtual int getNumButtons();
    virtual Button** getButtons();

};

class HomePage : public Page{
private:
  String name {"Default Home Page Name..."};
  int num_buttons {4};
  Button* buttons[4];
  Adafruit_TFTLCD* tft;
  public:

    HomePage()
    {
    }
    HomePage(String name, Adafruit_TFTLCD& tft)
    {
      this->name = name;
      this->tft = &tft;
      Serial.println("HomePage");
    }

    void initialize(){
        Serial.println("INITIALIZING");
        this->buttons[0] = new Button("SOC", 10, 10, 120, 100, WHITE);
        this->buttons[1] = new Button("Temps", 180, 10, 120, 100, WHITE);
        this->buttons[2] = new Button("Cells", 10, 130, 120, 100, WHITE);
        this->buttons[3] = new Button("??", 180, 130, 120, 100, WHITE);
    }
    void render();
    void makeBlankScreen(){
      Serial.println("Making blank screen");
    //  this->tft->fillScreen(BLUE);
  //    this->tft->setTextColor(WHITE);
  //    this->tft->setTextSize(3);
    };
    void drawButtons();
    String getName();
    int getNumButtons();
    Button** getButtons();
};

class NotHomePage : public HomePage{
  private:
    String name {"Default HomePage Name"};
    int num_buttons {1};
    Button* buttons[1];
    Adafruit_TFTLCD* tft;

  public:
    NotHomePage():HomePage(){
    }
    NotHomePage(String name, Adafruit_TFTLCD& tft){
      this->name = name;
      this->tft = &tft;
      Serial.println("HomePage");
    }
    void initialize(){
      Serial.println("INITIALIZING NOT BASE CLASS");
      this->buttons[0] = new Button("Test", 50, 50, 50, 50, WHITE);
      Serial.print("Button Name: ");
      Serial.println(this->buttons[0]->name);
    }
    void render();
    void makeBlankScreen(){
      Serial.println("test");
      HomePage::makeBlankScreen();
      Serial.println("test1");
    }
    void drawButtons();
    String getName();
    int getNumButtons();
    Button** getButtons();
};


class Base{
  private:
    String name {"Base"};
  public:
    Base(){

    }
    Base(String name){
      this->name = name;
      Serial.println(name);
    }
    void printName(){
      Serial.println(this->name);
    }
};

class Derived : public Base{
  private:
    String name {"Derived"};
  public:
    Derived():Base(){
    }
    Derived(String name):Base(name){
    }
    void printName(){
      Base::printName();
      Serial.print("DERIVED");
      Serial.println(this->name);
    }
};


#endif
