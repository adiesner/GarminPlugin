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


// The following profiles are not yet implemented!
#define FIT_MESSAGE_CAPABILITIES                                ((unsigned char)1)
#define FIT_MESSAGE_DEVICE_SETTINGS                             ((unsigned char)2)
#define FIT_MESSAGE_USER_PROFILE                                ((unsigned char)3)
#define FIT_MESSAGE_HRM_PROFILE                                 ((unsigned char)4)
#define FIT_MESSAGE_SDM_PROFILE                                 ((unsigned char)5)
#define FIT_MESSAGE_BIKE_PROFILE                                ((unsigned char)6)
#define FIT_MESSAGE_ZONES_TARGET                                ((unsigned char)7)
#define FIT_MESSAGE_HR_ZONE                                     ((unsigned char)8)
#define FIT_MESSAGE_POWER_ZONE                                  ((unsigned char)9)
#define FIT_MESSAGE_MET_ZONE                                    ((unsigned char)10)
#define FIT_MESSAGE_SPORT                                       ((unsigned char)12)
#define FIT_MESSAGE_TRAINING_GOALS                              ((unsigned char)15)
#define FIT_MESSAGE_WORKOUT                                     ((unsigned char)26)
#define FIT_MESSAGE_WORKOUT_STEP                                ((unsigned char)27)
#define FIT_MESSAGE_WEIGHT_SCALE                                ((unsigned char)30)
#define FIT_MESSAGE_TOTALS                                      ((unsigned char)33)
#define FIT_MESSAGE_SOFTWARE                                    ((unsigned char)35)
#define FIT_MESSAGE_FILE_CAPABILITIES                           ((unsigned char)37)
#define FIT_MESSAGE_MESG_CAPABILITIES                           ((unsigned char)38)
#define FIT_MESSAGE_FIELD_CAPABILITIES                          ((unsigned char)39)
#define FIT_MESSAGE_BLOOD_PRESSURE                              ((unsigned char)51)

#ifndef FITMSG_H
#define FITMSG_H

#include <string.h>

class FitMsg
{
public:

    FitMsg(unsigned char type)
    {
        this->classType = type;
    };

    virtual ~FitMsg() {};

    /**
     * Sets the timestamp of the message
     */
    virtual void SetTimestamp(unsigned int time) {};

    /**
     * Get type of message
     */
    unsigned char GetType()
    {
        return this->classType;
    };

    /**
     * Add a field to this message.
     * @return bool true if field was added successfully
     */
    virtual bool addField(unsigned char fieldDefNum, unsigned char size, unsigned char baseType, unsigned char arch, char * data) = 0;

protected:

    /**
     * Reads 2 bytes unsigned short from buffer with the type 0x84 (endianes aware)
     */
    static unsigned short read0x84(char * buf, unsigned char arch) {
        bool isLittleEndian = ((arch & 0x01) == 0) ? 1 : 0;
        unsigned short ret;
        if (isLittleEndian) {
            ret = ((unsigned char)buf[0]) + ((unsigned char)buf[1]<<8);
        } else {
            // God knows why, but the sdk never reads in big endian.
            ret = ((unsigned char)(buf[1]) + ((unsigned char)buf[0]<<8));
        }
        return ret;
    };

    /**
     * Reads 4 bytes unsigned long from buffer with the type 0x8C (endianes aware)
     */
    static unsigned long read0x8C(char * buf, unsigned char arch) {
        bool isLittleEndian = ((arch & 0x01) == 0) ? 1 : 0;
        unsigned long ret;
        if (isLittleEndian) {
            ret = ((0xFF & buf[3]) << 24) | ((0xFF & buf[2]) << 16) | ((0xFF & buf[1]) << 8) | (0xFF & buf[0]);
        } else {
            // God knows why, but the sdk never reads in big endian.
            ret = ((unsigned char) buf[0] << 24) | ((unsigned char) buf[1] << 16) | ((unsigned char) buf[2] << 8) | (unsigned char)buf[3];
        }
        ret = ret & 0xFFFFFFFF;
        return ret;
    };

    /**
     * Reads 4 bytes unsigned long from buffer with the type 0x86 (endianes aware)
     */
    static unsigned long read0x86(char * buf, unsigned char arch) {
        // only difference between 0x86 and 0x8C is the default value
        return read0x8C(buf, arch);
    };


    /**
     * Reads 4 bytes signed long from buffer with the type 0x85 (endianes aware)
     */
    static signed long  read0x85(char * buf, unsigned char arch) {
        bool isLittleEndian = ((arch & 0x01) == 0) ? 1 : 0;
        signed long ret;
        if (isLittleEndian) {
            ret = ((0xFF & buf[3]) << 24) | ((0xFF & buf[2]) << 16) | ((0xFF & buf[1]) << 8) | (0xFF & buf[0]);
        } else {
            // God knows why, but the sdk never reads in big endian.
            ret = ((0xFF & buf[0]) << 24) | ((0xFF & buf[1]) << 16) | ((0xFF & buf[2]) << 8) | (0xFF & buf[3]);
        }
        return ret;
    };

    /**
     * Reads 4 bytes float from buffer with the type 0x88 (endianes aware)
     */
    static float  read0x88(char * buf, unsigned char arch) {
    	return read0x88(buf,arch, 0x88,1,0);
    };


    /**
     * Reads 4 bytes float from buffer with the type 0x88 (endianes aware)
     * Supports scale and offset
     */
    static float  read0x88(char * buf, unsigned char arch, float scale, float offset, unsigned char baseType) {
    	float float32Value=0;
    	if (baseType == 0x84) {
    		unsigned short sShortValue = read0x84(buf, arch);
        	float32Value= sShortValue;
    	} else if (baseType == 0x86) {
			unsigned long sLongValue = read0x86(buf, arch);
			float32Value= sLongValue;
    	} else if (baseType == 0x85) {
    		signed long sLongValue = read0x85(buf, arch);
            memcpy(&float32Value, &sLongValue, sizeof(float32Value));
    	} else if (baseType == 0x00) {
    		unsigned char charVal= read0x00(buf,arch);
			float32Value= charVal;
    	}
    	float32Value = float32Value / scale - offset;
        return float32Value;
    };

    /**
     * Reads 1 bytes unsigned char from buffer with the type 0x02
     */
    static unsigned char read0x02(char * buf, unsigned char arch) {
    	unsigned char ret;
    	ret = (0xFF & buf[0]);
        return ret;
    };

    /**
     * Reads 1 bytes unsigned char from buffer with the type 0x00
     */
    static unsigned char read0x00(char * buf, unsigned char arch) {
    	unsigned char ret;
    	ret = (0xFF & buf[0]);
        return ret;
    };

    /**
     * Reads 1 bytes signed char from buffer with the type 0x00
     */
    static signed char read0x01(char * buf, unsigned char arch) {
    	signed char ret;
    	ret = (signed char)buf[0];
        return ret;
    };
    /**
     * Stores the class type
     */
    unsigned char classType;
};

#endif // FITMSG_H
