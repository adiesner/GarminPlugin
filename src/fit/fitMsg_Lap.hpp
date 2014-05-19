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


#ifndef FITMSG_LAP_H
#define FITMSG_LAP_H

#define FIT_MESSAGE_LAP                                         ((unsigned char)19)

class FitMsg_Lap : public FitMsg
{
public:
	FitMsg_Lap() : FitMsg(FIT_MESSAGE_LAP),
	    //TODO: Check if other default values are better
                messageIndex(0),
                timestamp(0),
                event(0),
                eventType(0),
                startTime(0),
                startPositionLat(0),
                startPositionLong(0),
                endPositionLat(0),
                endPositionLong(0),
                totalElapsedTime(0),
                totalTimerTime(0),
                totalDistance(0),
                totalCycles(0),
                totalStrides(0),
                totalCalories(0),
                totalFatCalories(0),
                avgSpeed(0),
                maxSpeed(0),
                avgHeartRate(0),
                maxHeartRate(0),
                avgCadence(0),
                maxCadence(0),
                avgPower(0),
                maxPower(0),
                totalAscent(0),
                totalDescent(0),
                intensity(0),
                lapTrigger(0),
                sport(0),
                eventGroup(0),
                numLengths(0),
                normalizedPower(0),
                leftRightBalance(0),
                firstLengthIndex(0),
                avgStrokeDistance(0),
                swimStroke(0),
                subSport(0),
                numActiveLengths(0),
                totalWork(0),
                avgAltitude(0),
                maxAltitude(0),
                gpsAccuracy(0),
                avgGrade(0),
                avgPosGrade(0),
                avgNegGrade(0),
                maxPosGrade(0),
                maxNegGrade(0),
                avgTemperature(0),
                maxTemperature(0),
                totalMovingTime(0),
                avgPosVerticalSpeed(0),
                avgNegVerticalSpeed(0),
                maxPosVerticalSpeed(0),
                maxNegVerticalSpeed(0),
                numTimeInHrZone(0),
                numTimeInSpeedZone(0),
                numTimeInCadenceZone(0),
                numTimeInPowerZone(0),
                repetitionNum(0),
                minAltitude(0),
                minHeartRate(0),
                wktStepIndex(0)
	{
    };

    virtual ~FitMsg_Lap() {};

    /**
     * Adds a field to the message. Unknown fields are rejected
     * @return bool if field was known to the message
     */
    bool addField(unsigned char fieldDefNum, unsigned char size, unsigned char baseType, unsigned char arch, char * data) {
        //TODO: Compare size with expected size
        //TODO: Compare baseType with expected baseType
        bool fieldWasAdded = true;
        switch (fieldDefNum) {
			case 254: setMessageIndex(read0x84(data,arch));
					 break;
			case 253: setTimestamp(read0x86(data,arch));
					 break;
			case 0: setEvent(read0x00(data,arch));
					 break;
			case 1: setEventType(read0x00(data,arch));
					 break;
			case 2: setStartTime(read0x86(data,arch));
					 break;
			case 3: setStartPositionLat(read0x85(data,arch));
					 break;
			case 4: setStartPositionLong(read0x85(data,arch));
					 break;
			case 5: setEndPositionLat(read0x85(data,arch));
					 break;
			case 6: setEndPositionLong(read0x85(data,arch));
					 break;
			case 7: setTotalElapsedTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 8: setTotalTimerTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 9: setTotalDistance(read0x88(data,arch,100,0,0x86));
					 break;
			case 10: setTotalCycles(read0x86(data,arch));
					 break;
			case 11: setTotalCalories(read0x84(data,arch));
					 break;
			case 12: setTotalFatCalories(read0x84(data,arch));
					 break;
			case 13: setAvgSpeed(read0x88(data,arch,1000,0,0x84));
					 break;
			case 14: setMaxSpeed(read0x88(data,arch,1000,0,0x84));
					 break;
			case 15: setAvgHeartRate(read0x02(data,arch));
					 break;
			case 16: setMaxHeartRate(read0x02(data,arch));
					 break;
			case 17: setAvgCadence(read0x02(data,arch));
					 break;
			case 18: setMaxCadence(read0x02(data,arch));
					 break;
			case 19: setAvgPower(read0x84(data,arch));
					 break;
			case 20: setMaxPower(read0x84(data,arch));
					 break;
			case 21: setTotalAscent(read0x84(data,arch));
					 break;
			case 22: setTotalDescent(read0x84(data,arch));
					 break;
			case 23: setIntensity(read0x00(data,arch));
					 break;
			case 24: setLapTrigger(read0x00(data,arch));
					 break;
			case 25: setSport(read0x00(data,arch));
					 break;
			case 26: setEventGroup(read0x02(data,arch));
					 break;
			case 32: setNumLengths(read0x84(data,arch));
					 break;
			case 33: setNormalizedPower(read0x84(data,arch));
					 break;
			case 34: setLeftRightBalance(read0x84(data,arch));
					 break;
			case 35: setFirstLengthIndex(read0x84(data,arch));
					 break;
			case 37: setAvgStrokeDistance(read0x88(data,arch,100,0,0x84));
					 break;
			case 38: setSwimStroke(read0x00(data,arch));
					 break;
			case 39: setSubSport(read0x00(data,arch));
					 break;
			case 40: setNumActiveLengths(read0x84(data,arch));
					 break;
			case 41: setTotalWork(read0x86(data,arch));
					 break;
			case 42: setAvgAltitude(read0x88(data,arch,5,500,0x84));
					 break;
			case 43: setMaxAltitude(read0x88(data,arch,5,500,0x84));
					 break;
			case 44: setGpsAccuracy(read0x02(data,arch));
					 break;
			case 45: setAvgGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 46: setAvgPosGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 47: setAvgNegGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 48: setMaxPosGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 49: setMaxNegGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 50: setAvgTemperature(read0x01(data,arch));
					 break;
			case 51: setMaxTemperature(read0x01(data,arch));
					 break;
			case 52: setTotalMovingTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 53: setAvgPosVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 54: setAvgNegVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 55: setMaxPosVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 56: setMaxNegVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 61: setRepetitionNum(read0x84(data,arch));
					 break;
			case 62: setMinAltitude(read0x88(data,arch,5,500,0x84));
					 break;
			case 63: setMinHeartRate(read0x02(data,arch));
					 break;
			case 71: setWktStepIndex(read0x84(data,arch));
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
	/* message_index - Unit:  */
	unsigned short messageIndex;

	/* timestamp - Unit: s */
	unsigned long timestamp;

	/* event - Unit:  */
	unsigned char event;

	/* event_type - Unit:  */
	unsigned char eventType;

	/* start_time - Unit:  */
	unsigned long startTime;

	/* start_position_lat - Unit: semicircles */
	signed long startPositionLat;

	/* start_position_long - Unit: semicircles */
	signed long startPositionLong;

	/* end_position_lat - Unit: semicircles */
	signed long endPositionLat;

	/* end_position_long - Unit: semicircles */
	signed long endPositionLong;

	/* total_elapsed_time - Unit: s */
	float totalElapsedTime;

	/* total_timer_time - Unit: s */
	float totalTimerTime;

	/* total_distance - Unit: m */
	float totalDistance;

	/* total_cycles - Unit: cycles */
	unsigned long totalCycles;

	/* total_cycles - Unit: cycles */
	unsigned long totalStrides;

	/* total_calories - Unit: kcal */
	unsigned short totalCalories;

	/* total_fat_calories - Unit: kcal */
	unsigned short totalFatCalories;

	/* avg_speed - Unit: m/s */
	float avgSpeed;

	/* max_speed - Unit: m/s */
	float maxSpeed;

	/* avg_heart_rate - Unit: bpm */
	unsigned char avgHeartRate;

	/* max_heart_rate - Unit: bpm */
	unsigned char maxHeartRate;

	/* avg_cadence - Unit: rpm */
	unsigned char avgCadence;

	/* avg_cadence - Unit: rpm */
	//unsigned char avgRunningCadence;  == avgCadence

	/* max_cadence - Unit: rpm */
	unsigned char maxCadence;

	/* max_cadence - Unit: rpm */
	//unsigned char maxRunningCadence;  == maxCadence

	/* avg_power - Unit: watts */
	unsigned short avgPower;

	/* max_power - Unit: watts */
	unsigned short maxPower;

	/* total_ascent - Unit: m */
	unsigned short totalAscent;

	/* total_descent - Unit: m */
	unsigned short totalDescent;

	/* intensity - Unit:  */
	unsigned char intensity;

	/* lap_trigger - Unit:  */
	unsigned char lapTrigger;

	/* sport - Unit:  */
	unsigned char sport;

	/* event_group - Unit:  */
	unsigned char eventGroup;

	/* num_lengths - Unit: lengths */
	unsigned short numLengths;

	/* normalized_power - Unit: watts */
	unsigned short normalizedPower;

	/* left_right_balance - Unit:  */
	unsigned short leftRightBalance;

	/* first_length_index - Unit:  */
	unsigned short firstLengthIndex;

	/* avg_stroke_distance - Unit: m */
	float avgStrokeDistance;

	/* swim_stroke - Unit:  */
	unsigned char swimStroke;

	/* sub_sport - Unit:  */
	unsigned char subSport;

	/* num_active_lengths - Unit: lengths */
	unsigned short numActiveLengths;

	/* total_work - Unit: J */
	unsigned long totalWork;

	/* avg_altitude - Unit: m */
	float avgAltitude;

	/* max_altitude - Unit: m */
	float maxAltitude;

	/* gps_accuracy - Unit: m */
	unsigned char gpsAccuracy;

	/* avg_grade - Unit: % */
	float avgGrade;

	/* avg_pos_grade - Unit: % */
	float avgPosGrade;

	/* avg_neg_grade - Unit: % */
	float avgNegGrade;

	/* max_pos_grade - Unit: % */
	float maxPosGrade;

	/* max_neg_grade - Unit: % */
	float maxNegGrade;

	/* avg_temperature - Unit: C */
	signed char avgTemperature;

	/* max_temperature - Unit: C */
	signed char maxTemperature;

	/* total_moving_time - Unit: s */
	float totalMovingTime;

	/* avg_pos_vertical_speed - Unit: m/s */
	float avgPosVerticalSpeed;

	/* avg_neg_vertical_speed - Unit: m/s */
	float avgNegVerticalSpeed;

	/* max_pos_vertical_speed - Unit: m/s */
	float maxPosVerticalSpeed;

	/* max_neg_vertical_speed - Unit: m/s */
	float maxNegVerticalSpeed;

	/* time_in_hr_zone - Unit: s */
	unsigned char numTimeInHrZone;

	/* time_in_speed_zone - Unit: s */
	unsigned char numTimeInSpeedZone;

	/* time_in_cadence_zone - Unit: s */
	unsigned char numTimeInCadenceZone;

	/* time_in_power_zone - Unit: s */
	unsigned char numTimeInPowerZone;

	/* repetition_num - Unit:  */
	unsigned short repetitionNum;

	/* min_altitude - Unit: m */
	float minAltitude;

	/* min_heart_rate - Unit: bpm */
	unsigned char minHeartRate;

	/* wkt_step_index - Unit:  */
	unsigned short wktStepIndex;

public:
	float getAvgAltitude() const {
		return avgAltitude;
	}

	void setAvgAltitude(float avgAltitude) {
		this->avgAltitude = avgAltitude;
	}

	unsigned char getAvgCadence() const {
		return avgCadence;
	}

	void setAvgCadence(unsigned char avgCadence) {
		this->avgCadence = avgCadence;
	}

	float getAvgGrade() const {
		return avgGrade;
	}

	void setAvgGrade(float avgGrade) {
		this->avgGrade = avgGrade;
	}

	unsigned char getAvgHeartRate() const {
		return avgHeartRate;
	}

	void setAvgHeartRate(unsigned char avgHeartRate) {
		this->avgHeartRate = avgHeartRate;
	}

	float getAvgNegGrade() const {
		return avgNegGrade;
	}

	void setAvgNegGrade(float avgNegGrade) {
		this->avgNegGrade = avgNegGrade;
	}

	float getAvgNegVerticalSpeed() const {
		return avgNegVerticalSpeed;
	}

	void setAvgNegVerticalSpeed(float avgNegVerticalSpeed) {
		this->avgNegVerticalSpeed = avgNegVerticalSpeed;
	}

	float getAvgPosGrade() const {
		return avgPosGrade;
	}

	void setAvgPosGrade(float avgPosGrade) {
		this->avgPosGrade = avgPosGrade;
	}

	float getAvgPosVerticalSpeed() const {
		return avgPosVerticalSpeed;
	}

	void setAvgPosVerticalSpeed(float avgPosVerticalSpeed) {
		this->avgPosVerticalSpeed = avgPosVerticalSpeed;
	}

	unsigned short getAvgPower() const {
		return avgPower;
	}

	void setAvgPower(unsigned short avgPower) {
		this->avgPower = avgPower;
	}

	unsigned char getAvgRunningCadence() const {
		return avgCadence;
	}

	void setAvgRunningCadence(unsigned char avgRunningCadence) {
		this->avgCadence = avgRunningCadence;
	}

	float getAvgSpeed() const {
		return avgSpeed;
	}

	void setAvgSpeed(float avgSpeed) {
		this->avgSpeed = avgSpeed;
	}

	float getAvgStrokeDistance() const {
		return avgStrokeDistance;
	}

	void setAvgStrokeDistance(float avgStrokeDistance) {
		this->avgStrokeDistance = avgStrokeDistance;
	}

	signed char getAvgTemperature() const {
		return avgTemperature;
	}

	void setAvgTemperature(signed char avgTemperature) {
		this->avgTemperature = avgTemperature;
	}

	signed long getEndPositionLat() const {
		return endPositionLat;
	}

	void setEndPositionLat(signed long endPositionLat) {
		this->endPositionLat = endPositionLat;
	}

	signed long getEndPositionLong() const {
		return endPositionLong;
	}

	void setEndPositionLong(signed long endPositionLong) {
		this->endPositionLong = endPositionLong;
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

	unsigned short getFirstLengthIndex() const {
		return firstLengthIndex;
	}

	void setFirstLengthIndex(unsigned short firstLengthIndex) {
		this->firstLengthIndex = firstLengthIndex;
	}

	unsigned char getGpsAccuracy() const {
		return gpsAccuracy;
	}

	void setGpsAccuracy(unsigned char gpsAccuracy) {
		this->gpsAccuracy = gpsAccuracy;
	}

	unsigned char getIntensity() const {
		return intensity;
	}

	void setIntensity(unsigned char intensity) {
		this->intensity = intensity;
	}

	unsigned char getLapTrigger() const {
		return lapTrigger;
	}

	void setLapTrigger(unsigned char lapTrigger) {
		this->lapTrigger = lapTrigger;
	}

	unsigned short getLeftRightBalance() const {
		return leftRightBalance;
	}

	void setLeftRightBalance(unsigned short leftRightBalance) {
		this->leftRightBalance = leftRightBalance;
	}

	float getMaxAltitude() const {
		return maxAltitude;
	}

	void setMaxAltitude(float maxAltitude) {
		this->maxAltitude = maxAltitude;
	}

	unsigned char getMaxCadence() const {
		return maxCadence;
	}

	void setMaxCadence(unsigned char maxCadence) {
		this->maxCadence = maxCadence;
	}

	unsigned char getMaxHeartRate() const {
		return maxHeartRate;
	}

	void setMaxHeartRate(unsigned char maxHeartRate) {
		this->maxHeartRate = maxHeartRate;
	}

	float getMaxNegGrade() const {
		return maxNegGrade;
	}

	void setMaxNegGrade(float maxNegGrade) {
		this->maxNegGrade = maxNegGrade;
	}

	float getMaxNegVerticalSpeed() const {
		return maxNegVerticalSpeed;
	}

	void setMaxNegVerticalSpeed(float maxNegVerticalSpeed) {
		this->maxNegVerticalSpeed = maxNegVerticalSpeed;
	}

	float getMaxPosGrade() const {
		return maxPosGrade;
	}

	void setMaxPosGrade(float maxPosGrade) {
		this->maxPosGrade = maxPosGrade;
	}

	float getMaxPosVerticalSpeed() const {
		return maxPosVerticalSpeed;
	}

	void setMaxPosVerticalSpeed(float maxPosVerticalSpeed) {
		this->maxPosVerticalSpeed = maxPosVerticalSpeed;
	}

	unsigned short getMaxPower() const {
		return maxPower;
	}

	void setMaxPower(unsigned short maxPower) {
		this->maxPower = maxPower;
	}

	unsigned char getMaxRunningCadence() const {
		return maxCadence;
	}

	void setMaxRunningCadence(unsigned char maxRunningCadence) {
		this->maxCadence = maxRunningCadence;
	}

	float getMaxSpeed() const {
		return maxSpeed;
	}

	void setMaxSpeed(float maxSpeed) {
		this->maxSpeed = maxSpeed;
	}

	signed char getMaxTemperature() const {
		return maxTemperature;
	}

	void setMaxTemperature(signed char maxTemperature) {
		this->maxTemperature = maxTemperature;
	}

	unsigned short getMessageIndex() const {
		return messageIndex;
	}

	void setMessageIndex(unsigned short messageIndex) {
		this->messageIndex = messageIndex;
	}

	float getMinAltitude() const {
		return minAltitude;
	}

	void setMinAltitude(float minAltitude) {
		this->minAltitude = minAltitude;
	}

	unsigned char getMinHeartRate() const {
		return minHeartRate;
	}

	void setMinHeartRate(unsigned char minHeartRate) {
		this->minHeartRate = minHeartRate;
	}

	unsigned short getNormalizedPower() const {
		return normalizedPower;
	}

	void setNormalizedPower(unsigned short normalizedPower) {
		this->normalizedPower = normalizedPower;
	}

	unsigned short getNumActiveLengths() const {
		return numActiveLengths;
	}

	void setNumActiveLengths(unsigned short numActiveLengths) {
		this->numActiveLengths = numActiveLengths;
	}

	unsigned short getNumLengths() const {
		return numLengths;
	}

	void setNumLengths(unsigned short numLengths) {
		this->numLengths = numLengths;
	}

	unsigned char getNumTimeInCadenceZone() const {
		return numTimeInCadenceZone;
	}

	void setNumTimeInCadenceZone(unsigned char numTimeInCadenceZone) {
		this->numTimeInCadenceZone = numTimeInCadenceZone;
	}

	unsigned char getNumTimeInHrZone() const {
		return numTimeInHrZone;
	}

	void setNumTimeInHrZone(unsigned char numTimeInHrZone) {
		this->numTimeInHrZone = numTimeInHrZone;
	}

	unsigned char getNumTimeInPowerZone() const {
		return numTimeInPowerZone;
	}

	void setNumTimeInPowerZone(unsigned char numTimeInPowerZone) {
		this->numTimeInPowerZone = numTimeInPowerZone;
	}

	unsigned char getNumTimeInSpeedZone() const {
		return numTimeInSpeedZone;
	}

	void setNumTimeInSpeedZone(unsigned char numTimeInSpeedZone) {
		this->numTimeInSpeedZone = numTimeInSpeedZone;
	}

	unsigned short getRepetitionNum() const {
		return repetitionNum;
	}

	void setRepetitionNum(unsigned short repetitionNum) {
		this->repetitionNum = repetitionNum;
	}

	unsigned char getSport() const {
		return sport;
	}

	void setSport(unsigned char sport) {
		this->sport = sport;
	}

	signed long getStartPositionLat() const {
		return startPositionLat;
	}

	void setStartPositionLat(signed long startPositionLat) {
		this->startPositionLat = startPositionLat;
	}

	signed long getStartPositionLong() const {
		return startPositionLong;
	}

	void setStartPositionLong(signed long startPositionLong) {
		this->startPositionLong = startPositionLong;
	}

	unsigned long getStartTime() const {
		return startTime;
	}

	void setStartTime(unsigned long startTime) {
		this->startTime = startTime;
	}

	unsigned char getSubSport() const {
		return subSport;
	}

	void setSubSport(unsigned char subSport) {
		this->subSport = subSport;
	}

	unsigned char getSwimStroke() const {
		return swimStroke;
	}

	void setSwimStroke(unsigned char swimStroke) {
		this->swimStroke = swimStroke;
	}

	unsigned long getTimestamp() const {
		return timestamp;
	}

	void setTimestamp(unsigned long timestamp) {
		this->timestamp = timestamp;
	}

	unsigned short getTotalAscent() const {
		return totalAscent;
	}

	void setTotalAscent(unsigned short totalAscent) {
		this->totalAscent = totalAscent;
	}

	unsigned short getTotalCalories() const {
		return totalCalories;
	}

	void setTotalCalories(unsigned short totalCalories) {
		this->totalCalories = totalCalories;
	}

	unsigned long getTotalCycles() const {
		return totalCycles;
	}

	void setTotalCycles(unsigned long totalCycles) {
		this->totalCycles = totalCycles;
	}

	unsigned short getTotalDescent() const {
		return totalDescent;
	}

	void setTotalDescent(unsigned short totalDescent) {
		this->totalDescent = totalDescent;
	}

	float getTotalDistance() const {
		return totalDistance;
	}

	void setTotalDistance(float totalDistance) {
		this->totalDistance = totalDistance;
	}

	float getTotalElapsedTime() const {
		return totalElapsedTime;
	}

	void setTotalElapsedTime(float totalElapsedTime) {
		this->totalElapsedTime = totalElapsedTime;
	}

	unsigned short getTotalFatCalories() const {
		return totalFatCalories;
	}

	void setTotalFatCalories(unsigned short totalFatCalories) {
		this->totalFatCalories = totalFatCalories;
	}

	float getTotalMovingTime() const {
		return totalMovingTime;
	}

	void setTotalMovingTime(float totalMovingTime) {
		this->totalMovingTime = totalMovingTime;
	}

	unsigned long getTotalStrides() const {
		return totalStrides;
	}

	void setTotalStrides(unsigned long totalStrides) {
		this->totalStrides = totalStrides;
	}

	float getTotalTimerTime() const {
		return totalTimerTime;
	}

	void setTotalTimerTime(float totalTimerTime) {
		this->totalTimerTime = totalTimerTime;
	}

	unsigned long getTotalWork() const {
		return totalWork;
	}

	void setTotalWork(unsigned long totalWork) {
		this->totalWork = totalWork;
	}

	unsigned short getWktStepIndex() const {
		return wktStepIndex;
	}

	void setWktStepIndex(unsigned short wktStepIndex) {
		this->wktStepIndex = wktStepIndex;
	}




};

#endif // FITMSG_LAP_H
