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
#include <npapi.h>
#if HAVE_NPFUNCTIONS_H
#include <npfunctions.h>
#else
#include <npupp.h>
#endif
#if HAVE_PRTYPES_H
	#include <prtypes.h>
#elif HAVE_ZIPSTUB_H
	#include <zipstub.h>
#endif
#include <npruntime.h>
#include <iostream>
#include <map>
#include "deviceManager.h"
#include "configManager.h"
#include "log.h"
#include <sstream>
#include "zlib.h"
#include <fstream>

#include <boost/iostreams/filtering_streambuf.hpp>
#include <boost/iostreams/copy.hpp>
#include <boost/iostreams/filter/gzip.hpp>
#include "boost/archive/iterators/base64_from_binary.hpp"
#include "boost/archive/iterators/transform_width.hpp"

#if HAVE_NEW_XULRUNNER
#define GETSTRING(_v)          ((_v).UTF8Characters)
#define GETSTRINGLENGTH(_v)    ((_v).UTF8Length)
#else
#define GETSTRING(_v)          ((_v).utf8characters)
#define GETSTRINGLENGTH(_v)    ((_v).utf8length)
#endif

using namespace std;

/**
 * A variable that stores the plugin name
 */
#if HAVE_NPFUNCTIONS_H
const char
#else
char
#endif
	 * pluginName = "Garmin Communicator";

/**
 * A variable that stores the plugin description (may contain HTML)
 */
#if HAVE_NPFUNCTIONS_H
const char
#else
char
#endif
 	* pluginDescription = "<a href=\"http://www.andreas-diesner.de/garminplugin/\">Garmin Communicator - Fake</a> plugin. Version 0.2.4";

/**
 * A variable that stores the mime description of the plugin.
 */
#if HAVE_NPFUNCTIONS_H
const char
#else
char
#endif
	 * pluginMimeDescription = "application/vnd-garmin.mygarmin:.Garmin Device Web Control";

/**
 * Manages all attached GPS Devices
 */
DeviceManager *devManager = NULL;

/**
 * Manages the plugin configuration
 */
ConfigManager * confManager = NULL;

/**
 * Unknown
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

typedef
   boost::archive::iterators::base64_from_binary<
        boost::archive::iterators::transform_width<string::const_iterator, 6, 8>
        > base64_t;

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
        std::string intValueStr = GETSTRING(args[pos].value.stringValue);
        Log::dbg("getIntParameter String: "+intValueStr);
        std::istringstream ss( intValueStr );
        ss >> intValue;
        /* Does not work
        if (! ss.good()) {
            [...]
        } */
    } else {
        std::ostringstream errTxt;
        errTxt << "Expected INT parameter at position " << pos << ". Found: " << getParameterTypeStr(args[pos]);
        if (Log::enabledErr()) Log::err(errTxt.str());
    }
    return intValue;
}

/**
 * Compresses a string using gzip and encodes it using base64
 * Use uudecode to unpack and then gunzip
 */
string compressStringData(const string text) {
    if (Log::enabledDbg()) {
        stringstream ss;
        ss << text.size();
        Log::dbg("Compressing content of string with length: " + ss.str());
    }
    std::stringstream decompressed;
    std::stringstream compressed;
    std::stringstream outstream;
    unsigned int bufferSize = 77;
    char * lineBuffer = new char [bufferSize];
    decompressed << text;

    boost::iostreams::filtering_streambuf<boost::iostreams::input> out;
    out.push(boost::iostreams::gzip_compressor());
    out.push(decompressed);
    boost::iostreams::copy(out, compressed);

    string bin = compressed.str();
    string enc(base64_t(bin.begin()), base64_t(bin.end()));

    // Maximum line length of 76 characters for a uuencoded file
    stringstream lineSplitter;
    lineSplitter << enc;
    outstream << "begin-base64 644 data.xml.gz";
    while (!lineSplitter.eof()) {
        lineSplitter.read (lineBuffer,bufferSize-1);
        unsigned int bytesRead = lineSplitter.gcount();
        if (bytesRead > 0) {
            lineBuffer[(bytesRead<bufferSize)? bytesRead : bufferSize-1] = 0;
            outstream << endl << lineBuffer;
        }
    }
    delete(lineBuffer);

    // uuencoded files must always have a line length that is a multiply of 4. So fill up the remaining bytes so that it is decompressable
    int fillUpToTripple = 4 - (enc.size() % 4);
    while (fillUpToTripple < 4) {
        outstream << "=";
        fillUpToTripple++;
    }

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
    return false;
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
    if (argCount >= 1) { // What is the second parameter for ? "FitnessHistory"
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFitnessData();
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
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData);
                debugOutputPropertyToFile("TcdXml");
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessData: No working device specified");
        }
    }
    return false;
}

bool methodStartReadFitnessDirectory(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (argCount >= 1) { // What is the second parameter for ? "FitnessHistory"
        int deviceId = getIntParameter(args, 0, -1);

        if (deviceId != -1) {
            currentWorkingDevice = devManager->getGpsDevice(deviceId);
            if (currentWorkingDevice != NULL) {
                result->type = NPVariantType_Int32;
                result->value.intValue = currentWorkingDevice->startReadFitnessDirectory();
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
    Log::err("Please implement me!");
    return true;
}

bool methodStartReadFitnessDetail(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    //  StartReadFitnessDetail(0,"FitnessHistory","2010-02-27T14:23:46Z")

    if (argCount >= 2) { // What is the second parameter for ? "FitnessHistory"
        int deviceId = getIntParameter(args, 0, -1);
        string id = "";

        if (args[2].type == NPVariantType_Int32) {
            id = args[2].value.intValue;
        } else if (args[2].type == NPVariantType_String) {
            id = GETSTRING(args[2].value.stringValue);
        } else {
            if (Log::enabledErr()) Log::err("StartReadFitnessDirectory: Expected STRING parameter 3");
        }

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
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData);
                debugOutputPropertyToFile("TcdXml");
            }

            return true;
        } else {
            if (Log::enabledInfo()) Log::info("FinishReadFitnessDetail: No working device specified");
        }
    }
    return false;
}

bool methodStartReadFromGps(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
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
        return true;
    }
    return false;
}

bool methodCancelReadFitnessDetail(NPObject *obj, const NPVariant args[], uint32_t argCount, NPVariant * result) {
    if (currentWorkingDevice != NULL) {
        currentWorkingDevice -> cancelReadFitnessDetail();
    }
    return true;
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
                propertyList["TcdXmlz"].stringValue = compressStringData(tcdData);
                debugOutputPropertyToFile("TcdXml");
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
        return true;
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
	value.type = NPVariantType_String;
	value.stringValue = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<Requests xmlns=\"http://www.garmin.com/xmlschemas/PcSoftwareUpdate/v2\">\n\n<Request>\n<PartNumber>006-A0160-00</PartNumber>\n<Version>\n<VersionMajor>2</VersionMajor>\n<VersionMinor>9</VersionMinor>\n<BuildMajor>2</BuildMajor>\n<BuildMinor>0</BuildMinor>\n<BuildType>Release</BuildType>\n</Version>\n<LanguageID>0</LanguageID>\n</Request>\n\n</Requests>\n";
	propertyList["VersionXml"] = value;
	value.stringValue = "<?xml version=\"1.0\" encoding=\"UTF-8\" standalone=\"no\" ?>\n<ProgressWidget xmlns=\"http://www.garmin.com/xmlschemas/PluginAPI/v1\">\n<Title>GarminPlugin Status not yet implemented</Title>\n<Text></Text>\n<Text></Text>\n<Text>90% complete</Text><ProgressBar Type=\"Percentage\" Value=\"90\"/></ProgressWidget>\n";
	propertyList["ProgressXml"] = value;
	value.stringValue = "";
	propertyList["MessageBoxXml"] = value;

	value.stringValue = "";
	propertyList["TcdXml"] = value;
	value.stringValue = "";
	propertyList["TcdXmlz"] = value; // Compressed

    // can be written from the outside
    value.writeable = true;
    value.stringValue = "";
	propertyList["GpsXml"] = value;
    value.stringValue = "";
	propertyList["FileName"] = value;
    value.writeable = false;

	value.type = NPVariantType_Int32;
	value.intValue = 0;
	propertyList["GpsTransferSucceeded"] = value;
	propertyList["Locked"] = value; // unlock

	value.intValue = 1;
	propertyList["FitnessTransferSucceeded"] = value;


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
            ss << args[i].value.intValue;
        } else if (args[i].type == NPVariantType_String) {
            ss << "\"" << GETSTRING(args[i].value.stringValue) << "\"";
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
                storedProperty.stringValue = GETSTRING(value->value.stringValue);
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
	inst = instance;
	if (Log::enabledDbg()) Log::dbg("new");

	return NPERR_NO_ERROR;
}

/**
 * Gets called by the browser to destroy an instance
 * @param instance ?
 * @param save ?
 * @return NPERR_NO_ERROR
 */
static NPError destroy(NPP instance, NPSavedData **save) {
	if(so)
		npnfuncs->releaseobject(so);
	so = NULL;
	if (Log::enabledDbg()) Log::dbg("destroy");
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
#if HAVE_NPFUNCTIONS_H
		*((const char **)value) = pluginName;
#else
		*((char **)value) = pluginName;
#endif
		break;
	case NPPVpluginDescriptionString:
        if (Log::enabledDbg()) Log::dbg("getvalue - description string");
#if HAVE_NPFUNCTIONS_H
		*((const char **)value) = pluginDescription;
#else
		*((char **)value) = pluginDescription;
#endif
		break;
	case NPPVpluginScriptableNPObject:
        if (Log::enabledDbg()) Log::dbg("getvalue - scriptable object");
		if(!so)
			so = npnfuncs->createobject(instance, &npcRefObject);
		npnfuncs->retainobject(so);
		*(NPObject **)value = so;
		break;
	case NPPVpluginNeedsXEmbed:
        if (Log::enabledDbg()) Log::dbg("getvalue - xembed");
		*((PRBool *)value) = PR_FALSE;
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

	initializePropertyList();

    if (devManager != NULL) {
        delete devManager;
    }
	devManager = new DeviceManager();

    if (confManager != NULL) {
        delete confManager;
    }
	confManager = new ConfigManager();
	confManager->readConfiguration();
	devManager->setConfiguration(confManager->getConfiguration());
	MessageBox * msg = confManager->getMessage();
	if (msg != NULL) {
	    messageList.push_back(msg);
	}
    Log::getInstance()->setConfiguration(confManager->getConfiguration());
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
#if HAVE_NPFUNCTIONS_H
const
#endif
char * NP_GetMIMEDescription(void) {
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
