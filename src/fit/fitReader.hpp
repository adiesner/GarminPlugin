/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2013 <garminplugin [AT] andreas.diesner [DOT] de>
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

#ifndef FITREADER_H
#define FITREADER_H

#include <iostream>
#include <fstream>
#include <string>
#include <vector>
#include "fitMsg.hpp"
#include "fitMsg_Listener.hpp"

using namespace std;

#define FIT_MAJOR_VERSION               1
#define FIT_MINOR_VERSION               0
#define FIT_PROFILE_MAJOR_VERSION       1
#define FIT_PROFILE_MINOR_VERSION       0

#define MAX_LOCAL_MSG                   16
#define MESSAGE_TYPE_UNDEFIND           -1

#define FIT_TIMESTAMP_FIELD_NUM        ((unsigned char)253)

class FitReader
{
    public:
        FitReader(string filename);
        virtual ~FitReader();

        /**
         * Checks if the size and header of the file is correct
         * Throws FitFileException on CRC error or incorrect file size information
         * @return bool true if fit file
         */
        bool isFitFile();

        /**
         * Registers a FitMsg-Receiver funktion. Gets called for every known and sucessfully
         * decoded message block in the fit file
         */
        void registerFitMsgFkt(FitMsg_Listener * listener);

        /**
         * Reads the next record in the file. If a FitMsgFkt was registered this function
         * gets called when a data message was decoded and the message is known.
         * Calls fitMsgListener to consume the next fitMsg.
         * @throw FitFileException - on data format error
         * @return bool - true if more records exist
         */
        bool readNextRecord();

        /**
         * Reads the next fit msg in the file and returns it
         * Caller must delete FitMsg!
         * @throw FitFileException - on data format error
         * @return bool - true if more records exist
         */
        FitMsg * readNextFitMsg();


        /**
         * Closes the fit file.
         */
        void closeFitFile();

        /**
         * Enable/Disable debug output to FitMsg_Listener
         */
        void setDebugOutput(bool enable);

        /**
         * Reads the next fit message matching messageType
         */
        FitMsg * getNextFitMsgFromType(int messageType);

    private:

        /**
         * Reads the fit header and checks for correct version. Function expects 12 bytes
         * @return bool - returns true if correct header detected
         */
        bool readHeader();

        /**
         * Reads the complete file and calculates the CRC.
         * @return bool - returns true if CRC is correct
         */
        bool isCorrectCRC();

        /**
         * Used to output debug messages
         */
        void dbg(const string txt);

        /**
         * Used to output debug messages with a number
         */
        void dbg(const string txt, const int nbr);

        /**
         * Used to output debug messages with hex buffers
         */
        void dbgHex(const string txt, const char* data, const unsigned int length);

        /**
         * Describes one field in a message definition
         */
        typedef struct _FieldDef
        {
            unsigned char fieldDefNum;  // Field number in given profile
            unsigned char size;         // Size of field in bytes
            unsigned char baseType;     // Base Type (Bit7 = 1 --> signed)
        } FieldDef;

        /**
         * Stores a message definition. Up to 16 different definitions can be read
         * from the fit file.
         */
        typedef struct _MsgDef
        {
            int globalMsgNum;           // Which profile to use for this data
            unsigned char arch;         // Architecture used to store data (Bit0)
            int numFields;              // Number of fields in data message
            vector<FieldDef> fields;    // Field description
        } MsgDef;

        /**
         * Stores all 16 profiles that have been read from fit file.
         */
        MsgDef localMsgDef[MAX_LOCAL_MSG];

        /**
         * Reads a data package from file and returns it
         * FitMsg needs to be deleted by caller!
         */
        FitMsg * readDataPackage(MsgDef msg, unsigned int timestamp);

        /**
         * Stores the header length (first byte in file)
         */
        unsigned char headerLength;

        /**
         * Stores the data size of the data section (file size - header size - 2 (crc))
         */
        unsigned int dataSize;

        /**
         * Remaining bytes in data section
         */
        unsigned int remainingDataBytes;

        /**
         * Pointer to fit file on disk
         */
        ifstream file;

        /**
         * Enables output of debug messages to fitMsgListener
         */
        bool doFitDebug;

        /**
         * Pointer to function that receives the fit messages stored in the file
         */
        FitMsg_Listener * fitMsgListener;

        /**
         * Last time offset in compressed header fields
         */
        unsigned char lastTimeOffset;

        /**
         * Last timestamp, needed for compressed header fields
         */
        unsigned int timestamp;
};

#endif // FITREADER_H
