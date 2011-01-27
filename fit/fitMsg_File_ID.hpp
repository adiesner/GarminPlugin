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
#define FIT_MESSAGE_FILE_ID                                              ((unsigned char)0)

#define FIT_FILE_ID_TYPE_INVALID                                         ((unsigned char)0xFF)
#define FIT_FILE_ID_TYPE_DEVICE                                          ((unsigned char)1)
#define FIT_FILE_ID_TYPE_SETTINGS                                        ((unsigned char)2)
#define FIT_FILE_ID_TYPE_SPORT                                           ((unsigned char)3)
#define FIT_FILE_ID_TYPE_ACTIVITY                                        ((unsigned char)4)
#define FIT_FILE_ID_TYPE_WORKOUT                                         ((unsigned char)5)
#define FIT_FILE_ID_TYPE_WEIGHT                                          ((unsigned char)9)
#define FIT_FILE_ID_TYPE_TOTALS                                          ((unsigned char)10)
#define FIT_FILE_ID_TYPE_GOALS                                           ((unsigned char)11)
#define FIT_FILE_ID_TYPE_BLOOD_PRESSURE                                  ((unsigned char)14)

#define FIT_FILE_ID_MANUFACTURER_INVALID                                 ((unsigned short)0xFFFF)
#define FIT_FILE_ID_MANUFACTURER_GARMIN                                  ((unsigned short)1)
#define FIT_FILE_ID_MANUFACTURER_GARMIN_FR405_ANTFS                      ((unsigned short)2)
#define FIT_FILE_ID_MANUFACTURER_ZEPHYR                                  ((unsigned short)3)
#define FIT_FILE_ID_MANUFACTURER_DAYTON                                  ((unsigned short)4)
#define FIT_FILE_ID_MANUFACTURER_IDT                                     ((unsigned short)5)
#define FIT_FILE_ID_MANUFACTURER_SRM                                     ((unsigned short)6)
#define FIT_FILE_ID_MANUFACTURER_QUARQ                                   ((unsigned short)7)
#define FIT_FILE_ID_MANUFACTURER_IBIKE                                   ((unsigned short)8)
#define FIT_FILE_ID_MANUFACTURER_SARIS                                   ((unsigned short)9)
#define FIT_FILE_ID_MANUFACTURER_SPARK_HK                                ((unsigned short)10)
#define FIT_FILE_ID_MANUFACTURER_TANITA                                  ((unsigned short)11)
#define FIT_FILE_ID_MANUFACTURER_ECHOWELL                                ((unsigned short)12)
#define FIT_FILE_ID_MANUFACTURER_DYNASTREAM_OEM                          ((unsigned short)13)
#define FIT_FILE_ID_MANUFACTURER_NAUTILUS                                ((unsigned short)14)
#define FIT_FILE_ID_MANUFACTURER_DYNASTREAM                              ((unsigned short)15)
#define FIT_FILE_ID_MANUFACTURER_TIMEX                                   ((unsigned short)16)
#define FIT_FILE_ID_MANUFACTURER_METRIGEAR                               ((unsigned short)17)
#define FIT_FILE_ID_MANUFACTURER_XELIC                                   ((unsigned short)18)
#define FIT_FILE_ID_MANUFACTURER_BEURER                                  ((unsigned short)19)
#define FIT_FILE_ID_MANUFACTURER_CARDIOSPORT                             ((unsigned short)20)
#define FIT_FILE_ID_MANUFACTURER_A_AND_D                                 ((unsigned short)21)
#define FIT_FILE_ID_MANUFACTURER_HMM                                     ((unsigned short)22)

#define FIT_FILE_ID_GARMIN_PRODUCT_INVALID                               ((unsigned short)0xFFFF)
#define FIT_FILE_ID_GARMIN_PRODUCT_HRM1                                  ((unsigned short)1)
#define FIT_FILE_ID_GARMIN_PRODUCT_AXH01                                 ((unsigned short)2) // AXH01 HRM chipset
#define FIT_FILE_ID_GARMIN_PRODUCT_AXB01                                 ((unsigned short)3)
#define FIT_FILE_ID_GARMIN_PRODUCT_AXB02                                 ((unsigned short)4)
#define FIT_FILE_ID_GARMIN_PRODUCT_HRM2SS                                ((unsigned short)5)
#define FIT_FILE_ID_GARMIN_PRODUCT_FR405                                 ((unsigned short)717) // Forerunner 405
#define FIT_FILE_ID_GARMIN_PRODUCT_FR50                                  ((unsigned short)782) // Forerunner 50
#define FIT_FILE_ID_GARMIN_PRODUCT_FR60                                  ((unsigned short)988) // Forerunner 60
#define FIT_FILE_ID_GARMIN_PRODUCT_FR310XT                               ((unsigned short)1018) // Forerunner 310
#define FIT_FILE_ID_GARMIN_PRODUCT_EDGE500                               ((unsigned short)1036)
#define FIT_FILE_ID_GARMIN_PRODUCT_FR110                                 ((unsigned short)1124) // Forerunner 110
#define FIT_FILE_ID_GARMIN_PRODUCT_TRAINING_CENTER                       ((unsigned short)20119)
#define FIT_FILE_ID_GARMIN_PRODUCT_CONNECT                               ((unsigned short)65534) // Garmin Connect website

#define FIT_FILE_ID_SERIAL_NUMBER_INVALID                                ((unsigned long)0x00000000)

#define FIT_FILE_ID_TIME_CREATED_INVALID                                 ((unsigned long)0xFFFFFFFF)

#define FIT_FILE_ID_NUMBER_INVALID                                       ((unsigned short)0xFFFF)

#ifndef FITMSG_FILE_ID_H
#define FITMSG_FILE_ID_H

class FitMsg_File_ID : public FitMsg
{
public:
    FitMsg_File_ID() : FitMsg(FIT_MESSAGE_FILE_ID) {
        this->fileType     = FIT_FILE_ID_TYPE_INVALID;              //  8 bit
        this->manufacturer = FIT_FILE_ID_MANUFACTURER_INVALID;      // 16 bit
        this->product      = FIT_FILE_ID_GARMIN_PRODUCT_INVALID;    // 16 bit
        this->serialNumber = FIT_FILE_ID_SERIAL_NUMBER_INVALID;     // 32 bit
        this->timeCreated  = FIT_FILE_ID_TIME_CREATED_INVALID;      // 32 bit
        this->number       = FIT_FILE_ID_NUMBER_INVALID;            // 16 bit
    };

    virtual ~FitMsg_File_ID() {};

    /**
     * Gets the file type. See defines for valid values
     * @return unsigned char device type
     */
    unsigned char GetFileType()    { return this->fileType; };

    /**
     * Gets the manufacturer. See defines for valid values
     * @return unsigned short manufacturer
     */
    unsigned short GetManufacturer() { return this->manufacturer; };

    /**
     * Gets the Product. See defines for valid values
     * @return unsigned long product
     */
    unsigned short GetProduct()      { return this->product; };

    /**
     * Gets the serial number.
     * @return unsigned long serial number
     */
    unsigned long GetSerialNumber()  { return this->serialNumber; };

    /**
     * Gets the creation time.
     * @return unsigned long creation time
     */
    unsigned long GetTimeCreated()   { return this->timeCreated; };

    /**
     * Gets the number (????).
     * @return unsigned short number
     */
    unsigned short GetNumber()       { return this->number; };

    void SetFileType(unsigned char type)       { this->fileType = type; };
    void SetManufacturer(unsigned short man)   { this->manufacturer = man; };
    void SetProduct(unsigned short prod)       { this->product = prod; };
    void SetSerialNumber(unsigned long serial) { this->serialNumber = serial; };
    void SetTimeCreated(unsigned long time)    { this->timeCreated = time; };
    void SetNumber(unsigned short nbr)         { this->number = nbr; };

    bool addField(unsigned char fieldDefNum, unsigned char size, unsigned char baseType, unsigned char arch, char * data) {
        //TODO: Compare size with expected size
        //TODO: Compare baseType with expected baseType
        bool fieldWasAdded = true;
        switch (fieldDefNum) {
            case 0: SetFileType((unsigned char)data[0]);
                break;
            case 1: SetManufacturer(read0x84(data,arch));
                break;
            case 2: SetProduct(read0x84(data,arch));
                break;
            case 3: SetSerialNumber(read0x8C(data,arch));
                break;
            case 4: SetTimeCreated(read0x86(data,arch));
                break;
            case 5: SetNumber(read0x84(data,arch));
                break;
            default:
                fieldWasAdded = false;
        }
        return fieldWasAdded;
    };


private:
    unsigned char fileType;
    unsigned short manufacturer;
    unsigned short product;
    unsigned long serialNumber;
    unsigned long timeCreated;
    unsigned short number;
};

#endif // FITMSG_FILE_ID_H
