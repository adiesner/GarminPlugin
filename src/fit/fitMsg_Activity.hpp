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

#ifndef FITMSG_ACTIVITY_H
#define FITMSG_ACTIVITY_H

#define FIT_MESSAGE_ACTIVITY                                    ((unsigned char)34)

class FitMsg_Activity : public FitMsg
{
public:
	FitMsg_Activity() : FitMsg(FIT_MESSAGE_ACTIVITY) {
    };

    virtual ~FitMsg_Activity() {};

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
			case 0: setTotalTimerTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 1: setNumSessions(read0x84(data,arch));
					 break;
			case 2: setType(read0x00(data,arch));
					 break;
			case 3: setEvent(read0x00(data,arch));
					 break;
			case 4: setEventType(read0x00(data,arch));
					 break;
			case 5: setLocalTimestamp(read0x86(data,arch));
					 break;
			case 6: setEventGroup(read0x02(data,arch));
					 break;
            default:
                fieldWasAdded = false;
                break;
        }
        return fieldWasAdded;
    };



    enum FIT_EVENT {

    };


private:
	/* timestamp - Unit:  */
	unsigned long timestamp;

	/* total_timer_time - Unit: s */
	float totalTimerTime;

	/* num_sessions - Unit:  */
	unsigned short numSessions;

	/* type - Unit:  */
	unsigned char type;

	/* event - Unit:  */
	unsigned char event;

	/* event_type - Unit:  */
	unsigned char eventType;

	/* local_timestamp - Unit:  */
	unsigned long localTimestamp;

	/* event_group - Unit:  */
	unsigned char eventGroup;

public:
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

	unsigned long getLocalTimestamp() const {
		return localTimestamp;
	}

	void setLocalTimestamp(unsigned long localTimestamp) {
		this->localTimestamp = localTimestamp;
	}

	unsigned short getNumSessions() const {
		return numSessions;
	}

	void setNumSessions(unsigned short numSessions) {
		this->numSessions = numSessions;
	}

	unsigned long getTimestamp() const {
		return timestamp;
	}

	void setTimestamp(unsigned long timestamp) {
		this->timestamp = timestamp;
	}

	float getTotalTimerTime() const {
		return totalTimerTime;
	}

	void setTotalTimerTime(float totalTimerTime) {
		this->totalTimerTime = totalTimerTime;
	}

	unsigned char getType() const {
		return type;
	}

	void setType(unsigned char type) {
		this->type = type;
	}


};

#endif // FITMSG_ACTIVITY_H
