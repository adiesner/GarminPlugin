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

#ifndef FITMSG_DEVICEINFO_H
#define FITMSG_DEVICEINFO_H

#define FIT_MESSAGE_DEVICE_INFO                                 ((unsigned char)23)

class FitMsg_DeviceInfo: public FitMsg
{
public:
	FitMsg_DeviceInfo() : FitMsg(FIT_MESSAGE_DEVICE_INFO) {
    };

    virtual ~FitMsg_DeviceInfo() {};

    /**
     * Adds a field to the message. Unknown fields are rejected
     * @return bool if field was known to the message
     */
    bool addField(unsigned char fieldDefNum, unsigned char size, unsigned char baseType, unsigned char arch, char * data) {
        //TODO: Compare size with expected size
        //TODO: Compare baseType with expected baseType
        bool fieldWasAdded = true;
        switch (fieldDefNum) {
			case 253: setTimestamp(read0x86(data,arch));
					 break;
			case 0: setDeviceIndex(read0x02(data,arch));
					 break;
			case 1: setDeviceType(read0x02(data,arch));
					 break;
			case 2: setManufacturer(read0x84(data,arch));
					 break;
			case 3: setSerialNumber(read0x8C(data,arch));
					 break;
			case 4: setProduct(read0x84(data,arch));
					 break;
			case 5: setSoftwareVersion(read0x88(data,arch,100,0,0x84));
					 break;
			case 6: setHardwareVersion(read0x02(data,arch));
					 break;
			case 7: setCumOperatingTime(read0x86(data,arch));
					 break;
			case 10: setBatteryVoltage(read0x88(data,arch,256,0,0x84));
					 break;
			case 11: setBatteryStatus(read0x02(data,arch));
					 break;

            default:
                fieldWasAdded = false;
                break;
        }
        return fieldWasAdded;
    };


private:
	/* timestamp - Unit: s */
	unsigned long timestamp;

	/* device_index - Unit:  */
	unsigned char deviceIndex;

	/* device_type - Unit:  */
	unsigned char deviceType;

	/* manufacturer - Unit:  */
	unsigned short manufacturer;

	/* serial_number - Unit:  */
	unsigned long serialNumber;

	/* product - Unit:  */
	unsigned short product;

	/* software_version - Unit:  */
	float softwareVersion;

	/* hardware_version - Unit:  */
	unsigned char hardwareVersion;

	/* cum_operating_time - Unit: s */
	unsigned long cumOperatingTime;

	/* battery_voltage - Unit: V */
	float batteryVoltage;

	/* battery_status - Unit:  */
	unsigned char batteryStatus;

public:
	unsigned char getBatteryStatus() const {
		return batteryStatus;
	}

	void setBatteryStatus(unsigned char batteryStatus) {
		this->batteryStatus = batteryStatus;
	}

	float getBatteryVoltage() const {
		return batteryVoltage;
	}

	void setBatteryVoltage(float batteryVoltage) {
		this->batteryVoltage = batteryVoltage;
	}

	unsigned long getCumOperatingTime() const {
		return cumOperatingTime;
	}

	void setCumOperatingTime(unsigned long cumOperatingTime) {
		this->cumOperatingTime = cumOperatingTime;
	}

	unsigned char getDeviceIndex() const {
		return deviceIndex;
	}

	void setDeviceIndex(unsigned char deviceIndex) {
		this->deviceIndex = deviceIndex;
	}

	unsigned char getDeviceType() const {
		return deviceType;
	}

	void setDeviceType(unsigned char deviceType) {
		this->deviceType = deviceType;
	}

	unsigned char getHardwareVersion() const {
		return hardwareVersion;
	}

	void setHardwareVersion(unsigned char hardwareVersion) {
		this->hardwareVersion = hardwareVersion;
	}

	unsigned short getManufacturer() const {
		return manufacturer;
	}

	void setManufacturer(unsigned short manufacturer) {
		this->manufacturer = manufacturer;
	}

	unsigned short getProduct() const {
		return product;
	}

	void setProduct(unsigned short product) {
		this->product = product;
	}

	unsigned long getSerialNumber() const {
		return serialNumber;
	}

	void setSerialNumber(unsigned long serialNumber) {
		this->serialNumber = serialNumber;
	}

	float getSoftwareVersion() const {
		return softwareVersion;
	}

	void setSoftwareVersion(float softwareVersion) {
		this->softwareVersion = softwareVersion;
	}

	unsigned long getTimestamp() const {
		return timestamp;
	}

	void setTimestamp(unsigned long timestamp) {
		this->timestamp = timestamp;
	}




};

#endif // FITMSG_DEVICEINFO_H
