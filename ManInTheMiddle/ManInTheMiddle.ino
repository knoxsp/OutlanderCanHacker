
#include <SPI.h>              //Library for using SPI Communication 
#include <mcp2515.h>          //Library for using CAN Communication

//The following pins are default aruino pins for doing SPI communication
// Pin 11 : SI
// Pin 12 : SO
// Pin 13 : CLK

char stringBuffer[128];

struct can_frame canMsg;
MCP2515 mcp2515(10);                 // SPI CS Pin 10
MCP2515 mcp2515_out(9);                 // SPI CS Pin 9
 
void setup() {
 
  Serial.begin(115200);                //Begins Serial Communication at 9600 baudrate 
  Serial.println("Setting Up");
  mcp2515.reset();                          
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515.setNormalMode();                  //Sets CAN at normal mode

  mcp2515_out.reset();                          
  mcp2515_out.setBitrate(CAN_500KBPS, MCP_8MHZ); //Sets CAN at speed 500KBPS and Clock 8MHz 
  mcp2515_out.setNormalMode();                  //Sets CAN at normal mode
}

void loop() 
{
  int msgStatus = mcp2515.readMessage(&canMsg);
  if (msgStatus == MCP2515::ERROR_OK) // To receive data (Poll Read)
  {
    /*If the can message comes from the outlander CMU (address 0x611)
    then reassign its ID and send out the message on bus 2 */
    int packID = ((canMsg.can_id & 0xF00) >> 8);
    if(packID == 6){
      int newId = canMsg.can_id | 0x700;
      canMsg.can_id = newId; // changed from 0x6XX to 0x7XX
      int status = mcp2515_out.sendMessage(&canMsg);     //Sends the CAN message
    }
  } 
}
