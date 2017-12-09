/*
  SerialCommand.h - Library for handle configurables serial commands.
  Created by Hector Oliveros, November 6, 2017.
  Released into the public domain.
*/

#ifndef SERIAL_COMMAND
#define SERIAL_COMMAND

#include <Arduino.h>
#include <SoftwareSerial.h>
#include "HashMap.h"

// Define the maximun number of commands
#define MAX_COMMAND_COUNT 20

struct SerialCommandConfig
{
  HardwareSerial *HW_IN_Stream;  // Hardware serial IN, e.g Serial, Serial1, etc
  HardwareSerial *HW_OUT_Stream; // Hardware serial OUT.e.g Serial, Serial1, etc

  SoftwareSerial *SW_IN_Stream;
  SoftwareSerial *SW_OUT_Stream;

  // IF is true, use the default HW_IN_Stream and HW_OUT_Stream else
  // use SW_IN_Stream and  SW_OUT_Stream
  bool USE_HW_SERIAL;

  /** Command format config **/
  char BOM; // Begin Of Message
  char ASSIGNATION_CHAR;
  char EOM; // End Of Message

  /** Confirmations (ACK / NACK) config **/
  byte ACK;  // Must be a NON printable char (Like 0x06)
  byte NACK; // Must be a NON printable char (Like 0x21)
  byte RESEND_MARK; 
  bool WAIT_FOR_ACK_ON_SEND;
  bool SEND_ACK_ON_CMD_RECEIVED;
  bool SEND_NACK_ON_UNKNOWN_CMD_RECEIVED;
  bool RESEND_MESSAGE_IN_ACK_ABSENSE;  // Valid only if WAIT_FOR_ACK_ON_SEND is true
  bool RESEND_MESSAGE_IN_NACK_RECEIVE;
  bool WRAP_CONFIRMATIONS;  
  int MAX_ATTEMPS_WAIT_FOR_RESPONSE;
  int DELAY_AFTER_FAILED_RESPONSE;
  int MAX_ATTEMPS_OF_FAILED_MESSAGE_SEND;
  int MAX_NACK_ATTEMPS;

  /**
    * WAIT_FOR_ACK = true     
    * ACK= 0x06 
    * NACK= 0x21
    * 
    * The command format defaults values are:
    * BOM = '['
    * EOM = ']'
    * ASSIGNATION = '='   
    */
  SerialCommandConfig()
  {
    // Serial streams
    HW_IN_Stream = &Serial;
    HW_OUT_Stream = &Serial;
    SW_IN_Stream = NULL;
    SW_OUT_Stream = NULL;

    // If is true then use HW_IN_Stream and HW_OUT_Stream. 
    // Else use SW_IN_Stream and SW_OUT_Stream
    USE_HW_SERIAL = true;

    // Message structure
    BOM = '[';
    ASSIGNATION_CHAR = '=';
    EOM = ']';

    // Confirmations
    ACK = 0x06;
    NACK = 0x21;
    WAIT_FOR_ACK_ON_SEND = true;
    SEND_ACK_ON_CMD_RECEIVED = true;
    RESEND_MESSAGE_IN_ACK_ABSENSE = true;
    SEND_NACK_ON_UNKNOWN_CMD_RECEIVED = true;    
    WRAP_CONFIRMATIONS = true;
    MAX_ATTEMPS_WAIT_FOR_RESPONSE = 3;    
    MAX_ATTEMPS_OF_FAILED_MESSAGE_SEND = 3;
    DELAY_AFTER_FAILED_RESPONSE = 0.1;
    RESEND_MARK = '!';
    RESEND_MESSAGE_IN_NACK_RECEIVE = false;
    MAX_NACK_ATTEMPS = 4;        
  }
};

struct Command
{
  String command;
  String value;
};

/**
 * The command must have a name and a value and it must have the 
 * following structure: * 
 * <BOM><COMMAND><ASSIGNATION_CHAR><VALUE><EOM>
 * 
 * Command Examples with BOM='#', COMMAND="SET_LED", ASSIGNATION_CHAR='=', VALUE="TRUE" and EOM='\n' 
 * #SET_LED_ON=TRUE
 */
class SerialCommandLib
{
public:

  SerialCommandLib();

  /**
   * if cfg is null then takes the default values  
   */
  void setConfig(SerialCommandConfig config);

  /**
     * Read a command from the serial stream and execute the associated callback (if exist). 
     * 
     * @return The incoming command. If no command is read then return null.
     * 
     * Notes:
     * - The command value must be a printable string (you must take care of parse it) 
     */
  String readCommand();

  /**
     * Add new command with their respective callback
     * 
     * @param cmd The expected command. E.g 'RUN'
     * @param callback The function that will be executed when the command is received.
     *                 Must be return int and receive a String as a param.
     *                 This value is the value assigned to the command.
     *                 e.g: int function(String value){} 
     */
  void addCommandCallback(String cmd, int (*callback)(String));

  /** Send a command for the serial stream  */
  void sendCommand(String cmd, String value);

  //private:
private:
  bool isSerialAvailable();
  
  String readUntil(char endString);
  byte readByte();
  bool processCommand(String command);  
  bool validateCommandFormat(String command);  
  String extractCommandName(String command);  
  String extractCommandValue(String command);  
  bool excecuteCommand(String commandName, String commandValue);  
  String extractString(String str, char start, char end);  

  /**
   * Make a command by data and value.
   * e.g if data is 'RUN' and value is 'true' then make: [RUN=true]
   */ 
  String makeCommand(String data,String value, bool isResend = false);

  void write(String data);
  void writeByte(byte data);
  
  /** Send ACK or NACK if apply **/
  void sendConfirmation(bool positive);
  void sendACK();
  void sendNACK();
  bool waitForResponse();

  // ATTR
  HashMap<String, int (*)(String), MAX_COMMAND_COUNT> mCommands;
  SerialCommandConfig mCfg;

  String __ack_prebuild;
  String __nack_prebuild;
};

#endif

