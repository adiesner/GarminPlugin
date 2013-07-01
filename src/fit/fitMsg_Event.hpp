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


#ifndef FITMSG_EVENT_H
#define FITMSG_EVENT_H

#define FIT_MESSAGE_EVENT                                       ((unsigned char)21)

class FitMsg_Event : public FitMsg
{
public:
	FitMsg_Event() : FitMsg(FIT_MESSAGE_EVENT) {
    };

    virtual ~FitMsg_Event() {};

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
			case 0: setEvent(read0x00(data,arch));
					 break;
			case 1: setEventType(read0x00(data,arch));
					 break;
			case 2: setData16(read0x84(data,arch));
					 break;
			case 3: fieldWasAdded = consumeEvent(size, baseType, arch, data);
					 break;
			case 4: setEventGroup(read0x02(data,arch));
					 break;

            default:
                fieldWasAdded = false;
                break;
        }
        return fieldWasAdded;
    };


private:
	unsigned long timestamp;
	unsigned char event;
	unsigned char eventType;
	unsigned short data16;
	unsigned long data;
	unsigned char timerTrigger;
	unsigned short coursePointIndex;
	float batteryLevel;
	float virtualPartnerSpeed;
	unsigned char hrHighAlert;
	unsigned char hrLowAlert;
	float speedHighAlert;
	float speedLowAlert;
	unsigned short cadHighAlert;
	unsigned short cadLowAlert;
	unsigned short powerHighAlert;
	unsigned short powerLowAlert;
	float timeDurationAlert;
	float distanceDurationAlert;
	unsigned long calorieDurationAlert;
	unsigned char fitnessEquipmentState;
	unsigned char eventGroup;

	bool consumeEvent(unsigned char size, unsigned char baseType, unsigned char arch, char * data) {
		/*
			case 3: setData(read0x86(data,arch));
					 break;
			case 3: setTimerTrigger(read0x00(data,arch));
					 break;
			case 3: setCoursePointIndex(read0x84(data,arch));
					 break;
			case 3: setBatteryLevel(read0x88(data,arch));
					 break;
			case 3: setVirtualPartnerSpeed(read0x88(data,arch));
					 break;
			case 3: setHrHighAlert(read0x02(data,arch));
					 break;
			case 3: setHrLowAlert(read0x02(data,arch));
					 break;
			case 3: setSpeedHighAlert(read0x88(data,arch));
					 break;
			case 3: setSpeedLowAlert(read0x88(data,arch));
					 break;
			case 3: setCadHighAlert(read0x84(data,arch));
					 break;
			case 3: setCadLowAlert(read0x84(data,arch));
					 break;
			case 3: setPowerHighAlert(read0x84(data,arch));
					 break;
			case 3: setPowerLowAlert(read0x84(data,arch));
					 break;
			case 3: setTimeDurationAlert(read0x88(data,arch));
					 break;
			case 3: setDistanceDurationAlert(read0x88(data,arch));
					 break;
			case 3: setCalorieDurationAlert(read0x86(data,arch));
					 break;
			case 3: setFitnessEquipmentState(read0x00(data,arch));
					 break;
		*/
		return false;
	}

public:

	float getBatteryLevel() const {
		return batteryLevel;
	}

	void setBatteryLevel(float batteryLevel) {
		this->batteryLevel = batteryLevel;
	}

	unsigned short getCadHighAlert() const {
		return cadHighAlert;
	}

	void setCadHighAlert(unsigned short cadHighAlert) {
		this->cadHighAlert = cadHighAlert;
	}

	unsigned short getCadLowAlert() const {
		return cadLowAlert;
	}

	void setCadLowAlert(unsigned short cadLowAlert) {
		this->cadLowAlert = cadLowAlert;
	}

	unsigned long getCalorieDurationAlert() const {
		return calorieDurationAlert;
	}

	void setCalorieDurationAlert(unsigned long calorieDurationAlert) {
		this->calorieDurationAlert = calorieDurationAlert;
	}

	unsigned short getCoursePointIndex() const {
		return coursePointIndex;
	}

	void setCoursePointIndex(unsigned short coursePointIndex) {
		this->coursePointIndex = coursePointIndex;
	}

	unsigned long getData() const {
		return data;
	}

	void setData(unsigned long data) {
		this->data = data;
	}

	unsigned short getData16() const {
		return data16;
	}

	void setData16(unsigned short data16) {
		this->data16 = data16;
	}

	float getDistanceDurationAlert() const {
		return distanceDurationAlert;
	}

	void setDistanceDurationAlert(float distanceDurationAlert) {
		this->distanceDurationAlert = distanceDurationAlert;
	}

	unsigned char getEvent() const {
		return event;
	}

	void setEvent(unsigned char event) {
		this->event = event;
	}

	unsigned char getEventGroup() const {
		return eventGroup;
	}

	void setEventGroup(unsigned char eventGroup) {
		this->eventGroup = eventGroup;
	}

	unsigned char getEventType() const {
		return eventType;
	}

	void setEventType(unsigned char eventType) {
		this->eventType = eventType;
	}

	unsigned char getFitnessEquipmentState() const {
		return fitnessEquipmentState;
	}

	void setFitnessEquipmentState(unsigned char fitnessEquipmentState) {
		this->fitnessEquipmentState = fitnessEquipmentState;
	}

	unsigned char getHrHighAlert() const {
		return hrHighAlert;
	}

	void setHrHighAlert(unsigned char hrHighAlert) {
		this->hrHighAlert = hrHighAlert;
	}

	unsigned char getHrLowAlert() const {
		return hrLowAlert;
	}

	void setHrLowAlert(unsigned char hrLowAlert) {
		this->hrLowAlert = hrLowAlert;
	}

	unsigned short getPowerHighAlert() const {
		return powerHighAlert;
	}

	void setPowerHighAlert(unsigned short powerHighAlert) {
		this->powerHighAlert = powerHighAlert;
	}

	unsigned short getPowerLowAlert() const {
		return powerLowAlert;
	}

	void setPowerLowAlert(unsigned short powerLowAlert) {
		this->powerLowAlert = powerLowAlert;
	}

	float getSpeedHighAlert() const {
		return speedHighAlert;
	}

	void setSpeedHighAlert(float speedHighAlert) {
		this->speedHighAlert = speedHighAlert;
	}

	float getSpeedLowAlert() const {
		return speedLowAlert;
	}

	void setSpeedLowAlert(float speedLowAlert) {
		this->speedLowAlert = speedLowAlert;
	}

	float getTimeDurationAlert() const {
		return timeDurationAlert;
	}

	void setTimeDurationAlert(float timeDurationAlert) {
		this->timeDurationAlert = timeDurationAlert;
	}

	unsigned char getTimerTrigger() const {
		return timerTrigger;
	}

	void setTimerTrigger(unsigned char timerTrigger) {
		this->timerTrigger = timerTrigger;
	}

	unsigned long getTimestamp() const {
		return timestamp;
	}

	void setTimestamp(unsigned long timestamp) {
		this->timestamp = timestamp;
	}

	float getVirtualPartnerSpeed() const {
		return virtualPartnerSpeed;
	}

	void setVirtualPartnerSpeed(float virtualPartnerSpeed) {
		this->virtualPartnerSpeed = virtualPartnerSpeed;
	}


};

#endif // FITMSG_EVENT_H
