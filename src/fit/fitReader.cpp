/* -*- Mode: C; indent-tabs-mode: t; c-basic-offset: 4; tab-width: 4 -*- */
/*
 * GarminPlugin
 * Copyright (C) Andreas Diesner 2011 <garminplugin [AT] andreas.diesner [DOT] de>
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
#include "fitReader.hpp"
#include "fitFileException.hpp"

#include "fitMsg_File_ID.hpp"
#include "fitMsg_File_Creator.hpp"

#include <sstream>

FitReader::FitReader(string filename)
: headerLength(0)
, dataSize(0)
, remainingDataBytes(0)
, doFitDebug(false)
, fitMsgListener(0)
, lastTimeOffset(0)
, timestamp(0)
{
    // Initialize all message definition with undefined
    for (int i=0; i<MAX_LOCAL_MSG; i++) {
        localMsgDef[i].globalMsgNum = MESSAGE_TYPE_UNDEFIND;
    }

    // open file in binary mode
    this->file.open(filename.c_str(), ios::in | ios::binary);
}

FitReader::~FitReader()
{
}

bool FitReader::readHeader() {
    if ((this->file.good()) && (this->file.is_open())) {
        // Header is stored at the beginning
        this->file.seekg(std::ios::beg);
        char buf[12];
        this->file.read(buf,12);
        dbgHex("RAW Header Data: ", buf, 12);

        // Header length
        this->headerLength = (unsigned char) buf[0];
        dbg("Header Length: ", this->headerLength);

        // Protocol Version and Profile Number
        if ((((unsigned char)buf[1] & 0xF0) >> 4) > FIT_MAJOR_VERSION) {
            dbg("Major Version too high: ", (((unsigned char)buf[1] & 0xF0) >> 4));
            return false;
        } else {
            dbg("Major Version: ", (((unsigned char)buf[1] & 0xF0) >> 4));
        }

        // Minor Version is also not checked by reference implementation
        //if (((unsigned char)buf[0] & 0x0F) > FIT_MINOR_VERSION) {
        //    throw FitFileException("Unknown MINOR Version in FIT file. Unable to decode.");
        //}

        // Ignore FIT PROFILE Version
        //(((((FIT_PROFILE_MAJOR_VERSION * 100) + FIT_PROFILE_MINOR_VERSION) & 0x00FF) == buf[1]) &&
        // ((((FIT_PROFILE_MAJOR_VERSION * 100) + FIT_PROFILE_MINOR_VERSION) & 0xFF00) == buf[2]))

        // Read DataSize of Records
        this->dataSize = ((unsigned char)buf[4]) + ((unsigned char)buf[5] << 8) + ((unsigned char)buf[6] << 16) + ((unsigned char)buf[7] << 24);
        dbg("Data size: ", this->dataSize);

        // Check constant string .FIT
        if ((buf[8]=='.')&&(buf[9]=='F')&&(buf[10]=='I')&&(buf[11]=='T')) {
            this->file.seekg(this->headerLength); // Seek to start of data
            this->remainingDataBytes = dataSize;  // There is data to read
            return true;
        } else {
            dbg(".FIT Header not found in file!");
        }
    }
    return false;
}

bool FitReader::isFitFile() {
    if ((this->file.good()) && (this->file.is_open())) {
        // check length of file
        this->file.seekg (0, ios::end);
        unsigned int length = this->file.tellg();
        // File length must be at least 14 bytes (12 bytes header, 2 bytes crc)
        if (length < 14) {
            dbg("Not a FIT file: File length is smaller than 14 bytes");
            return false;
        }

        if (readHeader()) {
            if (length != (this->dataSize + this->headerLength + 2)) {
                dbg("File size in header does not match actual file size");
                throw FitFileException("FIT Decode Error. Filesize does not match header information!");
            }
            if (!isCorrectCRC()) {
                dbg("CRC is incorrect");
                throw FitFileException("FIT Decode Error. CRC incorrect!");
            }
            return true;
        }
    }
    dbg("Fit file is not open or has i/o errors");
    return false;
}

void FitReader::registerFitMsgFkt(FitMsg_Listener * listener) {
    this->fitMsgListener = listener;
}

void FitReader::setDebugOutput(bool enable) {
    this->doFitDebug = enable;
}

void FitReader::dbg(const string txt) {
    if ((doFitDebug) && (this->fitMsgListener != NULL)) {
        // output debug message to debug sink
        this->fitMsgListener->fitDebugMsg("FitReader: " + txt);
    }
}

void FitReader::dbg(const string txt, const int nbr) {
    if ((doFitDebug) && (this->fitMsgListener != NULL)) {
        stringstream ss;
        ss << txt << nbr;
        dbg(ss.str());
    }
}

void FitReader::dbgHex(const string txt, const char* data, const unsigned int length) {
    if ((doFitDebug) && (this->fitMsgListener != NULL)) {
        // Build hex string for debug message
        stringstream ss;
        ss << txt;
        unsigned int i = 0;
        while (i < length) {
            if ((unsigned char)data[i] < 16) {
                ss << "0";
                ss << hex << (unsigned int)(data[i++] & 0xFF);
            } else {
                ss << hex << (unsigned int)(data[i++] & 0xFF);
            }
            ss << " ";
        }
        dbg(ss.str());
    }
}

bool FitReader::isCorrectCRC() {
    // Only calculate header on an open file without errors
    if ((this->file.is_open()) && (this->file.good())) {
        // CRC is calculated over the complete file
        this->file.seekg(std::ios::beg);
        char buf[1024];

        const unsigned short crc_table[16] = {
            0x0000, 0xCC01, 0xD801, 0x1400, 0xF001, 0x3C00, 0x2800, 0xE401,
            0xA001, 0x6C00, 0x7800, 0xB401, 0x5000, 0x9C01, 0x8801, 0x4400 };
        unsigned short crc = 0;

        while (!this->file.eof()) {
            this->file.read(buf, 1024);
            int bytesRead = this->file.gcount();

            for (int i=0; i<bytesRead; i++) {
               unsigned short tmp;
               // compute checksum of lower four bits of byte
               tmp = crc_table[crc & 0x0F];
               crc  = (crc >> 4) & 0x0FFF;
               crc  = crc ^ tmp ^ crc_table[ (unsigned char)buf[i] & 0x0F];

               // now compute checksum of upper four bits of byte
               tmp = crc_table[crc & 0x0F];
               crc  = (crc >> 4) & 0x0FFF;
               crc  = crc ^ tmp ^ crc_table[( (unsigned char)buf[i] >> 4) & 0x0F];
            }
        }
        this->file.clear(); // Clear eof bit
        this->file.seekg(this->headerLength); // Seek to start of data

        if (crc != 0) {
            dbg("CRC is incorrect: ", crc);
            return false;
        }
        dbg("CRC is correct: ", crc);
        return true;
    }
    dbg("Fit file is not open or has i/o errors");
    return false;
}

bool FitReader::readNextRecord() {
    if ((!this->file.is_open()) || (this->file.bad()) || (this->remainingDataBytes <= 0)) {
        if (this->remainingDataBytes == 0) { // just for better debug message text
            dbg("End of fit file");
        } else {
            dbg("File i/o error");
        }
        return false;
    }

    FitMsg *fitMsg = readNextFitMsg();
    if (fitMsg != NULL) {
        // Notify message listener
    	if (this->fitMsgListener != NULL) {
    		this->fitMsgListener->fitMsgReceived(fitMsg);
    	}
    	delete(fitMsg);
    }
    return true;
}

FitMsg * FitReader::readNextFitMsg() {

    if ((!this->file.is_open()) || (this->file.bad()) || (this->remainingDataBytes <= 0)) {
        if (this->remainingDataBytes == 0) { // just for better debug message text
            dbg("End of fit file");
        } else {
            dbg("File i/o error");
        }
        return NULL;
    }

    char buf[6];
    this->file.read(buf,1); // read record header byte
    this->remainingDataBytes--;
    dbgHex("RAW Record Header: ", buf, 1);

    // Compressed header or normal header?
    bool isNormalHeader       = ((buf[0] & 0x80) == 0) ? true : false;

    if (!isNormalHeader) {
        dbg("Compressed header");
        unsigned int localMsgNr   = (buf[0] & 0x60) >> 5;   // compressed header can only use profile 0-3
        unsigned char timeOffset  = (buf[0] & 0x1F);        // Time offset in seconds from last timestamp
        if (this->localMsgDef[localMsgNr].globalMsgNum == MESSAGE_TYPE_UNDEFIND) {
            dbg("FIT Decode Error, undefined local message: ", localMsgNr);
            throw FitFileException("FIT Decode Error. Local Message not yet defined!");
        }

        //TODO: calculate correct timestamp!
        //TODO: timestamp is never set, but should be!
        this->timestamp += (timeOffset - this->lastTimeOffset) & 0x1F;
        this->lastTimeOffset = timeOffset;
        FitMsg *fitMsg = readDataPackage(this->localMsgDef[localMsgNr], this->timestamp);
        return fitMsg;
    } else { // if (!isNormalHeader)
        dbg("Is normal header");
        // isNormalHeader
        bool isDefinitionMsg      = ((buf[0] & 0x40) == 0) ? 0 : 1;     // Definition or data message?
        unsigned int localMsgNr   =  (buf[0] & 0x0F);                   // Profile number

        if (isDefinitionMsg) {
            this->remainingDataBytes-=5;
            this->file.read(buf,5);           // Read 5 bytes of definition field
            dbgHex("Definition Msg Header: ", buf, 5);
            //char reserved = buf[0];           // First byte is reserved
            bool isLittleEndian = ((buf[1] & 0x01) == 0) ? 1 : 0;   // Little or big endian
            unsigned int globalMsgNum = 0;
            if (isLittleEndian) {
                globalMsgNum = ((buf[2]) + ((unsigned int)buf[3]<<8));
            } else {
                globalMsgNum = (((unsigned int)buf[2]<<8) + (buf[3]));
            }

            this->localMsgDef[localMsgNr].globalMsgNum = globalMsgNum;  // Profile number
            this->localMsgDef[localMsgNr].arch = buf[1];                // Architecture (little/big endian)
            this->localMsgDef[localMsgNr].numFields = buf[4];           // Number of fields following

            dbg("Definition Msg-Type: Number=", localMsgNr);
            dbg("Definition Msg-Type: Arch=", buf[1]);
            dbg("Definition Msg-Type: NumFields=", this->localMsgDef[localMsgNr].numFields);
            dbg("Definition Msg-Type: GlobalMsgNum=", this->localMsgDef[localMsgNr].globalMsgNum);
            this->localMsgDef[localMsgNr].fields.clear();   // Messages can be redefined! Clear fields

            // Read all following fields of this profile
            int curFieldNum = 0;
            while (curFieldNum < this->localMsgDef[localMsgNr].numFields) {
                this->remainingDataBytes-=3;
                this->file.read(buf,3);         // read 3 bytes definition of a field
                FieldDef field;
                field.fieldDefNum = (unsigned char)buf[0];
                field.size        = (unsigned char)buf[1];
                field.baseType    = (unsigned char)buf[2];

                dbg("Field Def: FieldDefNum=", (unsigned char)buf[0]);
                dbg("Field Def: Size=",        (unsigned char)buf[1]);
                dbg("Field Def: BaseType=",    (unsigned char)buf[2]);

                this->localMsgDef[localMsgNr].fields.push_back(field);
                curFieldNum++;
            }
        } // if (isDefinitionMsg)
        else {
            // if (!isDefinitionMsg)
            if (this->localMsgDef[localMsgNr].globalMsgNum == MESSAGE_TYPE_UNDEFIND) {
                dbg("FIT Decode Error, undefined local message: ", localMsgNr);
                throw FitFileException("FIT Decode Error. Local Message not yet defined!");
            }

            FitMsg *fitMsg = readDataPackage(localMsgDef[localMsgNr], 0);
            return fitMsg;
        }
    }
    return NULL;
}

FitMsg * FitReader::readDataPackage(MsgDef msg, unsigned int timestamp) {

    FitMsg * fitMsg = NULL;
    switch (msg.globalMsgNum)
    {
        case FIT_MESSAGE_FILE_ID :
                fitMsg = new FitMsg_File_ID();
                break;
        case FIT_MESSAGE_FILE_CREATOR :
                fitMsg = new FitMsg_File_Creator();
                break;
        default:
            dbg("Profile not yet implemented: ", msg.globalMsgNum);
    }

    // If timestamp from compressed header was given and message is known
    if ((timestamp != 0) && (fitMsg != NULL)) {
        dbg("Setting timestamp from compressed header: ", timestamp);
        fitMsg->SetTimestamp(timestamp);
    }

    char buf[256]; // 256 is maximum message length
    std::vector<FieldDef>::iterator fieldIter = msg.fields.begin();
    while( fieldIter != msg.fields.end() ) {
        FieldDef f = (*fieldIter);
        this->remainingDataBytes-=f.size;
        this->file.read(buf,f.size);        // read data from file
        if (fitMsg != NULL) {
            // Add field to profile. Returns false if unknown field for this profile
            if (!fitMsg->addField(f.fieldDefNum, f.baseType, f.size, msg.arch, buf)) {
                dbg("Field is unknown for this profile: ", f.fieldDefNum);
                dbg("Reading FieldDefNum: ", f.fieldDefNum);
                dbg("Reading BaseType: ", f.baseType);
                dbgHex("Raw Read: ", buf, f.size);
            }
        }
        // If this is a timestamp field, use it for compressed header
        if (f.fieldDefNum == FIT_TIMESTAMP_FIELD_NUM) {
            if ((msg.arch & 0x01) == 0) {
                this->timestamp = ((unsigned char) buf[3] << 24) | ((unsigned char) buf[2] << 16) | ((unsigned char) buf[1] << 8) | (unsigned char)buf[0];
            } else {
                this->timestamp = ((unsigned char) buf[0] << 24) | ((unsigned char) buf[1] << 16) | ((unsigned char) buf[2] << 8) | (unsigned char)buf[3];
            }
            this->lastTimeOffset = (unsigned char)(this->timestamp & 0x1F);
        }
        ++fieldIter;
    }

    return fitMsg;
}

void FitReader::closeFitFile() {
    // Close file
    if (this->file.is_open()) {
        this->file.close();
    }
}

FitMsg * FitReader::getNextFitMsgFromType(int messageType) {

	if ((!this->file.good()) || (!this->file.is_open())) {
		dbg("File not open");
		return NULL;
	}

	while (this->remainingDataBytes > 0) {
		FitMsg * msg = readNextFitMsg();
		if (msg != NULL) {
			if (msg->GetType() == messageType) {
				return msg;
			}
			delete(msg);
		}
	}

	return NULL;

}
