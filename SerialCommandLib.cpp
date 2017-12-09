#include <Arduino.h>
#include "SerialCommandLib.h"

////////////////////////////
// Serial read functions ///
////////////////////////////
SerialCommandLib::SerialCommandLib()
{
    __ack_prebuild = makeCommand(String("ACK"), String(mCfg.ACK), false);
    __nack_prebuild = makeCommand(String("NACK"), String(mCfg.NACK), false);
}

String SerialCommandLib::readCommand()
{
    String command = "";
    while (isSerialAvailable())
    {
        command = readUntil(mCfg.EOM);
        bool result = processCommand(command);
        sendConfirmation(result);
    }
    return command;
}

bool SerialCommandLib::isSerialAvailable()
{
    return mCfg.USE_HW_SERIAL ? mCfg.HW_IN_Stream->available() : mCfg.SW_IN_Stream->available();
}

String SerialCommandLib::readUntil(char endString)
{
    String cmd = mCfg.USE_HW_SERIAL ? mCfg.HW_IN_Stream->readStringUntil(endString) : mCfg.SW_IN_Stream->readStringUntil(endString);
    cmd += mCfg.EOM;
    return cmd;
}

byte SerialCommandLib::readByte()
{
    return byte(mCfg.USE_HW_SERIAL ? mCfg.HW_IN_Stream->read() : mCfg.SW_IN_Stream->read());
}

///////////////////////////////
// Command process functions //
///////////////////////////////

bool SerialCommandLib::processCommand(String command)
{
    if (!validateCommandFormat(command))
    {
        return false;
    }
    // Example: If command is [RUN=true]
    // The commandName == "RUN" and commandValue == "true"
    String commandName = extractCommandName(command);
    String commandValue = extractCommandValue(command);
    return excecuteCommand(commandName, commandValue);
}

bool SerialCommandLib::excecuteCommand(String commandName, String commandValue)
{
    int commandIdx = this->mCommands.indexOf(commandName);
    if (commandIdx == -1)
    {
        return false;
    }
    int (*_callback)(String) = this->mCommands.valueAt(commandIdx);
    _callback(commandValue);
    return true;
}

////////////////////////////////////////
// Command field extraction functions //
////////////////////////////////////////

String SerialCommandLib::extractString(String str, char start, char end)
{
    int startIDX = str.indexOf(start) + 1;
    int endIDX = str.indexOf(end);
    return str.substring(startIDX, endIDX);
}

String SerialCommandLib::extractCommandName(String command)
{
    return extractString(command, mCfg.BOM, mCfg.ASSIGNATION_CHAR);
}

String SerialCommandLib::extractCommandValue(String command)
{
    return extractString(command, mCfg.ASSIGNATION_CHAR, mCfg.EOM);
}

bool SerialCommandLib::validateCommandFormat(String command)
{
    return command.indexOf(mCfg.BOM) != -1 &&
           command.indexOf(mCfg.ASSIGNATION_CHAR) != -1;
}

////////////////////////////
// Serial write functions //
////////////////////////////

void SerialCommandLib::sendCommand(String cmd, String value)
{
    int count = 0;
    bool result = false;
    while (!result && count < mCfg.MAX_ATTEMPS_OF_FAILED_MESSAGE_SEND)
    {
        write(makeCommand(cmd, value, count > 0));
        if (mCfg.WAIT_FOR_ACK_ON_SEND)
        {
            result = waitForResponse();
        }
        else
        {
            break;
        }
        count++;
    }
}

bool SerialCommandLib::waitForResponse()
{
    int count = 0;
    int max_nack_attemps_remaining = mCfg.MAX_NACK_ATTEMPS;        
    while (max_nack_attemps_remaining && count < mCfg.MAX_ATTEMPS_WAIT_FOR_RESPONSE)
    {
        if (mCfg.WRAP_CONFIRMATIONS)
        {
            String cmd = readUntil(mCfg.EOM);
            if (cmd == __ack_prebuild || cmd == __nack_prebuild)
            {
                break;
            }
        }
        else
        {
            byte b = readByte();
            if (b == mCfg.ACK || b == mCfg.NACK)
            {
                if (b == mCfg.NACK && mCfg.RESEND_MESSAGE_IN_NACK_RECEIVE)
                {
                    count = 0;
                    max_nack_attemps_remaining--;
                    continue;
                }
                break;
            }
        }
        count++;     
        delay(mCfg.DELAY_AFTER_FAILED_RESPONSE);
    }
    return count < mCfg.MAX_ATTEMPS_WAIT_FOR_RESPONSE;
}

String SerialCommandLib::makeCommand(String data, String value, bool isResend)
{
    // <RESEND_MARK>?<BOM><COMMAND><ASSIGNATION_CHAR><VALUE><EOM>
    String resendMark = isResend ? String(mCfg.RESEND_MARK) : "";
    return resendMark + String(mCfg.BOM) + data + mCfg.ASSIGNATION_CHAR + value + mCfg.EOM;
}

void SerialCommandLib::sendConfirmation(bool positive)
{
    if (positive && mCfg.SEND_ACK_ON_CMD_RECEIVED)
    {
        sendACK();
    }
    else if (!positive && mCfg.SEND_NACK_ON_UNKNOWN_CMD_RECEIVED)
    {
        sendNACK();
    }
}

void SerialCommandLib::sendACK()
{
    mCfg.WRAP_CONFIRMATIONS ? write(__ack_prebuild) : writeByte(mCfg.ACK);
}

void SerialCommandLib::sendNACK()
{
    mCfg.WRAP_CONFIRMATIONS ? write(__nack_prebuild) : writeByte(mCfg.NACK);
}

void SerialCommandLib::write(String data)
{
    if (mCfg.USE_HW_SERIAL)
    {
        mCfg.HW_OUT_Stream->print(data);
    }
    else
    {
        mCfg.SW_OUT_Stream->print(data);
    }
}

void SerialCommandLib::writeByte(byte data)
{
    if (mCfg.USE_HW_SERIAL)
    {
        mCfg.HW_OUT_Stream->write(data);
    }
    else
    {
        mCfg.SW_OUT_Stream->write(data);
    }
}

////////////////////
// Configuration //
///////////////////

void SerialCommandLib::addCommandCallback(String cmd, int (*callback)(String))
{
    mCommands[cmd] = callback;
}

void SerialCommandLib::setConfig(SerialCommandConfig config)
{
    this->mCfg = config;
}