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


#ifndef FITMSG_RECORD_H
#define FITMSG_RECORD_H

#define FIT_MESSAGE_RECORD                                      ((unsigned char)20)

class FitMsg_Record : public FitMsg
{
public:
	FitMsg_Record() : FitMsg(FIT_MESSAGE_RECORD) {
    };

    virtual ~FitMsg_Record() {};

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
        case 0: setPositionLat(read0x85(data,arch));
                 break;
        case 1: setPositionLong(read0x85(data,arch));
                 break;
        case 2: setAltitude(read0x88(data,arch,5,500,0x84));
                 break;
        case 3: setHeartRate(read0x02(data,arch));
                 break;
        case 4: setCadence(read0x02(data,arch));
                 break;
        case 5: setDistance(read0x88(data,arch,100,0,0x86));
                 break;
        case 6: setSpeed(read0x88(data,arch,1000,0,0x84));
                 break;
        case 7: setPower(read0x84(data,arch));
                 break;
        case 9: setGrade(read0x88(data,arch,100,0,0x83));
                 break;
        case 10: setResistance(read0x02(data,arch));
                 break;
        case 11: setTimeFromCourse(read0x88(data,arch,1000,0,0x85));
                 break;
        case 12: setCycleLength(read0x88(data,arch,100,0,0x02));
                 break;
        case 13: setTemperature(read0x01(data,arch));
                 break;
        case 17: setNumSpeed1s(read0x02(data,arch));
                 break;
        case 18: setCycles(read0x02(data,arch));
                 break;
        case 19: setTotalCycles(read0x86(data,arch));
                 break;
        case 28: setCompressedAccumulatedPower(read0x84(data,arch));
                 break;
        case 29: setAccumulatedPower(read0x86(data,arch));
                 break;
        case 30: setLeftRightBalance(read0x02(data,arch));
                 break;
        case 31: setGpsAccuracy(read0x02(data,arch));
                 break;
        case 32: setVerticalSpeed(read0x88(data,arch,1000,0,0x83));
                 break;
        case 33: setCalories(read0x84(data,arch));
                 break;
        case 43: setLeftTorqueEffectiveness(read0x88(data,arch,2,0,0x02));
                 break;
        case 44: setRightTorqueEffectiveness(read0x88(data,arch,2,0,0x02));
                 break;
        case 45: setLeftPedalSmoothness(read0x88(data,arch,2,0,0x02));
                 break;
        case 46: setRightPedalSmoothness(read0x88(data,arch,2,0,0x02));
                 break;
        case 47: setCombinedPedalSmoothness(read0x88(data,arch,2,0,0x02));
                 break;
        case 52: setCadence256(read0x88(data,arch,256,0,0x84));
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

	/* position_lat - Unit: semicircles */
	signed long positionLat;

	/* position_long - Unit: semicircles */
	signed long positionLong;

	/* altitude - Unit: m */
	float altitude;

	/* heart_rate - Unit: bpm */
	unsigned char heartRate;

	/* cadence - Unit: rpm */
	unsigned char cadence;

	/* distance - Unit: m */
	float distance;

	/* speed - Unit: m/s */
	float speed;

	/* power - Unit: watts */
	unsigned short power;

	/* compressed_speed_distance - Unit:  */
	unsigned char numCompressedSpeedDistance;

	/* grade - Unit: % */
	float grade;

	/* resistance - Unit:  */
	unsigned char resistance;

	/* time_from_course - Unit: s */
	float timeFromCourse;

	/* cycle_length - Unit: m */
	float cycleLength;

	/* temperature - Unit: C */
	signed char temperature;

	/* speed_1s - Unit: m/s */
	unsigned char numSpeed1s;

	/* cycles - Unit:  */
	unsigned char cycles;

	/* total_cycles - Unit: cycles */
	unsigned long totalCycles;

	/* compressed_accumulated_power - Unit:  */
	unsigned short compressedAccumulatedPower;

	/* accumulated_power - Unit: watts */
	unsigned long accumulatedPower;

	/* left_right_balance - Unit:  */
	unsigned char leftRightBalance;

	/* gps_accuracy - Unit: m */
	unsigned char gpsAccuracy;

	/* vertical_speed - Unit: m/s */
	float verticalSpeed;

	/* calories - Unit: kcal */
	unsigned short calories;

	/* left_torque_effectiveness - Unit: percent */
	float leftTorqueEffectiveness;

	/* right_torque_effectiveness - Unit: percent */
	float rightTorqueEffectiveness;

	/* left_pedal_smoothness - Unit: percent */
	float leftPedalSmoothness;

	/* right_pedal_smoothness - Unit: percent */
	float rightPedalSmoothness;

	/* combined_pedal_smoothness - Unit: percent */
	float combinedPedalSmoothness;

	/* cadence256 - Unit: rpm */
	float cadence256;

public:
	unsigned long getAccumulatedPower() const {
		return accumulatedPower;
	}

	void setAccumulatedPower(unsigned long accumulatedPower) {
		this->accumulatedPower = accumulatedPower;
	}

	float getAltitude() const {
		return altitude;
	}

	void setAltitude(float altitude) {
		this->altitude = altitude;
	}

	unsigned char getCadence() const {
		return cadence;
	}

	void setCadence(unsigned char cadence) {
		this->cadence = cadence;
	}

	float getCadence256() const {
		return cadence256;
	}

	void setCadence256(float cadence256) {
		this->cadence256 = cadence256;
	}

	unsigned short getCalories() const {
		return calories;
	}

	void setCalories(unsigned short calories) {
		this->calories = calories;
	}

	float getCombinedPedalSmoothness() const {
		return combinedPedalSmoothness;
	}

	void setCombinedPedalSmoothness(float combinedPedalSmoothness) {
		this->combinedPedalSmoothness = combinedPedalSmoothness;
	}

	unsigned short getCompressedAccumulatedPower() const {
		return compressedAccumulatedPower;
	}

	void setCompressedAccumulatedPower(
			unsigned short compressedAccumulatedPower) {
		this->compressedAccumulatedPower = compressedAccumulatedPower;
	}

	float getCycleLength() const {
		return cycleLength;
	}

	void setCycleLength(float cycleLength) {
		this->cycleLength = cycleLength;
	}

	unsigned char getCycles() const {
		return cycles;
	}

	void setCycles(unsigned char cycles) {
		this->cycles = cycles;
	}

	float getDistance() const {
		return distance;
	}

	void setDistance(float distance) {
		this->distance = distance;
	}

	unsigned char getGpsAccuracy() const {
		return gpsAccuracy;
	}

	void setGpsAccuracy(unsigned char gpsAccuracy) {
		this->gpsAccuracy = gpsAccuracy;
	}

	float getGrade() const {
		return grade;
	}

	void setGrade(float grade) {
		this->grade = grade;
	}

	unsigned char getHeartRate() const {
		return heartRate;
	}

	void setHeartRate(unsigned char heartRate) {
		this->heartRate = heartRate;
	}

	float getLeftPedalSmoothness() const {
		return leftPedalSmoothness;
	}

	void setLeftPedalSmoothness(float leftPedalSmoothness) {
		this->leftPedalSmoothness = leftPedalSmoothness;
	}

	unsigned char getLeftRightBalance() const {
		return leftRightBalance;
	}

	void setLeftRightBalance(unsigned char leftRightBalance) {
		this->leftRightBalance = leftRightBalance;
	}

	float getLeftTorqueEffectiveness() const {
		return leftTorqueEffectiveness;
	}

	void setLeftTorqueEffectiveness(float leftTorqueEffectiveness) {
		this->leftTorqueEffectiveness = leftTorqueEffectiveness;
	}

	unsigned char getNumCompressedSpeedDistance() const {
		return numCompressedSpeedDistance;
	}

	void setNumCompressedSpeedDistance(
			unsigned char numCompressedSpeedDistance) {
		this->numCompressedSpeedDistance = numCompressedSpeedDistance;
	}

	unsigned char getNumSpeed1s() const {
		return numSpeed1s;
	}

	void setNumSpeed1s(unsigned char numSpeed1s) {
		this->numSpeed1s = numSpeed1s;
	}

	signed long getPositionLat() const {
		return positionLat;
	}

	void setPositionLat(signed long positionLat) {
		this->positionLat = positionLat;
	}

	signed long getPositionLong() const {
		return positionLong;
	}

	void setPositionLong(signed long positionLong) {
		this->positionLong = positionLong;
	}

	unsigned short getPower() const {
		return power;
	}

	void setPower(unsigned short power) {
		this->power = power;
	}

	unsigned char getResistance() const {
		return resistance;
	}

	void setResistance(unsigned char resistance) {
		this->resistance = resistance;
	}

	float getRightPedalSmoothness() const {
		return rightPedalSmoothness;
	}

	void setRightPedalSmoothness(float rightPedalSmoothness) {
		this->rightPedalSmoothness = rightPedalSmoothness;
	}

	float getRightTorqueEffectiveness() const {
		return rightTorqueEffectiveness;
	}

	void setRightTorqueEffectiveness(float rightTorqueEffectiveness) {
		this->rightTorqueEffectiveness = rightTorqueEffectiveness;
	}

	float getSpeed() const {
		return speed;
	}

	void setSpeed(float speed) {
		this->speed = speed;
	}

	signed char getTemperature() const {
		return temperature;
	}

	void setTemperature(signed char temperature) {
		this->temperature = temperature;
	}

	float getTimeFromCourse() const {
		return timeFromCourse;
	}

	void setTimeFromCourse(float timeFromCourse) {
		this->timeFromCourse = timeFromCourse;
	}

	unsigned long getTimestamp() const {
		return timestamp;
	}

	void setTimestamp(unsigned long timestamp) {
		this->timestamp = timestamp;
	}

	unsigned long getTotalCycles() const {
		return totalCycles;
	}

	void setTotalCycles(unsigned long totalCycles) {
		this->totalCycles = totalCycles;
	}

	float getVerticalSpeed() const {
		return verticalSpeed;
	}

	void setVerticalSpeed(float verticalSpeed) {
		this->verticalSpeed = verticalSpeed;
	}


};

#endif // FITMSG_RECORD_H
