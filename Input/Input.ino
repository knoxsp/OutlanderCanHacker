#include <SPI.h>
#include <mcp2515.h>

struct can_frame canMsg;

MCP2515 mcp2515(10);

void setup() {
  while (!Serial);
  Serial.begin(115200);
  //NANO: can receive from an uno at 8mhz but not 16mhz, but receives from a nano at 16mhz
  //UNO: Can receive at 16mhz but not 8mhz
  mcp2515.reset();
  
  mcp2515.setBitrate(CAN_500KBPS, MCP_16MHZ);
  mcp2515.setNormalMode();
  
  Serial.println("------- CAN Read ----------");
  Serial.println("ID  DLC   DATA");
}

void loop() {
  int msgstatus = mcp2515.readMessage(&canMsg);
  if (msgstatus == MCP2515::ERROR_OK) {
    Serial.println(canMsg.can_id, HEX); // print ID
    Serial.print(" "); 
    Serial.print(canMsg.can_dlc, HEX); // print DLC
    Serial.print(" ");
    
    for (int i = 0; i<canMsg.can_dlc; i++)  {  // print the data
      Serial.print(canMsg.data[i],HEX);
      Serial.print(" ");
    }
    Serial.println();    
  }else{
    //Serial.println(msgstatus);
  }
}
