/*
 * 
 * This module is designed to monitor multiple outlander BMS packs, and is based on the code found here:
 * https://github.com/Tom-evnut/OutlanderPHEVBMS
 * 
 * This reads any number of outlander packs, and displays their data to a display
 * 
 * Outlander packs output IDS in the range 0x61X -> 0x6AX
 * In order to read more packs, the IDS of the extra packs have to be altered using a man-in-the-middle circuit (also in this repo)
 * to change their IDs. The assumption is that they will be sequential (ex: 0x71x -> 0x7AX). 
 * 
 */
#include <mcp_can.h>
#include <SPI.h>

/*Show data on an LED display*/
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

long unsigned int rxId;
const int num_packs = 2;
const int num_modules = 10; 
short boardpres = 0;
unsigned char len = 0;
unsigned char rxBuf[8];
char msgString[128];                        // Array to store serial string
byte mes[8] = {0, 0, 0, 4, 3, 0, 0, 0};

//The number of milliseconds before the reading from a cell expires, and
//gets set to -1
int CELL_EXPIRY = 10000;
/*dimensions are: 
 * Pack ID 
 * ModuleID
 * Voltage 
 * expiry time
 */
int voltage[num_packs][num_modules][8][2];

/*Assume the can IDS look like: 0x6XX, 0x7XX*/
int pack_ids[num_packs] = {6, 7};

//The voltage of the lowest cell
unsigned int lowcell = 5000;
//The voltage of the hightest cell
unsigned int highcell = 0;

//The temp of the hottest module
int highTemp = 0;


unsigned int temp[num_packs][num_modules][6];
//int balstat[num_modules];
unsigned long looptime = 0;
unsigned int screenupdate = 0;
long balvol = 5000;
char incomingByte;

int moduleIds[num_packs][num_modules];

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
  for(int i=0; i<num_packs; i++){
    int packID = pack_ids[i];
    for (int j=0; j<num_modules; j++){
      moduleIds[i][j] = -1;
    }
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
  if (millis() > screenupdate + 100)
  {
    processPackData();
    screenupdate = millis();
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

    /*turn 0x6XX into 6 or 0x7XX into 7 by shifting across 8 bits*/
    int packID = ((rxId & 0xF00) >> 8);

    /*check if this is a message coming from one of the batteries*/
    bool isBatteryMsg = false;
    int packIdx = -1;
    for (int i=0; i<num_packs; i++){
      if (packID == pack_ids[i]){
        packIdx = i;
        break;
      }
      
    }
    
    //this isn't coming from one of the packs, so ignore.
    if (packIdx == -1){
      return;
    }

   // Serial.println(packIdx);
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
    int moduleID = ((rxId & 0x0F0) >> 4);

    int moduleIndex = -1;
    for (int i=0; i<num_modules; i++){
      if (moduleIds[packIdx][i] == moduleID){
        moduleIndex = i;
        break;
      }
    }

    //If it's not in the list, then add it, and sort the array
    if (moduleIndex == -1){
      for (int i=0; i<num_modules; i++){
        if (moduleIds[packIdx][i] == -1){//find an unset entry in the list
          moduleIds[packIdx][i] = moduleID;
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
        temp[packIdx][moduleIndex][0] = rxBuf[2] * 256 + rxBuf[3];
        temp[packIdx][moduleIndex][1] = rxBuf[4] * 256 + rxBuf[5];
        temp[packIdx][moduleIndex][2] = rxBuf[6] * 256 + rxBuf[7];
        break;

      case 0x2:
        voltage[packIdx][moduleIndex][0][0] = rxBuf[0] * 256 + rxBuf[1];
        voltage[packIdx][moduleIndex][1][0] = rxBuf[2] * 256 + rxBuf[3];
        voltage[packIdx][moduleIndex][2][0] = rxBuf[4] * 256 + rxBuf[5];
        voltage[packIdx][moduleIndex][3][0] = rxBuf[6] * 256 + rxBuf[7];
        voltage[packIdx][moduleIndex][0][1] = timeNow;
        voltage[packIdx][moduleIndex][1][1] = timeNow;
        voltage[packIdx][moduleIndex][2][1] = timeNow;
        voltage[packIdx][moduleIndex][3][1] = timeNow;
        

         if (debug == true){
          Serial.print(rxId, HEX);
          Serial.print(" ");
          Serial.print(packID, HEX);
          Serial.print(" ");
          for (int i=0; i<8; i++){
              Serial.print(rxBuf[i], HEX);
              Serial.print(" ");
            }
            Serial.println();
         }
        break;

      case 0x3:
        voltage[packIdx][moduleIndex][4][0] = rxBuf[0] * 256 + rxBuf[1];
        voltage[packIdx][moduleIndex][5][0] = rxBuf[2] * 256 + rxBuf[3];
        voltage[packIdx][moduleIndex][6][0] = rxBuf[4] * 256 + rxBuf[5];
        voltage[packIdx][moduleIndex][7][0] = rxBuf[6] * 256 + rxBuf[7];
        voltage[packIdx][moduleIndex][4][1] = timeNow;
        voltage[packIdx][moduleIndex][5][1] = timeNow;
        voltage[packIdx][moduleIndex][6][1] = timeNow;
        voltage[packIdx][moduleIndex][7][1] = timeNow;
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
  for (int i=0; i<num_packs; i++){
    for (int j=0; j<num_modules; j++){
        for (int k = 0; k < 8; k++){

         if(timeNow - voltage[i][j][k][1] > CELL_EXPIRY){
            voltage[i][j][k][0] = -1;
            continue;
         }
  
          int cellVoltage = voltage[i][j][k][0];
          
          if (cellVoltage < lowcell)
          {
            lowcell = cellVoltage;
          }
          if (cellVoltage > highcell)
          {
            highcell = cellVoltage;
          }
          packVoltage += (float(cellVoltage) / float(1000));
        }
        for (int k = 0; k < 3; k++){
          int moduleTemp = float(temp[i][j][k]) * 0.001;
          if (moduleTemp > highTemp){
            highTemp = moduleTemp;
          }
        }
    } 
  }

  //As there could be more than 1 pack, and we assume they are in parallel, we need to divide the voltage by that nuber of packs
  packVoltage = packVoltage / num_packs;
}

void printPackInfo()
{ 
  if (printData == false){
    return;
  }
  
  for (int i=0; i<num_packs; i++){
    int packID = pack_ids[i];
    sprintf(msgString, "Pack %d (%d)\n", (i+1), packID);
    Serial.print(msgString);
    
    for (int j = 0; j < num_modules; j++)
    {
      //if ( boardpres == (1 << x) )
     // {
    
        sprintf(msgString, "Module_%d\n", (j+1));
        Serial.print(msgString);
  //      Serial.print(" || ");
  //      Serial.print(balstat[x], BIN);
  //      Serial.print(F(" || "));
        
        for (int k = 0; k < 8; k++)
        {
          int cellVoltage = voltage[i][j][k][0];
          sprintf(msgString, "Cell_ %d %d mv, ", k+1, cellVoltage);
          Serial.print(msgString);
        }
        
        Serial.println(); 
  
        for (int k = 0; k < 3; k++)
        {
          int moduleTemp = float(temp[i][j][k]) * 0.001;
          sprintf(msgString, "Temp_ %d %d C", k+1, moduleTemp);
          Serial.print(msgString);
        }
        Serial.print(F("\n\n"));
        
      //}
    }
  }

  Serial.print("HighCell"); Serial.println(highcell%100);
  Serial.print("LowCell"); Serial.println(lowcell);

  int cellVariance = highcell - lowcell;

  Serial.print("Variance"); Serial.println(cellVariance);
  sprintf(msgString, "Variance %d\n", cellVariance);
  Serial.print(msgString);


  sprintf(msgString, "Pack Voltage : %d.%02d mV \nVariance : %d mV", (int)packVoltage, (int)(packVoltage*100)%100, cellVariance);
  Serial.print(msgString);
 
}

void printPackInfoToLED()
{ 
  oled.clearDisplay(); // clear display

  oled.setTextSize(1);          // text size
  oled.setTextColor(WHITE);     // text color
  oled.setCursor(0, 00);        // position to display
  //sprintf doesn't support floats, so we need to do it this way
  sprintf(msgString, "Pack: %d.%02d V",  (int)packVoltage, (int)(packVoltage*100)%100);
  oled.println(msgString);

  sprintf(msgString, "HI: %d.%02dV \nLOW: %d.%02dV, Bal: %dmv", highcell/100, highcell%100, lowcell/100, lowcell%100, (highcell - lowcell));
  oled.println(msgString);

  sprintf(msgString, "High Temp: %d", highTemp);
  oled.println(msgString);

  oled.display();               // show on OLED
 
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
        Serial.print(F(" Print OFF"));
      }
      else
      {
        printData = true;
        Serial.print(F(" Print ON"));
      }
      break;
    case 'd' ://toggle debug

      if (debug == false)
      {
        debug = true;
        Serial.print(F(" Debug ON"));
      }
      else
      {
        debug = false;
        Serial.print(F(" Debug OFF"));
      }
      break;
    case 'b' ://toggle balance
      if (balance == false)
      {
        balance = true;
        Serial.print(F(" Balancing ON"));
      }
      else
      {
        balance = false;
        Serial.print(F(" Balancing OFF"));
      }
      break;
  }
  Serial.println();
}
/*********************************************************************************************************
  END FILE
*********************************************************************************************************/
