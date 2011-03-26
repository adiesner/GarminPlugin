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
#ifndef TCXLAP_H_INCLUDED
#define TCXLAP_H_INCLUDED

#include <string>
#include <vector>
#include "TcxTrack.h"
#include "TcxTypes.h"

using namespace std;

class TcxLap
{
public:

    TcxLap();

    ~TcxLap();

    void addTrack(TcxTrack* track);

    TiXmlElement * getTiXml(bool readTrackData);
    TiXmlElement * getGpxTiXml();

    void setTotalTimeSeconds(string time);
    void setDistanceMeters(string distance);
    void setMaximumSpeed(string speed);
    void setCalories(string calories);
    void setAverageHeartRateBpm(string averageBpm);
    void setMaximumHeartRateBpm(string maximumBpm);
    void setIntensity(TrainingCenterDatabase::Intensity_t intensitiy);
    void setCadence(string cadence);
    void setTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method);
    void setNotes(string note);
    void setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type);

    /**
     * Returns the distance of this lap - used for debug output
     * Do not call before all tracks and trackpoints are added to the lap
     */
    string getDistance();

    bool isEmpty();

    friend TcxLap& operator<<(TcxLap& base, TcxTrack* track);

private:
    vector<TcxTrack*> trackList;

    void calculateTotalTimeSeconds();
    void calculateDistanceMeters();
    void calculateCalories();
    string getTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method);
    string getIntensity(TrainingCenterDatabase::Intensity_t intensity);
    string getStartTime();

    string totalTimeSeconds;
    string distanceMeters;
    string maximumSpeed;
    string calories;
    string averageHeartRateBpm;
    string maximumHeartRateBpm;
    TrainingCenterDatabase::Intensity_t intensity;
    string cadence;
    TrainingCenterDatabase::TriggerMethod_t triggerMethod;
    string notes;
    TrainingCenterDatabase::CadenceSensorType_t cadenceSensorType;

};

#endif // TCXLAP_H_INCLUDED
