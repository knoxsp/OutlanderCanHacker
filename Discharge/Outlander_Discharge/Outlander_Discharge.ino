  // CAN Receive Example
//

#include <mcp_can.h>
#include <SPI.h>

/*Show data on an LED display*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

long unsigned int rxId;
const int num_modules = 10; 
short boardpres = 0;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[64];                        // Array to store serial string
byte mes[8] = {0, 0, 0, 4, 3, 0, 0, 0};

//The number of milliseconds before the reading from a cell expires, and
//gets set to -1
int CELL_EXPIRY = 1000;

int voltage[num_modules][8][2];

//The voltage of the lowest cell
unsigned int lowcell = 5000;
//The voltage of the hightest cell
unsigned int highcell = 0;

//The temp of the hottest module
int highTemp = 0;


unsigned int temp[num_modules][6];
//int balstat[num_modules];
unsigned long looptime = 0;
//unsigned int canupdate = 0;
long balvol = 5000;
char incomingByte;

int moduleIds[10];

float packVoltage = 0;

bool balance = false;
bool debug = false;
bool printData = false;

#define CAN0_INT 2                              // Set INT to pin 2
MCP_CAN CAN0(10);                               // Set CS to pin 9 de[ending on shield used

#define RELAY_PIN 7                            // This pin opens or closes a relay, which allows the battery to be connected (discharging) or not 

#define SCREEN_WIDTH 128 // OLED display width,  in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels

// declare an SSD1306 display object connected to I2C
Adafruit_SSD1306 oled(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, -1);

void setup()
{
  Serial.begin(115200);

  if (!oled.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    while (true);
  }

  //Initialize the module IDS to -1. By default empty arrays initialise with 0s as the default.
  //0s are valid numbers in this array, so must change them to something else.
  for (int i=0; i<num_modules; i++){
    moduleIds[i] = -1;
  }

  // Initialize MCP2515 running at 16MHz with a baudrate of 500kb/s and the masks and filters disabled.
  if (CAN0.begin(MCP_ANY, CAN_500KBPS, MCP_16MHZ) == CAN_OK)
    Serial.println(F("MCP2515 Initialized Successfully!")); // the 'F' notation is used to save on RAM by storing the message on the flash memory
  else
    Serial.println(F("Error Initializing MCP2515..."));

  CAN0.setMode(MCP_NORMAL);                     // Set operation mode to normal so the MCP2515 sends acks to received data.

  pinMode(CAN0_INT, INPUT);                            // Configuring pin for /INT input

  pinMode(RELAY_PIN, OUTPUT);

  //Turn high by default, so the relay is open. Wait for correct CAN signals to close it.
  digitalWrite(RELAY_PIN, LOW);

}

void loop()
{
  if (Serial.available())
  {
    menu();
  }


  if (CAN0.checkReceive() == 3) // If CAN0_INT pin is low, read receive buffer
  {
    candecode();
  }

  /*if (millis() > canupdate + 400)
  {
       cansend();
  }*/
  if (millis() > looptime + 100)
  {

    processPackData();
    controlConnectionRelay();
  }

  if (millis() > looptime + 50)
  {
    printPackInfo();
    printPackInfoToLED();
    looptime = millis();
  }
}

void cansend()
{
  if (balance == true)
  {
  //  mes[0] = highByte(lowcell);
  //  mes[1] = lowByte(lowcell);
  //  mes[2] = balance;

  
  }
  else
  {
  //  mes[0] = 0;
  //  mes[1] = 0;
  //  mes[2] = 0;
  }
  //CAN0.sendMsgBuf(0x3C3, 0, 8, mes);

}

// qsort requires you to create a sort function
int sort_asc(const void *cmp1, const void *cmp2)
{
  // Need to cast the void * to int *
  int a = *((int *)cmp1);
  int b = *((int *)cmp2);
  return a - b;
}

void candecode()
{
    CAN0.readMsgBuf(&rxId, &len, rxBuf);      // Read data: len = data length, buf = data byte(s)
    
    /*
     * Id variable resolves to 0x1 0x2 0x4, 0x4
     * 0x1: Temp data
     * 0x2: Voltage data 1
     * 0x3: Voltage data 2
     * 0x4: Not used.
     */
    int Id = rxId & 0x00F; // 0x693 => 0x003
    //get the module index..ex 0x693
    //1: cast with 0x0f0 -> 0x090
    //2: then shift 4 bits to the right to get 0.009
    //3: then take 1 to put it into a 0-10 range.
    int moduleID = ((rxId & 0x0F0) >> 4);

    int moduleIndex = -1;
    for (int i=0; i<num_modules; i++){
      if (moduleIds[i] == moduleID){
        moduleIndex = i;
        break;
      }
    }

    //If it's not in the list, then add it, and sort the array
    if (moduleIndex == -1){
      for (int i=0; i<num_modules; i++){
        if (moduleIds[i] == -1){
          moduleIds[i] = moduleID;
          moduleIndex = i;
          break;
        }
      }
      
      qsort(moduleIds, num_modules, sizeof(int), sort_asc);
    }


  //  boardpres = boardpres | (1 << CMU);

    if (len != 8){
      return;
    }

    int timeNow = millis();

    switch (Id)
    {
      case 0x1:
       // balstat[CMU] = rxBuf[0];
        temp[moduleIndex][0] = rxBuf[2] * 256 + rxBuf[3];
        temp[moduleIndex][1] = rxBuf[4] * 256 + rxBuf[5];
        temp[moduleIndex][2] = rxBuf[6] * 256 + rxBuf[7];
        break;

      case 0x2:
        voltage[moduleIndex][0][0] = rxBuf[0] * 256 + rxBuf[1];
        voltage[moduleIndex][1][0] = rxBuf[2] * 256 + rxBuf[3];
        voltage[moduleIndex][2][0] = rxBuf[4] * 256 + rxBuf[5];
        voltage[moduleIndex][3][0] = rxBuf[6] * 256 + rxBuf[7];
        voltage[moduleIndex][0][1] = timeNow;
        voltage[moduleIndex][1][1] = timeNow;
        voltage[moduleIndex][2][1] = timeNow;
        voltage[moduleIndex][3][1] = timeNow;
        break;

      case 0x3:
        voltage[moduleIndex][4][0] = rxBuf[0] * 256 + rxBuf[1];
        voltage[moduleIndex][5][0] = rxBuf[2] * 256 + rxBuf[3];
        voltage[moduleIndex][6][0] = rxBuf[4] * 256 + rxBuf[5];
        voltage[moduleIndex][7][0] = rxBuf[6] * 256 + rxBuf[7];
        voltage[moduleIndex][4][1] = timeNow;
        voltage[moduleIndex][5][1] = timeNow;
        voltage[moduleIndex][6][1] = timeNow;
        voltage[moduleIndex][7][1] = timeNow;
        break;

      default:

        break;
    }
}

void processPackData()
{ 

  packVoltage = 0;
  lowcell = 5000;
  highcell = 0;
  highTemp = 0;

  //Get the current time to check the expiry of the cell data.
  //Do it here instead of inside the for so there's a consistent comparator
  int timeNow = millis();
  
  for (int x = 0; x < num_modules; x++)
  {
      for (int i = 0; i < 8; i++)
      {
        if(timeNow - voltage[x][i][1] > CELL_EXPIRY){
          voltage[x][i][0] = -1;
        }

        int cellVoltage = voltage[x][i][0];
        
        if (cellVoltage < lowcell)
        {
          lowcell = cellVoltage;
        }
        if (cellVoltage > highcell)
        {
          highcell = cellVoltage;
        }
        packVoltage += float(cellVoltage) / 1000;
      }
      for (int i = 0; i < 3; i++)
      {
        int moduleTemp = float(temp[x][i]) * 0.001;
        if (moduleTemp > highTemp){
          highTemp = moduleTemp;
        }
      }
    
  }
}

void printPackInfo()
{ 
  if (printData == false){
    return;
  }
  
  for (int x = 0; x < num_modules; x++)
  {
    //if ( boardpres == (1 << x) )
   // {
  
      //Serial.print(F("Module_"));
      //Serial.print(x + 1);
      //Serial.print(" || ");
      //Serial.print(balstat[x], BIN);
      //Serial.print(F(" || "));
      
      for (int i = 0; i < 8; i++)
      {
        int cellVoltage = voltage[x][i][0];
       // sprintf(msgString, "Cell_ %d %d mv", i+1, cellVoltage);
        //Serial.print(msgString);
      }
      
      //Serial.println(); 

      for (int i = 0; i < 3; i++)
      {
        int moduleTemp = float(temp[x][i]) * 0.001;
        //sprintf(msgString, "Temp_ %d %d C", i+1, moduleTemp);
        //Serial.print(msgString);
      }
      //Serial.print(F("\n\n"));
      
    //}
  }  

  //sprintf(msgString, "\n\n *** Pack Voltage : %d V...Variance : %d mV \n\n***\n\n", packVoltage, (highcell - lowcell));
 
}

void printPackInfoToLED()
{ 
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 00);        // position to display

  sprintf(msgString, "Pack: %d V", packVoltage);
  oled.println(msgString);

  sprintf(msgString, "HI: %dV \nLOW: %dV, Bal: %dmv", highcell/100, lowcell/100, (highcell - lowcell)/100);
  oled.println(msgString);

  sprintf(msgString, "High Temp: %d", highTemp);
  oled.println(msgString);

  oled.display();               // show on OLED
 
}

void controlConnectionRelay(){
  /*decide whether to stop discharging by performing some tests on the incoming data*/

  if (packVoltage < 300){
    digitalWrite(RELAY_PIN, LOW);
    //Serial.println(F("LOW -- less than 300V"));
  } else if (highcell - lowcell > 100){
    int x = lowcell;
    //if the difference between the hightest and lowest cell is over 100mv,
    //balance is going out of wack, so stop discharging.
    digitalWrite(RELAY_PIN, LOW);
    //Serial.print(F("LOW -- out of balance..."));
    //Serial.print(x);
    //Serial.println();
  }else if (highTemp >= 40){
    digitalWrite(RELAY_PIN, LOW);
    //Serial.print(F("LOW -- temo too highL.."));
    //Serial.print(highTemp);
    //Serial.println();
  }else{
    
    digitalWrite(RELAY_PIN, HIGH);
    //Serial.println(F("HIGH"));
  }
}

void menu ()
{
  incomingByte = Serial.read(); // read the incoming byte:
  Serial.println();
  switch (incomingByte)
  {
    /*
      case 'v'://v Balance setpoint
      balvol = Serial.parseInt();
      if (balvol < 3500)
      {F
        balvol = 3500;
      }
      Serial.println();
      Serial.print(balvol);
      Serial.print(" mV balance setpoint");
      Serial.println();
      break;
    */

    case 'p' ://toggle balance

      if (printData == true)
      {
        printData = false;
        //Serial.print(F(" Print OFF"));
      }
      else
      {
        printData = true;
        //Serial.print(F(" Print ON"));
      }
      break;
    case 'd' ://toggle debug

      if (debug == false)
      {
        debug = true;
        //Serial.print(F(" Debug ON"));
      }
      else
      {
        debug = false;
        //Serial.print(F(" Debug OFF"));
      }
      break;
    case 'b' ://toggle balance
      if (balance == false)
      {
        balance = true;
        //Serial.print(F(" Balancing ON"));
      }
      else
      {
        balance = false;
        //Serial.print(F(" Balancing OFF"));
      }
      break;
  }
  //Serial.println();
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
