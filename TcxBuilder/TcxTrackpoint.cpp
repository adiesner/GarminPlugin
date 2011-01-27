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

#include "TcxTrackpoint.h"
#include "../gpsFunctions.h"
#include "../log.h"

void TcxTrackpoint::initializeVariables() {
    this->longitude = "";
    this->latitude = "";
    this->altitudeMeters = "";
    this->distanceMeters = "";
    this->heartRateBpm = "";
    this->cadence = "";
    this->sensorState = TrainingCenterDatabase::UndefinedSensorState;
    this->cadenceSensorType = TrainingCenterDatabase::UndefinedCadenceType;
}

TcxTrackpoint::TcxTrackpoint(string time, string latitude, string longitude) {
    initializeVariables();
    this->time = time;
    this->longitude = longitude;
    this->latitude = latitude;
}

TcxTrackpoint::TcxTrackpoint(string time) {
    initializeVariables();
    this->time = time;
}

TcxTrackpoint::~TcxTrackpoint() {
}


void TcxTrackpoint::setPosition(string latitude, string longitude) {
    this->longitude = longitude;
    this->latitude = latitude;
}

void TcxTrackpoint::setAltitudeMeters(string altitude) {
    this->altitudeMeters = altitude;
}

void TcxTrackpoint::setDistanceMeters(string distance) {
    this->distanceMeters = distance;
}

void TcxTrackpoint::setHeartRateBpm(string heartrate) {
    this->heartRateBpm = heartrate;
}

void TcxTrackpoint::setCadence(string cadence) {
    this->cadence = cadence;
}

void TcxTrackpoint::setSensorState(TrainingCenterDatabase::SensorState_t state) {
    this->sensorState = state;
}

void TcxTrackpoint::setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type) {
    this->cadenceSensorType = type;
}


TiXmlElement * TcxTrackpoint::getTiXml() {
    TiXmlElement * xmlTrackPoint = new TiXmlElement("Trackpoint");

    TiXmlElement * xmlTime = new TiXmlElement("Time");
    xmlTime->LinkEndChild(new TiXmlText(this->time));
    xmlTrackPoint->LinkEndChild(xmlTime);

    TiXmlElement * xmlTrackPointExtensions = NULL;

    if ((this->latitude.length() > 0) && (this->latitude.length() > 0)) {
        TiXmlElement * xmlPosition = new TiXmlElement("Position");
        TiXmlElement * xmlLat = new TiXmlElement("LatitudeDegrees");
        xmlLat->LinkEndChild(new TiXmlText(this->latitude));
        TiXmlElement * xmlLon = new TiXmlElement("LongitudeDegrees");
        xmlLon->LinkEndChild(new TiXmlText(this->longitude));
        xmlPosition->LinkEndChild(xmlLat);
        xmlPosition->LinkEndChild(xmlLon);
        xmlTrackPoint->LinkEndChild(xmlPosition);
    }

    if (this->altitudeMeters.length() > 0) {
        TiXmlElement * xmlAlt = new TiXmlElement("AltitudeMeters");
        xmlAlt->LinkEndChild(new TiXmlText(this->altitudeMeters));
        xmlTrackPoint->LinkEndChild(xmlAlt);
    }

    if (this->distanceMeters.length() > 0) {
        TiXmlElement * xmlDist = new TiXmlElement("DistanceMeters");
        xmlDist->LinkEndChild(new TiXmlText(this->distanceMeters));
        xmlTrackPoint->LinkEndChild(xmlDist);
    }

    if (this->heartRateBpm.length() > 0) {
        TiXmlElement * xmlHeart = new TiXmlElement("HeartRateBpm");
        xmlHeart->SetAttribute("xsi:type","HeartRateInBeatsPerMinute_t");
        TiXmlElement * xmlValue = new TiXmlElement("Value");
        this->heartRateBpm = TrainingCenterDatabase::limitIntValue(this->heartRateBpm, 0,255);
        xmlValue->LinkEndChild(new TiXmlText(this->heartRateBpm));
        xmlHeart->LinkEndChild(xmlValue);
        xmlTrackPoint->LinkEndChild(xmlHeart);
    }

    if ((this->cadence.length() > 0) && ((this->cadenceSensorType != TrainingCenterDatabase::UndefinedCadenceType))) {
        this->cadence = TrainingCenterDatabase::limitIntValue(this->cadence, 0,255);
        if (this->cadence != "255") {
            if (this->cadenceSensorType == TrainingCenterDatabase::Bike) {
                TiXmlElement * xmlCad = new TiXmlElement("Cadence");
                xmlCad->LinkEndChild(new TiXmlText(this->cadence));
                xmlTrackPoint->LinkEndChild(xmlCad);
            }
        }
    }

    if (this->sensorState != TrainingCenterDatabase::UndefinedSensorState) {
        TiXmlElement * xmlSensor = new TiXmlElement("SensorState");
        string state = "Absent";
        if (this->sensorState == TrainingCenterDatabase::Present) {
            state = "Present";
        }
        xmlSensor->LinkEndChild(new TiXmlText(state));
        xmlTrackPoint->LinkEndChild(xmlSensor);
    }

    if ((this->cadence.length() > 0) && ((this->cadenceSensorType != TrainingCenterDatabase::UndefinedCadenceType))) {
        if (this->cadence != "255") {
            if (xmlTrackPointExtensions == NULL) {
                xmlTrackPointExtensions = new TiXmlElement("Extensions");
                xmlTrackPoint->LinkEndChild(xmlTrackPointExtensions);
            }

            TiXmlElement * xmlTPX = new TiXmlElement("TPX");
            xmlTPX->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/ActivityExtension/v2");
            string cadType = "Unknown";
            if (this->cadenceSensorType == TrainingCenterDatabase::Bike) {
                cadType = "Bike";
            } else if (this->cadenceSensorType == TrainingCenterDatabase::Footpod) {
                cadType = "Footpod";
            }
            xmlTPX->SetAttribute("CadenceSensor",cadType);
            xmlTrackPointExtensions->LinkEndChild(xmlTPX);

            if (this->cadenceSensorType == TrainingCenterDatabase::Footpod) {
                TiXmlElement * xmlRunCad = new TiXmlElement("RunCadence");
                xmlRunCad->LinkEndChild(new TiXmlText(this->cadence));
                xmlTPX->LinkEndChild(xmlRunCad);
            }
        }
    }

    return xmlTrackPoint;
}

TiXmlElement * TcxTrackpoint::getGpxTiXml() {
    TiXmlElement * xmlTrackPoint = new TiXmlElement("trkpt");

    if (this->latitude.length() > 0) { xmlTrackPoint->SetAttribute("lat",this->latitude); }
    if (this->longitude.length() > 0) { xmlTrackPoint->SetAttribute("lon",this->longitude); }

    if (this->altitudeMeters.length() > 0) {
        TiXmlElement * xmlAlt = new TiXmlElement("ele");
        xmlAlt->LinkEndChild(new TiXmlText(this->altitudeMeters));
        xmlTrackPoint->LinkEndChild(xmlAlt);
    }
    TiXmlElement * xmlTime = new TiXmlElement("time");
    xmlTime->LinkEndChild(new TiXmlText(this->time));
    xmlTrackPoint->LinkEndChild(xmlTime);

    return xmlTrackPoint;
}

string TcxTrackpoint::getTime() {
    return this->time;
}

double TcxTrackpoint::calculateDistanceTo(double totalTrackDistance, TcxTrackpoint * nextPoint) {
    double distance = 0;
    distance = GpsFunctions::haversine_m_str(this->latitude, this->longitude, nextPoint->latitude, nextPoint->longitude);

    char distanceBuf[50];
    snprintf(&distanceBuf[0], sizeof(distanceBuf), "%.2f", totalTrackDistance);
    this->distanceMeters = distanceBuf;

    return distance;
}

bool TcxTrackpoint::hasCoordinates() {
    if ((this->longitude.length() > 0) && (this->latitude.length() > 0)) {
        return true;
    }
    return false;
}
