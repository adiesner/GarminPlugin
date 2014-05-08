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

#ifndef FITMSG_FILE_CREATOR_H
#define FITMSG_FILE_CREATOR_H

#define FIT_MESSAGE_FILE_CREATOR                                         ((unsigned char)49)

#define FIT_FILE_ID_SOFTWARE_VERSION_INVALID                             ((unsigned short)0xFFFF)

#define FIT_FILE_ID_HARDWARE_VERSION_INVALID                             ((unsigned char)0xFF)

class FitMsg_File_Creator : public FitMsg
{
public:
    FitMsg_File_Creator() : FitMsg(FIT_MESSAGE_FILE_CREATOR) {
        this->software_version   = FIT_FILE_ID_SOFTWARE_VERSION_INVALID;      // 16 bit
        this->hardware_version   = FIT_FILE_ID_HARDWARE_VERSION_INVALID;      //  8 bit
    };

    virtual ~FitMsg_File_Creator() {};

    /**
     * Returns the software version
     * @return unsigned short Software Version
     */
    unsigned short getSoftwareVersion()  { return this->software_version; };

    /**
     * Returns the hardware version
     * @return unsigned char Hardware Version
     */
    unsigned char getHardwareVersion()   { return this->hardware_version; };

    /**
     * Sets the software version
     */
    void setSoftwareVersion(unsigned short ver)     { this->software_version = ver; };

    /**
     * Sets the hardware version
     */
    void setHardwareVersion(unsigned char ver)    { this->hardware_version = ver; };

    /**
     * Adds a field to the message. Unknown fields are rejected
     * @return bool if field was known to the message
     */
    bool addField(unsigned char fieldDefNum, unsigned char size, unsigned char baseType, unsigned char arch, char * data) {
        //TODO: Compare size with expected size
        //TODO: Compare baseType with expected baseType
        bool fieldWasAdded = true;
        switch (fieldDefNum) {
            case 0: setSoftwareVersion(read0x84(data,arch));
                break;
            case 1: setHardwareVersion((unsigned char)data[0]);
                break;
            default:
                fieldWasAdded = false;
                break;
        }
        return fieldWasAdded;
    };


private:
    /**
     * Stores the software version
     */
    unsigned short software_version;

    /**
     * Stores the hardware version
     */
    unsigned char hardware_version;
};

#endif // FITMSG_FILE_CREATOR_H
