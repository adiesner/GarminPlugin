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

#include "TcxLap.h"
#include "TcxTypes.h"

TcxLap::TcxLap() {
    this->totalTimeSeconds="";
    this->distanceMeters="";
    this->maximumSpeed="";
    this->calories = "";
    this->averageHeartRateBpm = "";
    this->maximumHeartRateBpm = "";
    this->intensity = TrainingCenterDatabase::Resting;
    this->cadence = "";
    this->triggerMethod = TrainingCenterDatabase::Manual;
    this->notes = "";
    this->cadenceSensorType = TrainingCenterDatabase::UndefinedCadenceType;
    this->maxCadence="";
    this->avgSpeed="";
}

TcxLap::~TcxLap() {
    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        delete(track);
    }
    trackList.clear();
}

void TcxLap::addTrack(TcxTrack* track) {
    this->trackList.push_back(track);
}

TiXmlElement * TcxLap::getTiXml(bool readTrackData) {
    TiXmlElement * xmlLap = new TiXmlElement("Lap");

    xmlLap->SetAttribute("StartTime",getStartTime());

    if (this->totalTimeSeconds.length() == 0) {
        calculateTotalTimeSeconds();
    }
    TiXmlElement * xmlTotalTimeSeconds = new TiXmlElement("TotalTimeSeconds");
    xmlTotalTimeSeconds->LinkEndChild(new TiXmlText(this->totalTimeSeconds));
    xmlLap->LinkEndChild(xmlTotalTimeSeconds);

    if (this->distanceMeters.length() == 0) {
        calculateDistanceMeters();
    }
    TiXmlElement * xmlDistanceMeters = new TiXmlElement("DistanceMeters");
    xmlDistanceMeters->LinkEndChild(new TiXmlText(this->distanceMeters));
    xmlLap->LinkEndChild(xmlDistanceMeters);

    if (this->maximumSpeed.length() > 0) {
        TiXmlElement * xmlMaxSpeed = new TiXmlElement("MaximumSpeed");
        xmlMaxSpeed->LinkEndChild(new TiXmlText(this->maximumSpeed));
        xmlLap->LinkEndChild(xmlMaxSpeed);
    }

    if (this->calories.length() == 0) {
        calculateCalories();
    }
    TiXmlElement * xmlCalories = new TiXmlElement("Calories");
    xmlCalories->LinkEndChild(new TiXmlText(this->calories));
    xmlLap->LinkEndChild(xmlCalories);

    if (this->averageHeartRateBpm.length() > 0) {
        //TODO: Think about calculating averageHeartRateBpm value
        TiXmlElement * xmlAvgHeart = new TiXmlElement("AverageHeartRateBpm");
        //xmlAvgHeart->SetAttribute("xsi:type","HeartRateInBeatsPerMinute_t");
        TiXmlElement * xmlValue = new TiXmlElement("Value");
        this->averageHeartRateBpm = TrainingCenterDatabase::limitIntValue(this->averageHeartRateBpm, 0,255);
        xmlValue->LinkEndChild(new TiXmlText(this->averageHeartRateBpm));
        xmlAvgHeart->LinkEndChild(xmlValue);
        xmlLap->LinkEndChild(xmlAvgHeart);
    }

    if (this->maximumHeartRateBpm.length() == 0) {
    	calculateMaximumHeartRateBpm();
    }

    if (this->maximumHeartRateBpm.length() > 0) {
        TiXmlElement * xmlAvgHeart = new TiXmlElement("MaximumHeartRateBpm");
        //xmlAvgHeart->SetAttribute("xsi:type","HeartRateInBeatsPerMinute_t");
        TiXmlElement * xmlValue = new TiXmlElement("Value");
        this->maximumHeartRateBpm = TrainingCenterDatabase::limitIntValue(this->maximumHeartRateBpm, 0,255);
        xmlValue->LinkEndChild(new TiXmlText(this->maximumHeartRateBpm));
        xmlAvgHeart->LinkEndChild(xmlValue);
        xmlLap->LinkEndChild(xmlAvgHeart);
    }

    TiXmlElement * xmlIntensity = new TiXmlElement("Intensity");
    xmlIntensity->LinkEndChild(new TiXmlText(getIntensity(this->intensity)));
    xmlLap->LinkEndChild(xmlIntensity);

    if ((this->cadence.length() > 0) && (this->cadenceSensorType != TrainingCenterDatabase::UndefinedCadenceType)) {
        this->cadence = TrainingCenterDatabase::limitIntValue(this->cadence, 0,255);
        if (this->cadence != "255") {
            if (this->cadenceSensorType == TrainingCenterDatabase::Bike) {
                TiXmlElement * xmlCadence = new TiXmlElement("Cadence");
                xmlCadence->LinkEndChild(new TiXmlText(this->cadence));
                xmlLap->LinkEndChild(xmlCadence);
            }
        }
    }

    TiXmlElement * xmlTriggerMethod = new TiXmlElement("TriggerMethod");
    xmlTriggerMethod->LinkEndChild(new TiXmlText(getTriggerMethod(this->triggerMethod)));
    xmlLap->LinkEndChild(xmlTriggerMethod);

    if (readTrackData) {
        vector<TcxTrack*>::iterator it;
        for ( it=trackList.begin() ; it < trackList.end(); ++it )
        {
            TcxTrack* track = *it;
            xmlLap->LinkEndChild(track->getTiXml());
        }
    }

    TiXmlElement * xmlLapExtensions = NULL;
    if ((this->cadence.length() > 0) && (this->cadenceSensorType != TrainingCenterDatabase::UndefinedCadenceType)) {
        if (this->cadence != "255") {
            if (this->cadenceSensorType != TrainingCenterDatabase::Bike) {
                if (xmlLapExtensions == NULL) {
                    xmlLapExtensions = new TiXmlElement("Extensions");
                    xmlLap->LinkEndChild(xmlLapExtensions);
                }

                TiXmlElement * xmlLX = new TiXmlElement("LX");
                xmlLX->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/ActivityExtension/v2");
                xmlLapExtensions->LinkEndChild(xmlLX);
                TiXmlElement * xmlAvgRunCadence = new TiXmlElement("AvgRunCadence");
                xmlAvgRunCadence->LinkEndChild(new TiXmlText(this->cadence));
                xmlLX->LinkEndChild(xmlAvgRunCadence);
            }
        }
    }

    if (this->maxCadence.length() > 0) {
        if (xmlLapExtensions == NULL) {
            xmlLapExtensions = new TiXmlElement("Extensions");
            xmlLap->LinkEndChild(xmlLapExtensions);
        }

        string name = "MaxBikeCadence";
        if (this->cadenceSensorType == TrainingCenterDatabase::Footpod) {
        	name = "MaxRunningCadence";
        }

        TiXmlElement * xmlLX = new TiXmlElement("LX");
        xmlLX->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/ActivityExtension/v2");
        xmlLapExtensions->LinkEndChild(xmlLX);
        TiXmlElement * xmlMaxCadence = new TiXmlElement(name);
        xmlMaxCadence->LinkEndChild(new TiXmlText(this->maxCadence));
        xmlLX->LinkEndChild(xmlMaxCadence);
    }

    if (this->avgSpeed.length() > 0) {
        if (xmlLapExtensions == NULL) {
            xmlLapExtensions = new TiXmlElement("Extensions");
            xmlLap->LinkEndChild(xmlLapExtensions);
        }

        TiXmlElement * xmlLX = new TiXmlElement("LX");
        xmlLX->SetAttribute("xmlns","http://www.garmin.com/xmlschemas/ActivityExtension/v2");
        xmlLapExtensions->LinkEndChild(xmlLX);
        TiXmlElement * xmlAvgSpeed = new TiXmlElement("AvgSpeed");
        xmlAvgSpeed->LinkEndChild(new TiXmlText(this->avgSpeed));
        xmlLX->LinkEndChild(xmlAvgSpeed);
    }

    return xmlLap;
}

TiXmlElement * TcxLap::getGpxTiXml() {
    TiXmlElement * segment = new TiXmlElement("trkseg");

    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        vector<TiXmlElement *> trkPointList = track->getGpxTiXml();
        vector<TiXmlElement *>::iterator it;
        for ( it=trkPointList.begin() ; it < trkPointList.end(); ++it ) {
            TiXmlElement * elem = *it;
            segment->LinkEndChild(elem);
        }
    }

    return segment;
}

void TcxLap::setTotalTimeSeconds(string time) {
    this->totalTimeSeconds=time;
}
void TcxLap::setDistanceMeters(string distance) {
    this->distanceMeters=distance;
}
void TcxLap::setMaximumSpeed(string speed) {
    this->maximumSpeed=speed;
}
void TcxLap::setCalories(string calories) {
    this->calories = calories;
}
void TcxLap::setAverageHeartRateBpm(string averageBpm) {
    this->averageHeartRateBpm = averageBpm;
}
void TcxLap::setMaximumHeartRateBpm(string maximumBpm) {
    this->maximumHeartRateBpm = maximumBpm;
}
void TcxLap::setIntensity(TrainingCenterDatabase::Intensity_t intensitiy) {
    this->intensity = intensitiy;
}
void TcxLap::setCadence(string cadence) {
    this->cadence = cadence;
}
void TcxLap::setTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method) {
    this->triggerMethod = method;
}
void TcxLap::setNotes(string note) {
    this->notes = note;
}

void TcxLap::setAvgSpeed(string speed) {
	this->avgSpeed = speed;
}
void TcxLap::setMaxCadence(string cadence) {
	this->maxCadence = cadence;
}

void TcxLap::calculateTotalTimeSeconds() {
    double totalTime = 0;

    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        totalTime += track->calculateTotalTime();
    }
    char totalTimeBuf[50];
    snprintf(&totalTimeBuf[0], sizeof(totalTimeBuf), "%.2f", totalTime);
    this->totalTimeSeconds=totalTimeBuf;
}

void TcxLap::calculateDistanceMeters() {
    double totalDistanceMeters = 0;

    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        totalDistanceMeters += track->calculateDistanceMeters();
    }
    char totalDistanceBuf[50];
    snprintf(&totalDistanceBuf[0], sizeof(totalDistanceBuf), "%.2f", totalDistanceMeters);
    this->distanceMeters=totalDistanceBuf;
}

void TcxLap::calculateCalories() {
    //TODO: Calculate Calories
    this->calories = "0";
}

void TcxLap::calculateMaximumHeartRateBpm() {
    vector<TcxTrack*>::iterator it;
    int maxHeartRate = 0;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        int currentMaxHeartRate = track->getMaxHeartRate();
        maxHeartRate = (currentMaxHeartRate > maxHeartRate) ? currentMaxHeartRate : maxHeartRate;
    }
    if (maxHeartRate > 0) {
    	stringstream ss;
    	ss << maxHeartRate;
    	this->maximumHeartRateBpm=ss.str();
    }
}

string TcxLap::getIntensity(TrainingCenterDatabase::Intensity_t intensity) {
    if (intensity == TrainingCenterDatabase::Active) {
        return "Active";
    }
    return "Resting";
}

string TcxLap::getTriggerMethod(TrainingCenterDatabase::TriggerMethod_t method) {
    switch(method) {
        case TrainingCenterDatabase::Manual:
            return "Manual";
        case TrainingCenterDatabase::Distance:
            return "Distance";
        case TrainingCenterDatabase::Location:
            return "Location";
        case TrainingCenterDatabase::Time:
            return "Time";
        case TrainingCenterDatabase::HeartRate:
            return "HeartRate";
    }
    return "";
}

void TcxLap::setCadenceSensorType(TrainingCenterDatabase::CadenceSensorType_t type) {
    this->cadenceSensorType = type;
}

TcxLap& operator<<(TcxLap& lap, TcxTrack* track)
{
    lap.addTrack(track);
    return lap;
}

string TcxLap::getStartTime() {
    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        string startTime = track->getStartTime();
        if (startTime.length() > 0) {
            return startTime;
        }
    }
    return "1970-01-01T00:00:00Z";
}

bool TcxLap::isEmpty() {
    vector<TcxTrack*>::iterator it;
    for ( it=trackList.begin() ; it < trackList.end(); ++it )
    {
        TcxTrack* track = *it;
        if (!track->isEmpty()) {
            return false;
        }
    }
    return true;
}

string TcxLap::getDistance() {
    if (this->distanceMeters.length() == 0) {
        calculateDistanceMeters();
    }
    return this->distanceMeters;
}
