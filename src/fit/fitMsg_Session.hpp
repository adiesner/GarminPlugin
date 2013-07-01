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


#ifndef FITMSG_SESSION_H
#define FITMSG_SESSION_H

#define FIT_MESSAGE_SESSION                                     ((unsigned char)18)

#include "fitDefines.hpp"
#include <sstream>

class FitMsg_Session: public FitMsg
{
public:
	FitMsg_Session() : FitMsg(FIT_MESSAGE_SESSION) {
    };

    virtual ~FitMsg_Session() {};

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
			case 5: setSport(read0x00(data,arch));
					 break;
			case 6: setSubSport(read0x00(data,arch));
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
			case 13: setTotalFatCalories(read0x84(data,arch));
					 break;
			case 14: setAvgSpeed(read0x88(data,arch,1000,0,0x84));
					 break;
			case 15: setMaxSpeed(read0x88(data,arch,1000,0,0x84));
					 break;
			case 16: setAvgHeartRate(read0x02(data,arch));
					 break;
			case 17: setMaxHeartRate(read0x02(data,arch));
					 break;
			case 18: setAvgCadence(read0x02(data,arch));
					 break;
			case 19: setMaxCadence(read0x02(data,arch));
					 break;
			case 20: setAvgPower(read0x84(data,arch));
					 break;
			case 21: setMaxPower(read0x84(data,arch));
					 break;
			case 22: setTotalAscent(read0x84(data,arch));
					 break;
			case 23: setTotalDescent(read0x84(data,arch));
					 break;
			case 24: setTotalTrainingEffect(read0x88(data,arch,10,0,0x02));
					 break;
			case 25: setFirstLapIndex(read0x84(data,arch));
					 break;
			case 26: setNumLaps(read0x84(data,arch));
					 break;
			case 27: setEventGroup(read0x02(data,arch));
					 break;
			case 28: setTrigger(read0x00(data,arch));
					 break;
			case 29: setNecLat(read0x85(data,arch));
					 break;
			case 30: setNecLong(read0x85(data,arch));
					 break;
			case 31: setSwcLat(read0x85(data,arch));
					 break;
			case 32: setSwcLong(read0x85(data,arch));
					 break;
			case 34: setNormalizedPower(read0x84(data,arch));
					 break;
			case 35: setTrainingStressScore(read0x88(data,arch,10,0,0x84));
					 break;
			case 36: setIntensityFactor(read0x88(data,arch,1000,0,0x84));
					 break;
			case 37: setLeftRightBalance(read0x84(data,arch));
					 break;
			case 41: setAvgStrokeCount(read0x88(data,arch,10,0,0x86));
					 break;
			case 42: setAvgStrokeDistance(read0x88(data,arch,100,0,0x84));
					 break;
			case 43: setSwimStroke(read0x00(data,arch));
					 break;
			case 44: setPoolLength(read0x88(data,arch,100,0,0x84));
					 break;
			case 46: setPoolLengthUnit(read0x00(data,arch));
					 break;
			case 47: setNumActiveLengths(read0x84(data,arch));
					 break;
			case 48: setTotalWork(read0x86(data,arch));
					 break;
			case 49: setAvgAltitude(read0x88(data,arch,5,500,0x84));
					 break;
			case 50: setMaxAltitude(read0x88(data,arch,5,500,0x84));
					 break;
			case 51: setGpsAccuracy(read0x02(data,arch));
					 break;
			case 52: setAvgGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 53: setAvgPosGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 54: setAvgNegGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 55: setMaxPosGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 56: setMaxNegGrade(read0x88(data,arch,100,0,0x83));
					 break;
			case 57: setAvgTemperature(read0x01(data,arch));
					 break;
			case 58: setMaxTemperature(read0x01(data,arch));
					 break;
			case 59: setTotalMovingTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 60: setAvgPosVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 61: setAvgNegVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 62: setMaxPosVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 63: setMaxNegVerticalSpeed(read0x88(data,arch,1000,0,0x83));
					 break;
			case 64: setMinHeartRate(read0x02(data,arch));
					 break;
			case 65: setNumTimeInHrZone(read0x02(data,arch));
					 break;
			case 66: setNumTimeInSpeedZone(read0x02(data,arch));
					 break;
			case 67: setNumTimeInCadenceZone(read0x02(data,arch));
					 break;
			case 68: setNumTimeInPowerZone(read0x02(data,arch));
					 break;
			case 69: setAvgLapTime(read0x88(data,arch,1000,0,0x86));
					 break;
			case 70: setBestLapIndex(read0x84(data,arch));
					 break;
			case 71: setMinAltitude(read0x88(data,arch,5,500,0x84));
					 break;


            default:
                fieldWasAdded = false;
                break;
        }
        return fieldWasAdded;
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

	/* sport - Unit:  */
	unsigned char sport;

	/* sub_sport - Unit:  */
	unsigned char subSport;

	/* total_elapsed_time - Unit: s */
	float totalElapsedTime;

	/* total_timer_time - Unit: s */
	float totalTimerTime;

	/* total_distance - Unit: m */
	float totalDistance;

	/* total_cycles - Unit: cycles */
	unsigned long totalCycles;

	/* total_cycles - Unit: cycles */
	//unsigned long totalStrides; == totalCycles

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
	//unsigned char avgRunningCadence; == avgCadence

	/* max_cadence - Unit: rpm */
	unsigned char maxCadence;

	/* max_cadence - Unit: rpm */
	//unsigned char maxRunningCadence; == maxCadence

	/* avg_power - Unit: watts */
	unsigned short avgPower;

	/* max_power - Unit: watts */
	unsigned short maxPower;

	/* total_ascent - Unit: m */
	unsigned short totalAscent;

	/* total_descent - Unit: m */
	unsigned short totalDescent;

	/* total_training_effect - Unit:  */
	float totalTrainingEffect;

	/* first_lap_index - Unit:  */
	unsigned short firstLapIndex;

	/* num_laps - Unit:  */
	unsigned short numLaps;

	/* event_group - Unit:  */
	unsigned char eventGroup;

	/* trigger - Unit:  */
	unsigned char trigger;

	/* nec_lat - Unit: semicircles */
	signed long necLat;

	/* nec_long - Unit: semicircles */
	signed long necLong;

	/* swc_lat - Unit: semicircles */
	signed long swcLat;

	/* swc_long - Unit: semicircles */
	signed long swcLong;

	/* normalized_power - Unit: watts */
	unsigned short normalizedPower;

	/* training_stress_score - Unit: tss */
	float trainingStressScore;

	/* intensity_factor - Unit: if */
	float intensityFactor;

	/* left_right_balance - Unit:  */
	unsigned short leftRightBalance;

	/* avg_stroke_count - Unit: strokes/lap */
	float avgStrokeCount;

	/* avg_stroke_distance - Unit: m */
	float avgStrokeDistance;

	/* swim_stroke - Unit: swim_stroke */
	unsigned char swimStroke;

	/* pool_length - Unit: m */
	float poolLength;

	/* pool_length_unit - Unit:  */
	unsigned char poolLengthUnit;

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

	/* min_heart_rate - Unit: bpm */
	unsigned char minHeartRate;

	/* time_in_hr_zone - Unit: s */
	unsigned char numTimeInHrZone;

	/* time_in_speed_zone - Unit: s */
	unsigned char numTimeInSpeedZone;

	/* time_in_cadence_zone - Unit: s */
	unsigned char numTimeInCadenceZone;

	/* time_in_power_zone - Unit: s */
	unsigned char numTimeInPowerZone;

	/* avg_lap_time - Unit: s */
	float avgLapTime;

	/* best_lap_index - Unit:  */
	unsigned short bestLapIndex;

	/* min_altitude - Unit: m */
	float minAltitude;

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

	float getAvgLapTime() const {
		return avgLapTime;
	}

	void setAvgLapTime(float avgLapTime) {
		this->avgLapTime = avgLapTime;
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

	float getAvgStrokeCount() const {
		return avgStrokeCount;
	}

	void setAvgStrokeCount(float avgStrokeCount) {
		this->avgStrokeCount = avgStrokeCount;
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

	unsigned short getBestLapIndex() const {
		return bestLapIndex;
	}

	void setBestLapIndex(unsigned short bestLapIndex) {
		this->bestLapIndex = bestLapIndex;
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

	unsigned short getFirstLapIndex() const {
		return firstLapIndex;
	}

	void setFirstLapIndex(unsigned short firstLapIndex) {
		this->firstLapIndex = firstLapIndex;
	}

	unsigned char getGpsAccuracy() const {
		return gpsAccuracy;
	}

	void setGpsAccuracy(unsigned char gpsAccuracy) {
		this->gpsAccuracy = gpsAccuracy;
	}

	float getIntensityFactor() const {
		return intensityFactor;
	}

	void setIntensityFactor(float intensityFactor) {
		this->intensityFactor = intensityFactor;
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

	signed long getNecLat() const {
		return necLat;
	}

	void setNecLat(signed long necLat) {
		this->necLat = necLat;
	}

	signed long getNecLong() const {
		return necLong;
	}

	void setNecLong(signed long necLong) {
		this->necLong = necLong;
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

	unsigned short getNumLaps() const {
		return numLaps;
	}

	void setNumLaps(unsigned short numLaps) {
		this->numLaps = numLaps;
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

	float getPoolLength() const {
		return poolLength;
	}

	void setPoolLength(float poolLength) {
		this->poolLength = poolLength;
	}

	unsigned char getPoolLengthUnit() const {
		return poolLengthUnit;
	}

	void setPoolLengthUnit(unsigned char poolLengthUnit) {
		this->poolLengthUnit = poolLengthUnit;
	}

	FIT_SPORT getSport() const {
		return (FIT_SPORT)sport;
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

	signed long getSwcLat() const {
		return swcLat;
	}

	void setSwcLat(signed long swcLat) {
		this->swcLat = swcLat;
	}

	signed long getSwcLong() const {
		return swcLong;
	}

	void setSwcLong(signed long swcLong) {
		this->swcLong = swcLong;
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
		return totalCycles;
	}

	void setTotalStrides(unsigned long totalStrides) {
		this->totalCycles = totalStrides;
	}

	float getTotalTimerTime() const {
		return totalTimerTime;
	}

	void setTotalTimerTime(float totalTimerTime) {
		this->totalTimerTime = totalTimerTime;
	}

	float getTotalTrainingEffect() const {
		return totalTrainingEffect;
	}

	void setTotalTrainingEffect(float totalTrainingEffect) {
		this->totalTrainingEffect = totalTrainingEffect;
	}

	unsigned long getTotalWork() const {
		return totalWork;
	}

	void setTotalWork(unsigned long totalWork) {
		this->totalWork = totalWork;
	}

	float getTrainingStressScore() const {
		return trainingStressScore;
	}

	void setTrainingStressScore(float trainingStressScore) {
		this->trainingStressScore = trainingStressScore;
	}

	unsigned char getTrigger() const {
		return trigger;
	}

	void setTrigger(unsigned char trigger) {
		this->trigger = trigger;
	}



};

#endif // FITMSG_SESSION_H
