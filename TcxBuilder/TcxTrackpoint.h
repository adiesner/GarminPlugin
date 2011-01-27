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
#ifndef TCXTRACKPOINT_H_INCLUDED
#define TCXTRACKPOINT_H_INCLUDED

#define TIXML_USE_TICPP
#include "ticpp.h"
#include <string>
#include "TcxTypes.h"

using namespace std;

class TcxTrackpoint
{
public:

    TcxTrackpoint(string time, string latitude, string longitude);
    TcxTrackpoint(string time);

    ~TcxTrackpoint();

    void setPosition(string latitude, string longitude);
    void setAltitudeMeters(string altitude);
    void setDistanceMeters(string distance);
    void setHeartRateBpm(string heartrate);
    void setCadence(string cadence);
    void setSensorState(TrainingCenterDatabase::SensorState_t state);
    void setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type);
    string getTime();
    double calculateDistanceTo(double totalTrackDistance, TcxTrackpoint * nextPoint);

    bool hasCoordinates();

    TiXmlElement * getTiXml();
    TiXmlElement * getGpxTiXml();

private:
    void initializeVariables();

    string time;
    string longitude;
    string latitude;
    string altitudeMeters;
    string distanceMeters;
    string heartRateBpm;
    string cadence;
    TrainingCenterDatabase::SensorState_t sensorState;
    TrainingCenterDatabase::CadenceSensorType_t cadenceSensorType;

};

#endif // TCXTRACKPOINT_H_INCLUDED
