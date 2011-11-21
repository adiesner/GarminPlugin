/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2010 <andreas.diesner [AT] gmx [DOT] de>
 *
 * GarminPlugin is free software: you can redistribute it and/or modify it
 * under the terms of the GNU General Public License as published by the
 * Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * GarminPlugin is distributed in the hope that it will be useful, but
 * WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.
 * See the GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "config.h"
#include <stdio.h>
#include "npapi/npapi.h"
#include "npapi/npfunctions.h"
#include "npapi/npruntime.h"
#if HAVE_PRTYPES_H
	#include <prtypes.h>
#elif HAVE_ZIPSTUB_H
	#include <zipstub.h>
#endif
#include <iostream>
#include <map>
#include "deviceManager.h"
#include "configManager.h"
#include "log.h"
#include <sstream>
#include "zlib.h"
#include <fstream>

#include <math.h>

#include <unistd.h>


#define GETSTRING(_v)          ((_v).UTF8Characters)
#define GETSTRINGLENGTH(_v)    ((_v).UTF8Length)

using namespace std;

/**
 * A variable that stores the plugin name
 */
const char * pluginName = "Garmin Communicator";

/**
 * A variable that stores the plugin description (may contain HTML)
 */
const char * pluginDescription = "<a href=\"http://www.andreas-diesner.de/garminplugin/\">Garmin Communicator - Fake</a> plugin. Version 0.3.6";

/**
 * A variable that stores the mime description of the plugin.
 */
const char * pluginMimeDescription = "application/vnd-garmin.mygarmin:garmin:Garmin Device Web Control";

/**
 * Manages all attached GPS Devices
 */
DeviceManager *devManager = NULL;

/**
 * Manages the plugin configuration
 */
ConfigManager * confManager = NULL;

/**
 * plugin_scriptable_object
 */
static NPObject        *so       = NULL;

/**
 * NP_Initialize gets called with a pointer to the browser functions
 * Use these functions to allocate/delete memory
 */
static NPNetscapeFuncs *npnfuncs = NULL;

/**
 * Unknown
 */
static NPP              inst     = NULL;

/**
 * Counts the number of new's that have been executed on the plugin
 */
static int              instanceCount = 0;

/**
 * Stores the information about plugin properties
 * All these properties can be accessed from the outside. They are stored
 * in the vector propertyList.
 * @see propertyList
 */
typedef struct _Property {
    NPVariantType type; /**< Stores the type of the property */
    bool boolValue;     /**< If boolean value, this contains the value */
    int32_t intValue;   /**< If int value, this contains the value */
    string stringValue; /**< If string value, this contains the value */
    bool writeable;     /**< True if property can be written from the outside */
} Property;

/**
 * Function prototype for functions that can be called from the outside
 */
typedef bool(*pt2Func)(NPObject*, const NPVariant[] , uint32_t, NPVariant *);

/**
 * Map that stores all properties and their names
 */
std::map<string, Property> propertyList;

/**
 * Map that stores all available functions and their names
 */
std::map<string, pt2Func> methodList;

/**
 * Vector that stores all waiting messageboxes that should be displayed to the user
 */
std::vector<MessageBox*> messageList;

/**
 * Stores the last device a write was started to
 */
GpsDevice * currentWorkingDevice = NULL;

/**
 * Stores if the browser supports XEmbed - need to report true to Chrome to get the plugin detected
 */
bool supportsXEmbed = false;


string getParameterTypeStr(const NPVariant arg) {
    switch (arg.type) {
        case NPVariantType_Void:
                return "VOID";
        case NPVariantType_Bool:
                return "BOOL";
        case NPVariantType_Int32:
                return "INT32";
        case NPVariantType_String:
                return "STRING";
        case NPVariantType_Double:
                return "DOUBLE";
        case NPVariantType_Object:
                return "OBJECT";
        case NPVariantType_Null:
                return "NULL";
        default:
            return "UNKNOWN";
    }
}

string getStringFromNPString(const NPString inputStr) {
    // Any NPString object returned when communicating with the browser cannot be
    // assumed to be null-terminated at the appropriate place
    // That is why I use memcpy here and null-terminate the string
    char * sTmp = new char[GETSTRINGLENGTH(inputStr)+1];
    memcpy(sTmp, GETSTRING(inputStr), GETSTRINGLENGTH(inputStr));
    sTmp[GETSTRINGLENGTH(inputStr)] = 0x00; // Null-terminate the string;
    string strValue = sTmp;
    delete sTmp;
    return strValue;
}

/**
 * If debug level, it outputs the content of string property variables into files in /tmp
 */
void debugOutputPropertyToFile(string property) {
    if (Log::enabledDbg()) {
        stringstream filename;
        time_t rawtime;
        time ( &rawtime );
        filename << "/tmp/" << rawtime << "." << property;
        Log::dbg("Writing "+property+" content to file: "+filename.str());
        ofstream output(filename.str().c_str());
        if(output.is_open()) {
            output << propertyList[property].stringValue;
            output.close();
        } else {
            Log::err("Error writing "+property+" content to file: "+filename.str());
        }
    }
}

int getIntParameter(const NPVariant args[], int pos, int defaultVal) {
    int intValue = defaultVal;
    if (args[pos].type == NPVariantType_Int32) {
        intValue = args[pos].value.intValue;
    } else if (args[pos].type == NPVariantType_String) {
        std::string intValueStr = getStringFromNPString(args[pos].value.stringValue);
        Log::dbg("getIntParameter String: "+intValueStr);
        std::istringstream ss( intValueStr );
        ss >> intValue;
        /* Does not work
        if (! ss.good()) {
            [...]
        } */
    } else if (args[pos].type == NPVariantType_Double) {
        double doubleValue = args[pos].value.doubleValue;
        if (Log::enabledDbg()) {
            std::stringstream ss;
            ss << "getIntParameter Double: " << doubleValue;
            Log::dbg(ss.str());
        }
        if ((isnan(doubleValue)) || (isinf(doubleValue))) { // Not A Number detection
            intValue = defaultVal;
        } else {
            intValue = (int)doubleValue;
        }
    } else {
        std::ostringstream errTxt;
        errTxt << "Expected INT parameter at position " << pos << ". Found: " << getParameterTypeStr(args[pos]);
        if (Log::enabledErr()) Log::err(errTxt.str());
    }
    return intValue;
}

string getStringParameter(const NPVariant args[], int pos, string defaultVal) {
    string strValue = defaultVal;
    if (args[pos].type == NPVariantType_Int32) {
        stringstream ss;
        ss << args[pos].value.intValue;
        strValue = ss.str();
    } else if (args[pos].type == NPVariantType_String) {
        strValue  = getStringFromNPString(args[pos].value.stringValue);
    } else {
        std::ostringstream errTxt;
        errTxt << "Expected STRING parameter at position " << pos << ". Found: " << getParameterTypeStr(args[pos]);
        if (Log::enabledErr()) Log::err(errTxt.str());
    }
    return strValue;
}

bool getBoolParameter(const NPVariant args[], int pos, bool defaultVal) {
    bool boolValue = defaultVal;
    if (args[pos].type == NPVariantType_Int32) {
        boolValue = (args[pos].value.intValue == 1) ? true : false;
    } else if (args[pos].type == NPVariantType_String) {
        string strValue = getStringFromNPString(args[pos].value.stringValue);
        boolValue = (strValue.compare("1") == 0) ? true : false;
    } else if (args[pos].type == NPVariantType_Bool) {
        boolValue = args[pos].value.boolValue;
    } else {
        std::ostringstream errTxt;
        errTxt << "Expected BOOL parameter at position " << pos << ". Found: " << getParameterTypeStr(args[pos]);
        if (Log::enabledErr()) Log::err(errTxt.str());
    }
    return boolValue;
}
/*
** encodeBase64
**
** base64 encode a stream adding padding and line breaks as per spec.
** Thanks to Bob Trower 08/04/01
** http://base64.sourceforge.net/b64.c
*/
static const char cb64[]="ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
void encodeBase64( stringstream * input, stringstream *output , int linesize )
{
    unsigned char in[3], out[4];
    int i, len, blocksout = 0;

    while( !input->eof() ) {
        len = 0;
        for( i = 0; i < 3; i++ ) {
            input->get((char&)in[i]);
            if( !input->eof() ) {
                len++;
            }
            else {
                in[i] = 0;
            }
        }
        if( len ) {
            out[0] = cb64[ in[0] >> 2 ];
            out[1] = cb64[ ((in[0] & 0x03) << 4) | ((in[1] & 0xf0) >> 4) ];
            out[2] = (unsigned char) (len > 1 ? cb64[ ((in[1] & 0x0f) << 2) | ((in[2] & 0xc0) >> 6) ] : '=');
            out[3] = (unsigned char) (len > 2 ? cb64[ in[2] & 0x3f ] : '=');
            for( i = 0; i < 4; i++ ) {
                output->put(out[i]);
            }
            blocksout++;
        }
        if( blocksout >= (linesize/4)) {
            input->peek(); // Read but do not remove one character, so that eof really returns eof if at the end of the file
            if (( blocksout ) && (!input->eof())) {
                (*output) << endl;
            }
            blocksout = 0;
        }
    }
}

/**
 * Compresses a string using gzip and encodes it using base64
 * Use uudecode to unpack and then gunzip
 */
#define CHUNK 16384
string compressStringData(const string text, const string filename) {
    if (Log::enabledDbg()) {
        stringstream ss;
        ss << text.size();
        Log::dbg("Compressing content of string with length: " + ss.str());
    }
    std::stringstream compressed ("");
    int ret;
    unsigned have;
    z_stream strm;
    char out[CHUNK];

    /* allocate deflate state */
    strm.zalloc = Z_NULL;
    strm.zfree = Z_NULL;
    strm.opaque = Z_NULL;
    ret = deflateInit2(&strm, Z_DEFAULT_COMPRESSION, Z_DEFLATED, 31, 8, Z_DEFAULT_STRATEGY);
    if (ret != Z_OK) {
         Log::err("zLib Initialization failed at deflateInit2()");
         return "";
    }

    strm.avail_in = text.length();
    strm.next_in = (Bytef*) text.c_str();

    /* run deflate() on input until output buffer not full, finish
       compression if all of source has been read in */
    do {
        strm.avail_out = CHUNK;
        strm.next_out = (Bytef*) out;
        ret = deflate(&strm, Z_FINISH);    /* no bad return value */
        have = CHUNK - strm.avail_out;
        compressed.write(out, have);
        if (compressed.bad()) {
            (void)deflateEnd(&strm);
            Log::err("compressStringData error during compression and writing to output buffer");
            return "";
        }
    } while (strm.avail_out == 0);

    /* clean up */
    (void)deflateEnd(&strm);

    std::stringstream outstream;
    outstream << "begin-base64 644 " << filename << endl;
    encodeBase64(&compressed,&outstream, 76);
    outstream << endl << "====" << endl;

    return outstream.str();
}

void printFinishState(string text, int state) {
    if (Log::enabledDbg()) {
        std::stringstream ss;
        ss << "Finish State of function " << text << ": ";
        // 0 = idle      1 = working      2 = waiting for user input      3 = finished
        switch (state) {
            case 0: ss << "Idle"; break;
            case 1: ss << "Working"; break;
            case 2: ss << "Waiting for user input"; break;
            case 3: ss << "Finished"; break;
            default: ss << "Unknown ("<<state<<")";
        }
        Log::dbg(ss.str());
    }
}

/**
 * Updates the property indicating the progress state
 * @param title Contains the title of the progress message
 * @param state Contains the percentage stage (0-100) of the progress
 */
void updateProgressBar(string title, int state) {
    stringstream ss;
    ss << "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<ProgressWidget xmlns=\"http://www.garmin.com/xmlschemas/PluginAPI/v1\">\n<Title>";
    ss << title;
    ss << "</Title>\n<Text></Text>\n<Text></Text>\n<Text>";
    ss << state;
    ss << "% complete</Text><ProgressBar Type=\"Percentage\" Value=\"";
    ss << state;
    ss << "\"/></ProgressWidget>\n";
	propertyList["ProgressXml"].stringValue = ss.str();
}

/**
 * Method to unlock the plugin.
 * This method gets called from the outside.
 * Example Parameter: "http://wwwdev.garmin.com","3e2bab85cbc4e17d3ce5b55c8f2aa652"
 * Return int(1) to signal that unlock was successful
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodUnlock(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    propertyList["Locked"].intValue = 0;

	result->type = NPVariantType_Int32;
	result->value.intValue = 1;
	return true;
}

/**
 * Method returns xml with attached devices.
 * This method gets called from the outside.
 * This method takes no parameter
 * Return string(xml) to signal which devices were found
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodDevicesXmlString(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    string deviceXml = devManager->getDevicesXML();
    char *outStr = (char*)npnfuncs->memalloc(deviceXml.size() + 1);
    memcpy(outStr, deviceXml.c_str(), deviceXml.size() + 1);
    result->type = NPVariantType_String;
    GETSTRING(result->value.stringValue) = outStr;
    GETSTRINGLENGTH(result->value.stringValue) = deviceXml.size();

	return true;
}

/**
 * Search for attached devices.
 * This method gets called from the outside.
 * This method takes no parameter
 * Return nothing
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodStartFindDevices(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    currentWorkingDevice = NULL;

    //Reread current configuration in case it has changed on disk
    confManager->readConfiguration();
    devManager->setConfiguration(confManager->getConfiguration());

    devManager->startFindDevices();
    return true;
}

/**
 * Cancel the search for devices.
 * This method gets called from the outside.
 * This method takes no parameter
 * Return nothing
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodCancelFindDevices(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    devManager->cancelFindDevices();
    return true;
}

/**
 * Returns the status of the device search (Finished or still searching)
 * This method gets called from the outside.
 * This method takes no parameter
 * Return int(1) when finished
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodFinishFindDevices(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    result->type = NPVariantType_Int32;
    result->value.intValue = devManager->finishedFindDevices();
    printFinishState("FinishFindDevices", result->value.intValue);
    return true;
}

/**
 * Starts writing data to the gps device. The data and the filename
 * is stored in the property FileName and GpsXml.
 * This method gets called from the outside.
 * This method takes one parameter - the device number as int
 * Return int(1) when write started
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodStartWriteToGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    updateProgressBar("Write to GPS", 0);
    if (argCount == 1) {
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startWriteToGps(propertyList["FileName"].stringValue, propertyList["GpsXml"].stringValue);
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartWriteToGps: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartWriteToGps: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartWriteToGps: Wrong parameter count");
    }

    return false;
}

/**
 * Checks if device has finished writing.
 *
 * Valid return values are:
 *   0 = idle      1 = working      2 = waiting for user input      3 = finished
 *
 * If user input is required the function fills the property MessageBoxXml with the message
 * On success the method updates the property GpsTransferSucceeded
 * This method gets called from the outside.
 * This method takes one parameter - the device number as int
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodFinishWriteToGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishWriteToGps();
            printFinishState("FinishWriteToGps", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["GpsTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                updateProgressBar("Write to GPS", 100);
            } else {
                updateProgressBar("Write to GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishWriteToGps: No working device specified");
        }
    }
    return false;
}


/**
 * Stops writing to the device
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodCancelWriteToGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice->cancelWriteToGps();
        return true;
    }
    return true;
}
/**
 * Fetches a detailed device description
 * The return value is a string(xml) with lots of details about the device functionality
 * This method gets called from the outside.
 * This method takes one parameter - the device number as int
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodDeviceDescription(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    if (argCount == 1) {
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            GpsDevice * device = devManager->getGpsDevice(deviceId);
            if (device != NULL) {
                string deviceDescr = device->getDeviceDescription();
                char *outStr = (char*)npnfuncs->memalloc(deviceDescr.size() + 1);
                memcpy(outStr, deviceDescr.c_str(), deviceDescr.size() + 1);
                result->type = NPVariantType_String;
                GETSTRING(result->value.stringValue) = outStr;
                GETSTRINGLENGTH(result->value.stringValue) = deviceDescr.size();
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("DeviceDescription: Device not found");
            }
        }
    } else {
        if (Log::enabledErr()) Log::err("DeviceDescription: Argument count is wrong");
    }

    return false;
}

/**
 * Receives the response to a messagebox answered by the user
 * This method gets called from the outside.
 * This method takes one parameter - the number of the button pressed by the user as int
 * @param obj
 * @param args[] contains all passed parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
bool methodRespondToMessageBox(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    if (messageList.size() > 0) {
        MessageBox * msg = messageList.front();
        if (msg != NULL) {

            if (argCount == 1) {
                if (args[0].type == NPVariantType_Int32) {
                    msg->responseReceived(args[0].value.intValue);
                } else {
                    if (Log::enabledErr()) Log::err("methodRespondToMessageBox: Unable to handle responses other than INT");
                }
            } else {
                if (Log::enabledErr()) Log::err("methodRespondToMessageBox: Wrong parameter count");
            }
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
        messageList.erase(messageList.begin());
        propertyList["MessageBoxXml"].stringValue = "";
        return true;

    } else {
        if (Log::enabledErr()) Log::err("Received a response to a messagebox that no longer exists !?");
    }
    return false;
}


bool methodStartReadFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
    updateProgressBar("Read from GPS", 0);
    if (argCount >= 2) {
        int deviceId = getIntParameter(args, 0, -1);
        string dataToRead = getStringParameter(args, 1, "");

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFitnessData(dataToRead);
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartReadFitnessData: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadFitnessData: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartReadFitnessData: Wrong parameter count");
    }

    return false;
}

bool methodFinishReadFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result)
{
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadFitnessData();
            printFinishState("FinishReadFitnessData", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                string tcdData = currentWorkingDevice->getFitnessData();
                propertyList["TcdXml"].stringValue = tcdData;
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData, "data.xml.gz");
                debugOutputPropertyToFile("TcdXml");
                updateProgressBar("Read from GPS", 100);
            } else {
                updateProgressBar("Read from GPS", currentWorkingDevice->getProgress());
            }
            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessData: No working device specified");
        }
    }
    return false;
}

bool methodStartReadFitnessDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    updateProgressBar("Read FITDIR from GPS", 0);
    if (argCount >= 2) { // What is the second parameter for ? "FitnessHistory"
        int deviceId = getIntParameter(args, 0, -1);
        string dataTypeName = getStringParameter(args, 1, "");

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFitnessDirectory(dataTypeName);
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartReadFitnessDirectory: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadFitnessDirectory: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartReadFitnessDirectory: Wrong parameter count");
    }

    return false;
}

bool methodStartReadFITDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount >= 1) {
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFITDirectory();
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartReadFITDirectory: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadFITDirectory: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartReadFITDirectory: Wrong parameter count");
    }

    return false;
}

bool methodFinishReadFITDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadFITDirectory();
            printFinishState("FinishReadFITDirectory", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                propertyList["DirectoryListingXml"].stringValue = currentWorkingDevice->getFITData();
                debugOutputPropertyToFile("DirectoryListingXml");
                updateProgressBar("Read FITDIR from GPS", 100);
            } else {
                updateProgressBar("Read FITDIR from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFITDirectory: No working device specified");
        }
    }
    return false;
}

bool methodCancelReadFITDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice -> cancelReadFITDirectory();
    }
    return true;
}

bool methodStartReadFitnessDetail(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    //  StartReadFitnessDetail(0,"FitnessHistory","2010-02-27T14:23:46Z")
    updateProgressBar("Read fitness detail from GPS", 0);
    if (argCount >= 2) { // What is the second parameter for ? "FitnessHistory"
        int deviceId = getIntParameter(args, 0, -1);
        string id = "";

        id = getStringParameter(args, 2, "");
        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFitnessDetail(id);
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartReadFitnessDirectory: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadFitnessDirectory: Unable to determine device id");
        }
    } else {
        if (Log::enabledErr()) Log::err("StartReadFitnessDirectory: Wrong parameter count");
    }

    return false;
}

bool methodFinishReadFitnessDetail(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadFitnessDetail();
            printFinishState("FinishReadFitnessDetail", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                string tcdData = currentWorkingDevice->getFitnessData();
                propertyList["TcdXml"].stringValue = tcdData;
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData, "data.xml.gz");
                debugOutputPropertyToFile("TcdXml");
                updateProgressBar("Read fitness detail from GPS", 100);
            } else {
                updateProgressBar("Read fitness detail from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessDetail: No working device specified");
        }
    }
    return false;
}

bool methodStartReadFromGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    updateProgressBar("Read from GPS", 0);
    if (argCount >= 1) {
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFromGps();
                return true;
            } else {
                if (Log::enabledInfo()) Log::info("StartReadFromGps: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadFromGps: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartReadFromGps: Wrong parameter count");
    }

    return false;
}

bool methodFinishReadFromGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadFromGps();
            printFinishState("FinishReadFromGps", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["GpsTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                string gpxdata = currentWorkingDevice->getGpxData();
                propertyList["GpsXml"].stringValue = gpxdata;
                debugOutputPropertyToFile("GpsXml");
                updateProgressBar("Read from GPS", 100);
            } else {
                updateProgressBar("Read from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessDetail: No working device specified");
        }
    }
    return false;
}

bool methodCancelReadFromGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        Log::dbg("Calling cancel read from gps");

        currentWorkingDevice->cancelReadFromGps();
    }
    return true;
}

bool methodCancelReadFitnessDetail(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice -> cancelReadFitnessDetail();
    }
    return true;
}

bool methodGetBinaryFile(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {

    if ((argCount < 2) || (argCount > 3)) {
        Log::err("GetBinaryFile: Wrong parameter count. Three parameter required! (DeviceID, Filename, [Compress])");
        return false;
    }

    int deviceId = getIntParameter(args, 0, -1);
    if (deviceId != -1) {
        GpsDevice * device = devManager->getGpsDevice(deviceId);
        if (device != NULL) {
            string fileName = getStringParameter(args,1,"");
            bool doCompress = false;
            if (argCount == 3) { doCompress = getBoolParameter(args,2,false); }
            string binaryData = device->getBinaryFile(fileName);
            string fileNameOnly = basename(fileName.c_str());
            if (doCompress) {
                binaryData = compressStringData(binaryData, fileNameOnly + ".gz");
            } else {
                std::stringstream outstream;
                std::stringstream binaryDataStream;
                binaryDataStream << binaryData;
                outstream << "begin-base64 644 " << fileNameOnly << endl;
                encodeBase64(&binaryDataStream,&outstream, 76);
                outstream << endl << "====" << endl;
                binaryData = outstream.str();
            }
            char *outStr = (char*)npnfuncs->memalloc(binaryData.size() + 1);
            memcpy(outStr, binaryData.c_str(), binaryData.size() + 1);
            result->type = NPVariantType_String;
            GETSTRING(result->value.stringValue) = outStr;
            GETSTRINGLENGTH(result->value.stringValue) = binaryData.size();
            return true;
        } else {
            Log::err("GetBinaryFile: No device with this ID!");
        }
    } else {
        Log::err("GetBinaryFile: Device ID is invalid");
    }
    return false;
}

bool methodStartDownloadData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount != 2) {
        Log::err("StartDownloadData: Wrong parameter count. Two parameter required! (gpsDataString, DeviceId)");
        return false;
    }

    updateProgressBar("Download to GPS", 0);
    int deviceId = getIntParameter(args, 1, -1);
    if (deviceId != -1) {
        currentWorkingDevice = devManager->getGpsDevice(deviceId);
        if (currentWorkingDevice != NULL) {
            string gpsDataString = getStringParameter(args,0,"");
            int urlsFound = currentWorkingDevice->startDownloadData(gpsDataString);
            if (urlsFound > 0) {
                string urlToDownload = currentWorkingDevice->getNextDownloadDataUrl();
                if (urlToDownload.length() > 0) {
                    if (Log::enabledDbg()) { Log::dbg("Requesting download for URL: "+urlToDownload); }

                    if (NPERR_NO_ERROR != npnfuncs->geturlnotify(inst, urlToDownload.c_str(), NULL, NULL)) {
                        Log::err("Unable to get url: "+urlToDownload);
                        return false;
                    }
                    return true;
                }
            } else {
                Log::err("StartDownloadData: No URLs found to download");
            }
        } else {
            Log::err("StartDownloadData: Unknown Device ID");
        }
    } else {
        Log::err("StartDownloadData: Device ID is invalid");
    }
    return false;
}

bool methodFinishDownloadData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishDownloadData();
            printFinishState("FinishDownloadData", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["DownloadDataSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                updateProgressBar("Download to GPS", 100);
            } else {
                updateProgressBar("Download to GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishDownloadData: No working device specified");
        }
    }
    return false;
}


bool methodFinishReadFitnessDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadFitnessDirectory();
            printFinishState("FinishReadFitnessDirectory", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                string tcdData = currentWorkingDevice->getFitnessData();
                propertyList["TcdXml"].stringValue = tcdData;
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData, "data.xml.gz");
                debugOutputPropertyToFile("TcdXml");
                updateProgressBar("Read Fitness Directory from GPS", 100);
            } else {
                updateProgressBar("Read Fitness Directory from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessData: No working device specified");
        }
    }
    return false;
}

bool methodCancelReadFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        Log::dbg("Calling cancel read fitness data");

        currentWorkingDevice->cancelReadFitnessData();
    }
    return true;
}

bool methodStartWriteFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount != 2) {
        Log::err("StartWriteFitnessData: Wrong parameter count. Two parameter required! (deviceNumber, dataTypeName)");
        return false;
    }

    updateProgressBar("Write fitness data to GPS", 0);
    int deviceId = getIntParameter(args, 0, -1);
    if (deviceId != -1) {
        currentWorkingDevice = devManager->getGpsDevice(deviceId);
        if (currentWorkingDevice != NULL) {
            string dataTypeName = getStringParameter(args,1,"");
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->startWriteFitnessData(propertyList["FileName"].stringValue, propertyList["TcdXml"].stringValue, dataTypeName);
            return true;
        } else {
            Log::err("StartWriteFitnessData: Unknown Device ID");
        }
    } else {
        Log::err("StartWriteFitnessData: Device ID is invalid");
    }
    return false;
}

bool methodFinishWriteFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishWriteFitnessData();
            printFinishState("FinishWriteFitnessData", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                //TODO: Is this the correct property?
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                updateProgressBar("Write fitness data to GPS", 100);
            } else {
                updateProgressBar("Write fitness data to GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishWriteFitnessData: No working device specified");
        }
    }
    return false;
}

bool methodCancelWriteFitnessData(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        Log::dbg("Calling CancelWriteFitnessData");

        currentWorkingDevice->cancelWriteFitnessData();
    }
    return true;
}

bool methodBytesAvailable(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount != 2) {
        Log::err("BytesAvailable: Wrong parameter count. Two parameter required! (deviceNumber, relativeFilePath)");
        return false;
    }

    int deviceId = getIntParameter(args, 0, -1);
    if (deviceId != -1) {
        currentWorkingDevice = devManager->getGpsDevice(deviceId);
        if (currentWorkingDevice != NULL) {
            string relativeFilePath = getStringParameter(args, 1, "");
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->bytesAvailable(relativeFilePath);
            return true;
        } else {
            Log::err("BytesAvailable: Unknown Device ID");
        }
    } else {
        Log::err("BytesAvailable: Device ID is invalid");
    }
    return false;
}



bool methodStartReadableFileListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount >= 4) {
        int deviceId = getIntParameter(args, 0, -1);
        string dataTypeName = getStringParameter(args, 1, "");
        string fileTypeName = getStringParameter(args, 2, "");
        bool computeMD5 = getBoolParameter(args, 3, false);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                if (currentWorkingDevice->startReadableFileListing(dataTypeName,fileTypeName,computeMD5) == 1) {
                    return true;
                } else {
                    // Not a file based device!
                    return false;
                }
            } else {
                if (Log::enabledInfo()) Log::info("StartReadableFileListing: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartReadableFileListing: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartReadableFileListing: Wrong parameter count");
    }

    return false;
}

bool methodFinishReadableFileListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishReadableFileListing();
            printFinishState("FinishReadableFileListing", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                propertyList["DirectoryListingXml"].stringValue = currentWorkingDevice->getDirectoryListingXml();
                debugOutputPropertyToFile("DirectoryListingXml");
                updateProgressBar("ReadableFileListing from GPS", 100);
            } else {
                updateProgressBar("ReadableFileListing from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadableFileListing: No working device specified");
        }
    }
    return false;
}

bool methodCancelReadableFileListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice -> cancelReadableFileListing();
    }
    return true;
}


bool methodStartDirectoryListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount >= 3) {
        int deviceId = getIntParameter(args, 0, -1);
        string devicePath = getStringParameter(args, 1, "");
        bool computeMD5 = getBoolParameter(args, 2, false);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                if (currentWorkingDevice->startDirectoryListing(devicePath,computeMD5) == 1) {
                    return true;
                } else {
                    // Not a file based device!
                    return false;
                }
            } else {
                if (Log::enabledInfo()) Log::info("StartDirectoryListing: Device not found");
            }
        } else {
            if (Log::enabledErr()) Log::err("StartDirectoryListing: Unable to determine device id");
        }

    } else {
        if (Log::enabledErr()) Log::err("StartDirectoryListing: Wrong parameter count");
    }

    return false;
}

bool methodFinishDirectoryListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
/* Return Values are
    0 = idle
    1 = working
    2 = waiting for user input
    3 = finished
*/
    if (messageList.size() > 0) {
        // Push messages first
        MessageBox * msg = messageList.front();
        if (msg != NULL) {
            propertyList["MessageBoxXml"].stringValue = msg->getXml();
            result->type = NPVariantType_Int32;
            result->value.intValue = 2; /* waiting for user input */
            return true;
        } else {
            if (Log::enabledErr()) Log::err("A null MessageBox is blocking the messages - fix the code!");
        }
    } else {
        if (currentWorkingDevice != NULL) {
            result->type = NPVariantType_Int32;
            result->value.intValue = currentWorkingDevice->finishDirectoryListing();
            printFinishState("FinishDirectoryListing", result->value.intValue);
            if (result->value.intValue == 2) { // waiting for user input
                messageList.push_back(currentWorkingDevice->getMessage());
                MessageBox * msg = messageList.front();
                if (msg != NULL) {
                    propertyList["MessageBoxXml"].stringValue = msg->getXml();
                }
            } else if (result->value.intValue == 3) { // transfer finished
                propertyList["FitnessTransferSucceeded"].intValue = currentWorkingDevice->getTransferSucceeded();
                propertyList["DirectoryListingXml"].stringValue = currentWorkingDevice->getDirectoryListingXml();
                debugOutputPropertyToFile("DirectoryListingXml");
                updateProgressBar("DirectoryListing from GPS", 100);
            } else {
                updateProgressBar("DirectoryListing from GPS", currentWorkingDevice->getProgress());
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishDirectoryListing: No working device specified");
        }
    }
    return false;
}

bool methodCancelDirectoryListing(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice -> cancelDirectoryListing();
    }
    return true;
}

bool methodParentDevice(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount >= 1) {
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            // TODO: Since version 3.0.1.0 it is possible to have parent devices
            // Currently the plugin does not return any child devices, which is why
            // this function always returns -1 (no child device)
            result->type = NPVariantType_Int32;
            result->value.intValue = -1;
            if (Log::enabledDbg()) {
                stringstream ss;
                ss << "ParentDevice for device " << deviceId << " - returning 'device has no parent device'";
                Log::dbg(ss.str());
            }
            return true;
        } else {
            if (Log::enabledErr()) Log::err("ParentDevice: Unable to determine device id (first parameter)");
        }
    } else {
        if (Log::enabledDbg()) { Log::dbg("Wrong argument count for ParentDevice"); }
    }
    return false;
}

/**
 * Initializes the Property List and Function List that are accessible from the outside
 */
void initializePropertyList() {
	// Properties
	propertyList.clear();
	Property value;
	value.writeable = false;
	value.type = NPVariantType_String;
	value.stringValue = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<Requests xmlns=\"http://www.garmin.com/xmlschemas/PcSoftwareUpdate/v2\">\n\n<Request>\n<PartNumber>006-A0160-00</PartNumber>\n<Version>\n<VersionMajor>3</VersionMajor>\n<VersionMinor>0</VersionMinor>\n<BuildMajor>1</BuildMajor>\n<BuildMinor>0</BuildMinor>\n<BuildType>Release</BuildType>\n</Version>\n<LanguageID>0</LanguageID>\n</Request>\n\n</Requests>\n";
	propertyList["VersionXml"] = value;
	value.stringValue = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<ProgressWidget xmlns=\"http://www.garmin.com/xmlschemas/PluginAPI/v1\">\n<Title>GarminPlugin Status not yet implemented</Title>\n<Text></Text>\n<Text></Text>\n<Text>0% complete</Text><ProgressBar Type=\"Percentage\" Value=\"0\"/></ProgressWidget>\n";
	propertyList["ProgressXml"] = value;
	value.stringValue = "";
	propertyList["MessageBoxXml"] = value;

	value.stringValue = "";
	propertyList["TcdXmlz"] = value; // Compressed

	value.stringValue = "";
	propertyList["DirectoryListingXml"] = value;

    // can be written from the outside
    value.writeable = true;
    value.stringValue = "";
	propertyList["GpsXml"] = value;
    value.stringValue = "";
	propertyList["FileName"] = value;
	value.stringValue = "";
	propertyList["TcdXml"] = value;
    value.writeable = false;

	value.type = NPVariantType_Int32;
	value.intValue = 0;
	propertyList["GpsTransferSucceeded"] = value;
	propertyList["Locked"] = value; // unlock

	value.intValue = 1;
	propertyList["FitnessTransferSucceeded"] = value;

	value.intValue = 1;
	propertyList["DownloadDataSucceeded"] = value;

	// Functions
	pt2Func fooPointer = &methodDevicesXmlString;
	methodList["DevicesXmlString"] = fooPointer;
	fooPointer = &methodUnlock;
	methodList["Unlock"] = fooPointer;
	fooPointer = &methodStartFindDevices;
	methodList["StartFindDevices"] = fooPointer;
	fooPointer = &methodCancelFindDevices;
	methodList["CancelFindDevices"] = fooPointer;
    fooPointer = &methodFinishFindDevices;
    methodList["FinishFindDevices"] = fooPointer;

    fooPointer = &methodDeviceDescription;
    methodList["DeviceDescription"] = fooPointer;

    fooPointer = &methodStartWriteToGps;
    methodList["StartWriteToGps"] = fooPointer;
    fooPointer = &methodFinishWriteToGps;
    methodList["FinishWriteToGps"] = fooPointer;
    fooPointer = &methodCancelWriteToGps;
    methodList["CancelWriteToGps"] = fooPointer;

    fooPointer = &methodRespondToMessageBox;
    methodList["RespondToMessageBox"] = fooPointer;

    fooPointer = &methodStartReadFitnessData;
    methodList["StartReadFitnessData"] = fooPointer;

    fooPointer = &methodFinishReadFitnessData;
    methodList["FinishReadFitnessData"] = fooPointer;

    fooPointer = &methodStartReadFITDirectory;
    methodList["StartReadFITDirectory"] = fooPointer;

    fooPointer = &methodFinishReadFITDirectory;
    methodList["FinishReadFITDirectory"] = fooPointer;

    fooPointer = &methodCancelReadFITDirectory;
    methodList["CancelReadFITDirectory"] = fooPointer;

    fooPointer = &methodStartReadFitnessDirectory;
    methodList["StartReadFitnessDirectory"] = fooPointer;

    fooPointer = &methodFinishReadFitnessDirectory;
    methodList["FinishReadFitnessDirectory"] = fooPointer;

    fooPointer = &methodCancelReadFitnessData;
    methodList["CancelReadFitnessData"] = fooPointer;

    fooPointer = &methodStartReadFitnessDetail;
    methodList["StartReadFitnessDetail"] = fooPointer;

    fooPointer = &methodFinishReadFitnessDetail;
    methodList["FinishReadFitnessDetail"] = fooPointer;

    fooPointer = &methodCancelReadFitnessDetail;
    methodList["CancelReadFitnessDetail"] = fooPointer;

    fooPointer = &methodStartReadFromGps;
    methodList["StartReadFromGps"] = fooPointer;
    fooPointer = &methodFinishReadFromGps;
    methodList["FinishReadFromGps"] = fooPointer;
    fooPointer = &methodCancelReadFromGps;
    methodList["CancelReadFromGps"] = fooPointer;

    fooPointer = &methodGetBinaryFile;
    methodList["GetBinaryFile"] = fooPointer;

    fooPointer = &methodStartDownloadData;
    methodList["StartDownloadData"] = fooPointer;

    fooPointer = &methodFinishDownloadData;
    methodList["FinishDownloadData"] = fooPointer;

    fooPointer = &methodStartWriteFitnessData;
    methodList["StartWriteFitnessData"] = fooPointer;
    fooPointer = &methodFinishWriteFitnessData;
    methodList["FinishWriteFitnessData"] = fooPointer;
    fooPointer = &methodCancelWriteFitnessData;
    methodList["CancelWriteFitnessData"] = fooPointer;

    fooPointer = &methodBytesAvailable;
    methodList["BytesAvailable"] = fooPointer;

    // Directory File listing of file based devices
    fooPointer = &methodStartReadableFileListing;
    methodList["StartReadableFileListing"] = fooPointer;
    fooPointer = &methodFinishReadableFileListing;
    methodList["FinishReadableFileListing"] = fooPointer;
    fooPointer = &methodCancelReadableFileListing;
    methodList["CancelReadableFileListing"] = fooPointer;

    // Directory File listing of file based devices (is marked as experimental but still used in myGarmin portal
    fooPointer = &methodStartDirectoryListing;
    methodList["StartDirectoryListing"] = fooPointer;
    fooPointer = &methodFinishDirectoryListing;
    methodList["FinishDirectoryListing"] = fooPointer;
    fooPointer = &methodCancelDirectoryListing;
    methodList["CancelDirectoryListing"] = fooPointer;

    // Get Parent Device (new in 3.0.1.0)
    fooPointer = &methodParentDevice;
    methodList["ParentDevice"] = fooPointer;

}

/**
 * The browser checks if the function actually exists by calling this function.
 * This function returns true if the methodList was initialized with a function by that name.
 * @param obj unknown
 * @param methodname name of the method
 * @return boolean. Return true if method exists
 */
static bool hasMethod(NPObject* obj, NPIdentifier methodName) {
    string name = npnfuncs->utf8fromidentifier(methodName);
	//if (Log::enabledDbg()) Log::dbg("hasMethod: "+name);

	map<string,pt2Func>::iterator it;

	it = methodList.find(name);
	if (it != methodList.end()) {
		return true;
	} else {
        if (Log::enabledInfo()) Log::info("hasMethod: "+name+" not found");
	}

	return false;

}

/**
 * Prints the parameter that are used while calling a function
 * Just good for debugging.
 * @param name function name
 * @param args an array with the passed arguments
 * @param argCount number of arguments passed in args
 */
void printParameter(string name, const NPVariant args[], uint32_t argCount) {
    std::stringstream ss;
    ss << name << "(";

    for (uint32_t i = 0; i < argCount; i++) {
        if (args[i].type == NPVariantType_Int32) {
            ss << "" << args[i].value.intValue;
        } else if (args[i].type == NPVariantType_String) {
            ss << "\"" << getStringParameter(args, i, "") << "\"";
        } else if (args[i].type == NPVariantType_Bool) {
            if (args[i].value.boolValue) {
                ss << "true";
            } else {
                ss << "false";
            }
        } else if (args[i].type == NPVariantType_Double) {
            ss << "" << args[i].value.doubleValue;
        } else if (args[i].type == NPVariantType_Null) {
            ss << "null";
        } else {
            ss << " ? ";
        }
        if (i < argCount-1) { ss << ","; }
    }
    ss << ")";
    string functionCall;
    ss >> functionCall;
    Log::dbg(functionCall);
}

/**
 * Gets called by the browser when javascript calls a method.
 * Just good for debugging.
 * @param obj
 * @param methodName name of the function that shall be called
 * @param args array with the parameters
 * @param argCount number of passed parameters
 * @param result store return value here
 * @return boolean. Return true if successful
 */
static bool invoke(NPObject* obj, NPIdentifier methodName, const NPVariant *args, uint32_t argCount, NPVariant *result) {
    string name = npnfuncs->utf8fromidentifier(methodName);
    if (Log::enabledDbg()) printParameter(name, args, argCount);

	map<string,pt2Func>::iterator it;
	it = methodList.find(name);
	if (it != methodList.end()) {
		pt2Func functionPointer = it->second;
		return (*functionPointer)(obj, args, argCount, result);
	} else {
        std::stringstream ss;
        ss << "Method "  << name << " not found";
        Log::err(ss.str());
	}

	// aim exception handling
	npnfuncs->setexception(obj, "exception during invocation");
	return false;
}


/**
 * Gets called by the browser to check if a property exists
 * @param obj
 * @param propertyName name of the property
 * @return boolean. Return true if property exists
 */
static bool hasProperty(NPObject *obj, NPIdentifier propertyName) {

    string name = npnfuncs->utf8fromidentifier(propertyName);
    //if (Log::enabledDbg()) Log::dbg("hasProperty: "+name);

	map<string,Property>::iterator it;

	it = propertyList.find(name);
	if (it != propertyList.end()) {
		return true;
	} else {
        if (Log::enabledInfo()) Log::info("hasProperty: "+name+" not found");
	}

	return false;
}

/**
 * Gets called by the browser to get the content of a property
 * @param obj
 * @param propertyName name of the property
 * @param result must be filled with the content of the property
 * @return boolean. Return true if successful
 */
static bool getProperty(NPObject *obj, NPIdentifier propertyName, NPVariant *result) {
    string name = npnfuncs->utf8fromidentifier(propertyName);

	map<string,Property>::iterator it;
	it = propertyList.find(name);
	if (it != propertyList.end()) {
	    stringstream dbgOut;
		Property storedProperty = it->second;
		result->type = storedProperty.type;
		if (NPVARIANT_IS_INT32(storedProperty)) {
			INT32_TO_NPVARIANT(storedProperty.intValue, *result);
			dbgOut << storedProperty.intValue;
		} else if (NPVARIANT_IS_STRING(storedProperty)) {
			// We have:
			// std::string str as the string to store
			// NPVariant *dst as the destination NPVariant
			char *outStr = (char*)npnfuncs->memalloc(storedProperty.stringValue.size() + 1);
			memcpy(outStr, storedProperty.stringValue.c_str(), storedProperty.stringValue.size() + 1);
			result->type = NPVariantType_String;
			GETSTRING(result->value.stringValue) = outStr;
			GETSTRINGLENGTH(result->value.stringValue) = storedProperty.stringValue.size();

            if (storedProperty.stringValue.size() > 50) {
                dbgOut << storedProperty.stringValue.substr(0,47) << "...";
            } else {
                dbgOut << storedProperty.stringValue;
            }
		} else {
            if (Log::enabledErr()) Log::err("getProperty "+name+": Type not yet implemented");
			return false;
		}
		if (Log::enabledDbg()) Log::dbg("getProperty: "+name +" = ["+dbgOut.str()+"]");
		return true;
	} else {
        if (Log::enabledInfo()) Log::info("getProperty: Property "+name+" not found");
	}

	return false;
}

/**
 * Gets called by the browser to set the content of a property
 * @param obj
 * @param propertyName name of the property
 * @param value the new content for the property
 * @return boolean. Return true if successful
 */
static bool setProperty(NPObject *obj, NPIdentifier propertyName, const NPVariant *value) {
    string name = npnfuncs->utf8fromidentifier(propertyName);
    if (Log::enabledDbg()) Log::dbg("setProperty "+name);

	map<string,Property>::iterator it;
	it = propertyList.find(name);
	if (it != propertyList.end()) {
		Property storedProperty = it->second;
		if (storedProperty.writeable) {
		    storedProperty.type = value->type;
            if (value->type == NPVariantType_String) {
                storedProperty.stringValue = getStringFromNPString(value->value.stringValue);
                propertyList[name] = storedProperty; // store
                return true;
            } else if (value->type == NPVariantType_Int32) {
                storedProperty.intValue = value->value.intValue;
                propertyList[name] = storedProperty; // store
                return true;
            } else {
                if (Log::enabledErr()) Log::err("setProperty: Unsupported type - must be implemented");
            }
		} else {
            if (Log::enabledInfo()) Log::info("setProperty: Property ist read-only");
		}
	} else {
	    if (Log::enabledInfo()) Log::info("setProperty: Property "+name+" not found");
	}


	return false;
}

/**
 * Functions that are accessible from the outside by the browser
 */
static NPClass npcRefObject = {
	NP_CLASS_STRUCT_VERSION,
	NULL,        /*allocate*/
	NULL,        /*deallocate*/
	NULL,        /*invalidate */
	hasMethod,
	invoke,
	NULL,        /*invokeDefault*/
	hasProperty,
	getProperty,
	setProperty,
	NULL,        /*removeProperty*/
};

/**
 * Gets called by the browser to create a new instance
 * @param pluginType ?
 * @param instance ?
 * @param mode ?
 * @param argc ?
 * @param argn ?
 * @param argv ?
 * @param saved ?
 */
static NPError nevv(NPMIMEType pluginType, NPP instance, uint16_t mode, int16_t argc, char *argn[], char *argv[], NPSavedData *saved) {
    instanceCount++;
	inst = instance;
	if (Log::enabledDbg()) Log::dbg("new");

    if(!so) {
        so = npnfuncs->createobject(instance, &npcRefObject);
    }

    if (Log::enabledDbg()) Log::dbg("Overwriting Garmin Javascript Browser detection!");
    NPString str;
    char buf[1000];


    NPObject* windowObject = NULL;
    NPError err = npnfuncs->getvalue(inst, NPNVWindowNPObject, &windowObject);
    if (err != NPERR_NO_ERROR) {
        Log::err("Error fetching NPNVWindowNPObject");
        return NPERR_NO_ERROR;
    }
    NPVariant ret;

    // This is needed for google chrome and firefox to get rid of the browser check
    string javascriptCode =  "var garminOverwriteBrowserDetectRunCount = 0;\
                              var garminOverwriteBrowserDetect = function() {\
                                if(typeof(BrowserDetect.init) != \"undefined\"){\
                                  BrowserDetect.init = function() { };\
                                }\
                                if(typeof(BrowserDetect.OS) != \"undefined\"){\
                                    BrowserDetect.OS='Windows';\
                                    BrowserDetect.browser='Firefox';\
                                }\
                                garminOverwriteBrowserDetectRunCount++;\
                                if (garminOverwriteBrowserDetectRunCount < 20) {\
                                    setTimeout ( \"garminOverwriteBrowserDetect()\", 100 );\
                                }\
                              };\
                              garminOverwriteBrowserDetect();";
    memcpy(buf, javascriptCode.c_str(), sizeof(buf) > javascriptCode.size() ? javascriptCode.size(): sizeof(buf));
    GETSTRING(str) = buf;
    GETSTRINGLENGTH(str) = sizeof(buf) > javascriptCode.size() ? javascriptCode.size(): sizeof(buf);
    if (!npnfuncs->evaluate(inst, windowObject, &str, &ret)) {
        Log::err("Unable to execute javascript: "+javascriptCode);
    }

    if (Log::enabledDbg()) Log::dbg("End Overwriting Garmin Javascript Browser detection!");

    if (Log::enabledDbg()) {
        string userAgentStr = npnfuncs->uagent(inst);
        Log::dbg("User Agent: "+userAgentStr);

        NPIdentifier identifier = npnfuncs->getstringidentifier("location");
        NPVariant variantValue;
        bool b1 = npnfuncs->getproperty( inst, windowObject, identifier, &variantValue );
        if (b1) {
            NPObject *locationObj = variantValue.value.objectValue;
            identifier = npnfuncs->getstringidentifier( "href" );
            bool b2 = npnfuncs->getproperty( inst, locationObj, identifier, &variantValue );
            if (b2) {
                if (variantValue.type == NPVariantType_String) {
                    string href = getStringFromNPString(variantValue.value.stringValue);
                    Log::dbg("URL: "+href);
                }
            }
            npnfuncs->releaseobject(locationObj);
        }
    }

    npnfuncs->releaseobject(windowObject);

	return NPERR_NO_ERROR;
}

/**
 * Gets called by the browser to destroy an instance
 * @param instance ?
 * @param save ?
 * @return NPERR_NO_ERROR
 */
static NPError destroy(NPP instance, NPSavedData **save) {
	if (Log::enabledDbg()) Log::dbg("destroy");
	instanceCount--;
	if (instanceCount == 0) {
        if (Log::enabledDbg()) Log::dbg("destroy - last instance");
        if ((so) && (npnfuncs != NULL)) {
            npnfuncs->releaseobject(so);
            so = NULL;
        }
	}

	return NPERR_NO_ERROR;
}

/**
* Gets called by the browser to get values like pluginname and mime type
* @param instance ?
* @param variable What value to get
* @param value ?
* @return NPERR_NO_ERROR
*/
static NPError getValue(NPP instance, NPPVariable variable, void *value) {
	inst = instance;
	switch(variable) {
	default:
		if (Log::enabledDbg()) Log::dbg("getValue - default");
		return NPERR_GENERIC_ERROR;
	case NPPVpluginNameString:
        if (Log::enabledDbg()) Log::dbg("getvalue - name string");
		*((const char **)value) = pluginName;
		break;
	case NPPVpluginDescriptionString:
        if (Log::enabledDbg()) Log::dbg("getvalue - description string");
		*((const char **)value) = pluginDescription;
		break;
	case NPPVpluginScriptableNPObject:
        if (Log::enabledDbg()) Log::dbg("getvalue - scriptable object");
		if(!so) {
			so = npnfuncs->createobject(instance, &npcRefObject);
		}
        npnfuncs->retainobject(so);
		*(NPObject **)value = so;
		break;
	case NPPVpluginNeedsXEmbed:
        if (Log::enabledDbg()) Log::dbg("getvalue - xembed");
		*((bool *)value) = supportsXEmbed; // Support XEmbed (without Chrome does not work)
		break;
	}
	return NPERR_NO_ERROR;
}

/**
* Expected by Safari on Darwin
* @param instance ?
* @param ev ?
* @return NPERR_NO_ERROR
*/
static NPError /*  */
handleEvent(NPP instance, void *ev) {
	inst = instance;
	if (Log::enabledDbg()) Log::dbg("handleEvent");
	return NPERR_NO_ERROR;
}

/**
* Expected by Opera
* @param instance ?
* @param pNPWindow ?
* @return NPERR_NO_ERROR
*/
static NPError /* expected by Opera */
setWindow(NPP instance, NPWindow* pNPWindow) {
	inst = instance;
	if (Log::enabledDbg()) Log::dbg("setWindow");
	return NPERR_NO_ERROR;
}

static void nppUrlNotify(NPP instance, const char* url, NPReason reason, void* notifyData) {
	if (reason == NPRES_DONE) {
        if (Log::enabledDbg()) Log::dbg("nppUrlNotify: Request was finished.");

        if (currentWorkingDevice != NULL) {
            string urlToDownload = currentWorkingDevice->getNextDownloadDataUrl();
            if (urlToDownload.length() > 0) {
                if (Log::enabledDbg()) { Log::dbg("Requesting download for URL: "+urlToDownload); }
                if (NPERR_NO_ERROR != npnfuncs->geturlnotify(inst, urlToDownload.c_str(), NULL, NULL)) {
                    Log::err("Unable to get url: "+urlToDownload);
                }
            }
        }
	} else if (reason == NPRES_USER_BREAK) {
        Log::err("nppUrlNotify: User canceled request");
        if (currentWorkingDevice != NULL) {
            currentWorkingDevice->cancelDownloadData();
        }
	} else if (reason == NPRES_NETWORK_ERR) {
        Log::err("nppUrlNotify: Canceled because of Network Error");
        if (currentWorkingDevice != NULL) {
            currentWorkingDevice->cancelDownloadData();
        }
	} else {
        if (Log::enabledDbg()) Log::dbg("nppUrlNotify: Unknown notify reason!");
	}
}

static NPError nppNewStream(NPP instance, NPMIMEType type, NPStream*  stream, NPBool seekable, uint16_t* stype) {
	if (*stype == NP_NORMAL) {
        if (Log::enabledDbg()) {
            string url = stream->url;
            Log::dbg("nppNewStream Type: NP_NORMAL URL: "+url);
        }
        return NPERR_NO_ERROR;
	} else {
	    Log::err("nppNewStream: Unknown stream type!");
	}
	return NPERR_GENERIC_ERROR;
}

#define NPP_STREAM_BUFFER_SIZE 5120
static int32_t nppWriteReady(NPP instance, NPStream* stream) {
	if (Log::enabledDbg()) Log::dbg("nppWriteReady");
    return NPP_STREAM_BUFFER_SIZE;
}

static int32_t nppWrite(NPP instance, NPStream* stream, int32_t offset, int32_t len, void* buf) {
	if (Log::enabledDbg()) {
        stringstream ss;
        ss << "nppWrite Parameter: Offset: " << offset << " Length: " << len;
	    Log::dbg(ss.str());
	}

    if (currentWorkingDevice != NULL) {
        return currentWorkingDevice->writeDownloadData((char*)buf, len);
    } else {
        if (Log::enabledDbg()) Log::dbg("nppWrite: No working device!?");
    }
    return -1;
}

static NPError nppDestroyStream(NPP instance, NPStream* stream, NPReason  reason) {
    if (currentWorkingDevice != NULL) {
        if (reason == NPRES_DONE) {
            if (Log::enabledDbg()) Log::dbg("nppDestroyStream: Stream done");
            currentWorkingDevice->saveDownloadData();
        } else {
            currentWorkingDevice->cancelDownloadData();
            Log::err("nppDestroyStream: Download to device was canceled because of errors");
        }
    } else {
        if (Log::enabledDbg()) Log::dbg("nppDestroyStream: No working device!?");
    }
    return NPERR_NO_ERROR;
}


#ifdef __cplusplus
extern "C" {
#endif

/**
* Sets the entry points for the browser
* @param nppfuncs ?
* @return NPERR_NO_ERROR
*/
NPError OSCALL NP_GetEntryPoints(NPPluginFuncs *nppfuncs) {
	if (Log::enabledDbg()) Log::dbg("NP_GetEntryPoints");
	nppfuncs->version       = (NP_VERSION_MAJOR << 8) | NP_VERSION_MINOR;
	nppfuncs->newp          = nevv;
	nppfuncs->destroy       = destroy;
	nppfuncs->getvalue      = getValue;
	nppfuncs->event         = handleEvent;
	nppfuncs->setwindow     = setWindow;
	nppfuncs->urlnotify     = nppUrlNotify;
	nppfuncs->newstream     = nppNewStream;
	nppfuncs->writeready    = nppWriteReady;
	nppfuncs->write         = nppWrite;
	nppfuncs->destroystream = nppDestroyStream;

	return NPERR_NO_ERROR;
}

#ifndef HIBYTE
#define HIBYTE(x) ((((uint32_t)(x)) & 0xff00) >> 8)
#endif

/**
* The browser initializes the plugin by calling this function
* @param npnf ?
* @param nppfuncs ?
* @return NPERR_NO_ERROR
*/
NPError OSCALL NP_Initialize(NPNetscapeFuncs *npnf
			, NPPluginFuncs *nppfuncs
			)
{
	if(npnf == NULL)
		return NPERR_INVALID_FUNCTABLE_ERROR;

	if(HIBYTE(npnf->version) > NP_VERSION_MAJOR)
		return NPERR_INCOMPATIBLE_VERSION_ERROR;

	npnfuncs = npnf;
	NP_GetEntryPoints(nppfuncs);

    // Setup Configuration (which also initializes logging)
    if (confManager != NULL) {
        delete confManager;
    }
	confManager = new ConfigManager();
	confManager->readConfiguration();
    Log::getInstance()->setConfiguration(confManager->getConfiguration());

    // Check for browser support of XEmbed
    NPError err = NPERR_NO_ERROR;
    err = npnfuncs->getvalue(NULL, NPNVSupportsXEmbedBool,(void *)&supportsXEmbed);

    if (err != NPERR_NO_ERROR) {
        supportsXEmbed = false;
        Log::err("Error while asking for XEmbed support");
    }

    if (Log::enabledDbg()) {
        if (supportsXEmbed == false) {
            Log::dbg("Browser does not support XEmbed");
        } else {
            Log::dbg("Browser supports XEmbed");
        }
    }

	initializePropertyList();

    if (devManager != NULL) {
        delete devManager;
    }
	devManager = new DeviceManager();

	devManager->setConfiguration(confManager->getConfiguration());
	MessageBox * msg = confManager->getMessage();
	if (msg != NULL) {
	    messageList.push_back(msg);
	}
    if (Log::enabledDbg()) Log::dbg("NP_Initialize successfull");

	return NPERR_NO_ERROR;
}

/**
* The browser shuts down the plugin by calling this function
* @return NPERR_NO_ERROR
*/
NPError OSCALL NP_Shutdown() {
	if (Log::enabledDbg()) Log::dbg("NP_Shutdown");
	delete devManager;
	delete confManager;
	devManager = NULL;
	return NPERR_NO_ERROR;
}

/**
* The browser requests the mime description of the plugin with this function
* @return Mime description
*/
const char * NP_GetMIMEDescription(void) {
	if (Log::enabledDbg()) Log::dbg("NP_GetMIMEDescription");
	return pluginMimeDescription;
}

/**
* Needs to be present for WebKit based browsers
* @return NPERR_NO_ERROR
*/
NPError OSCALL NP_GetValue(void *npp, NPPVariable variable, void *value) {
	inst = (NPP)npp;
	return getValue((NPP)npp, variable, value);
}

#ifdef __cplusplus
}
#endif
