/**
 * Example of use of the SerialCommandLib 
 */


#include <Arduino.h>
#include <SoftwareSerial.h>
#include "SerialCommandLib.h"

SerialCommandLib serialCMD;

/** Callback functions **/
int run(String saludo);
int set(String saludo);

void setup()
{
    Serial.begin(9600);

    // 1 - Create config 
    SerialCommandConfig cfg;

    // 2 - Set serial streams objects
    cfg.HW_IN_Stream = &Serial;  // Only if you need to read    
    cfg.HW_OUT_Stream = &Serial; // Only if you need to write
    cfg.USE_HW_SERIAL = true;    // Not necessary, is true by default

    // 3 - Set config
    serialCMD.setConfig(cfg);

    // 4 - Asociate command with functions           
    serialCMD.addCommandCallback("RUN", &run);  
    serialCMD.addCommandCallback("SET", &set);

    // 5- You can immediately start writing and reading commands
    serialCMD.sendCommand("INIT_EXAMPLE", "anyvalue");
    // serialCMD.readCommand();

    /**
     * To test this sketch send a serial message to this arduino, such as:
     * [SET=true] or [RUN = anyvalue]
     */ 
}

int run(String value)
{
    Serial.println("main -> run: RUN:"+value);
    return 1;
}

int set(String value)
{
    Serial.println("main -> run: SET:"+value);
    return 1;
}

void loop()
{   
    String cmd = serialCMD.readCommand();
}